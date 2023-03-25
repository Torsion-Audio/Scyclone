//
//  RNBO_Engine.cpp
//
//  Created by stb on 07/08/15.
//
//

#include "RNBO.h"
#include "RNBO_Engine.h"
#include "RNBO_NullPatcher.h"
#include "RNBO_ParameterInterfaceAsyncImpl.h"
#include "RNBO_ParameterInterfaceSync.h"
#include "RNBO_Utils.h"

namespace RNBO {

	class SetPatcherLocker {

	public:

		SetPatcherLocker(Engine* engine)
		: _engine(engine)
		, _didLock(false)
		{
			tryLock();
		}

		~SetPatcherLocker() {
			if (_didLock) {
				_engine->_inSetPatcher = false;
			}
		}

		bool tryLock(std::chrono::seconds timeout = std::chrono::seconds(10)) {
			if (_didLock) {
				return true;
			}
			std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

			_engine->_inSetPatcher = true;
			while (_engine->_inProcess) {
				// retry the lock
				std::chrono::time_point<std::chrono::system_clock> currenttime = std::chrono::system_clock::now();
				if (currenttime - start > timeout) {
					RNBO_ASSERT(false);
					_engine->_inSetPatcher = false;
					return false;
				}
			}
			_didLock = true;
			return true;
		}

		bool didLock() const {
			return _didLock;
		}

	private:

		Engine* _engine;
		bool _didLock;

	};

	class ProcessLocker {

	public:

		ProcessLocker(Engine* engine)
		: _engine(engine)
		, _didLock(false)
		{
			tryLock();
		}

		~ProcessLocker() {
			if (_didLock) {
				_engine->_inProcess = false;
			}
		}

		bool tryLock() {
			if (_didLock) {
				return true;  // we already locked
			}
			if (_engine->_inSetPatcher) {
				return false;	// sorry, set patcher is pending
			}
			_engine->_inProcess = true;
			if (_engine->_inSetPatcher) {
				// oops, we lost a race with setPatcher
				_engine->_inProcess = false;
				return false;
			}
			_didLock = true;
			return true;
		}

		bool didLock() const {
			return _didLock;
		}

	private:

		Engine* _engine;
		bool _didLock;

	};

	Engine::Engine()
	: _inSetPatcher(false)
	, _inProcess(false)
	{
	}

	Engine::~Engine() {
		for (auto pi : _registeredParameterInterfaces) {
			pi->deactivate();
		}
	}

	bool Engine::setPatcher(UniquePtr<PatcherInterface> patcher) {
		bool rv = patcher ? true : false;
		if (!rv) {
			patcher.reset(new NullPatcher());
		}

		SetPatcherLocker lock(this);

		if (!lock.didLock()) {
			RNBO_ASSERT(false);
			return false;
		}

		handleServiceQueue(); // ensure that the parameter interfaces are up-to-date

		RNBO_ASSERT(_patcher); // we will always start with a patcher

		PatcherState state;
		// get the old patcher state
		_patcher->getState(state);
		number previousSampleRate = _patcher->getSampleRate();
		Index previousBlockSize = _patcher->getMaxBlockSize();
		// move all scheduledEvents that have event targets to the _currentEventsList
		// they might be rescheduled by the new patcher
		CurrentEventListType& currentEvents = _currentEvents;
		_scheduledEvents.erase(remove_if(_scheduledEvents.begin(), _scheduledEvents.end(),
									   [ &currentEvents ](EventVariant& ev) {
										   if (ev.getEventTarget() != nullptr) {
											   currentEvents.addEvent(ev);
											   return true;
										   }
										   return false;
									   }),
							 _scheduledEvents.end());

		_patcher = std::move(patcher);

		_patcher->setEngineAndPatcher(this, nullptr);
		scheduleEvent(StartupEvent(_currentTime, StartupEvent::Begin));
		_patcher->initialize(state);
		scheduleEvent(StartupEvent(_currentTime, StartupEvent::End));

		_paramNameHash.update(_patcher.get());
		updateExternalDataRefs();
		_currentEvents.clear();

		// we need to update the size of the shadow value array and update shadow values in the ParameterInterfaces
		for (auto&& pi : _activeParameterInterfaces) {
			pi->refreshParameterCountAndValues();
		}

		if (previousSampleRate != 0 || previousBlockSize != 0) {
			// calling the core version directly to bypass the setPatcher lock
			EngineCore::prepareToProcess(previousSampleRate, previousBlockSize);
		}

		if (_patcherChangedHandler) {
			_patcherChangedHandler->patcherChanged();
		}

		return rv;
	}

	void Engine::updatePatcherEventTarget(const EventTarget *oldTarget, PatcherEventTarget *newTarget)
	{
		for (auto it = _currentEvents.begin(); it != _currentEvents.end(); ++it)
		{
			if (it->getEventTarget() == oldTarget) {
				it->setPatcherEventTarget(newTarget);
				_scheduledEvents.addEvent(*it);
			}
		}
	}

	void Engine::rescheduleEventTarget(const EventTarget *target)
	{
		for (auto it = _currentEvents.begin(); it != _currentEvents.end(); ++it)
		{
			if (it->getEventTarget() == target) {
				_scheduledEvents.addEvent(*it);
			}
		}
	}

	ParameterEventInterfaceUniquePtr Engine::createParameterInterface(ParameterEventInterface::Type type, EventHandler* handler) {
		if (type == ParameterEventInterface::NotThreadSafe || type == ParameterEventInterface::Trigger) {
			return EngineCore::createParameterInterface(type, handler);
		} else {
			return ParameterEventInterfaceUniquePtr(new ParameterInterfaceAsync(*this, handler, type));
		}
	}

	class ThreadIDSetter {

	public:

		ThreadIDSetter(std::thread::id& threadIDToSet)
		: _threadIDToSet(threadIDToSet)
		{
			_threadIDToSet = std::this_thread::get_id();
		}

		~ThreadIDSetter() {
			// reset to an invalid thread ID
			_threadIDToSet = std::thread::id();
		}

	private:

		std::thread::id& _threadIDToSet;

	};

	bool Engine::prepareToProcess(number sampleRate, Index maxBlockSize, bool force)
	{
		if (_inSetPatcher) return false;
		return EngineCore::prepareToProcess(sampleRate, maxBlockSize, force);
	}


	void Engine::process(const SampleValue* const* audioInputs, Index numInputs,
						 SampleValue* const* audioOutputs, Index numOutputs,
						 Index	sampleFrames,
						 const MidiEventList* midiInput, MidiEventList* midiOutput
	) {
		ProcessLocker lock(this);

		if (!lock.didLock()) {
			// TODO: we should probably clear the output buffers here, but
			// if we didn't lock it's not clear if we can write to the output buffers
			return;
		}
		else {
			ThreadIDSetter threadIDSetter(_processThreadID);		// sets _processThreadID to current thread, clears on exit

			// first handle our ServiceQueue
			handleServiceQueue();

			RNBO_ASSERT(sampleFrames > 0);
			EngineCore::process(audioInputs, numInputs,
								audioOutputs, numOutputs,
								sampleFrames,
								midiInput, midiOutput);
		}
	}

	void Engine::queueServiceEvent(const ServiceNotification& event) {
		std::lock_guard<std::mutex> lock(_serviceQueueMutex);
		_serviceQueue.enqueue(event);
	}

	size_t Engine::InitialActiveParameterInterfaces() {
		return 32;
	}

	void Engine::registerAsyncParameterInterface(ParameterEventInterfaceImplPtr pi) {
		std::lock_guard<std::mutex> lock(_registeredParameterInterfacesMutex);
		clearInactiveParameterInterfaces();
		_registeredParameterInterfaces.push_back(pi);
	}

	void Engine::clearInactiveParameterInterfaces() {
		_registeredParameterInterfaces.erase(std::remove_if(_registeredParameterInterfaces.begin(), _registeredParameterInterfaces.end(),
														 [](ParameterEventInterfaceImplPtr pi) { return !pi->isActive(); }),
										  _registeredParameterInterfaces.end());
	}

	void Engine::handleServiceQueue() {
		ServiceNotification se;
		while (_serviceQueue.dequeue(se)) {
			switch (se.getType()) {
				case ServiceNotification::ParameterInterfaceCreated:
					_activeParameterInterfaces.push_back(static_cast<ParameterEventInterfaceImpl*>(se.getPayload()));
					break;
				case ServiceNotification::ParameterInterfaceDeleted:
					_activeParameterInterfaces.erase(std::remove(_activeParameterInterfaces.begin(), _activeParameterInterfaces.end(), se.getPayload()),
											   _activeParameterInterfaces.end());
					static_cast<ParameterEventInterfaceImpl*>(se.getPayload())->deactivate();
					break;
				case ServiceNotification::ServiceNotificationUndefined:
				default:
					break;
			}
		}

	}

	bool Engine::insideProcessThread() {
		std::thread::id currentID = std::this_thread::get_id();
		bool isInsideProcess = currentID == _processThreadID ? true : false;
		return isInsideProcess;
	}

	ExternalDataIndex Engine::getNumExternalDataRefs() const {
		return ExternalDataIndex(_externalDataRefs.size());
	}

	ExternalDataId Engine::getExternalDataId(ExternalDataIndex index) const {
		if (index < 0 || index >= getNumExternalDataRefs()) return InValidExternalDataId;
		return _externalDataRefs[static_cast<Index>(index)]->getMemoryId();
	}

	const ExternalDataInfo Engine::getExternalDataInfo(ExternalDataIndex index) const {
		if (index < 0 || index >= getNumExternalDataRefs()) {
			return {
				DataType::Untyped,
				nullptr,
				nullptr
			};
		}

		Index iindex = static_cast<Index>(index);
		return {
			_externalDataRefs[iindex]->getType().type,
			_externalDataRefs[iindex]->getFile(),
			_externalDataRefs[iindex]->getTag()
		};
	}

	void Engine::setExternalData(ExternalDataId memoryId, char *data, size_t sizeInBytes, DataType type, ReleaseCallback callback) {
		auto ref = _externalDataRefMap.find(memoryId);
		if (ref != _externalDataRefMap.end()) {
			if (ref->second->getType().matches(type)) {
				ExternalDataEvent ev(memoryId, data, sizeInBytes, type, ExternalDataEvent::EventAction::SetExternalData, callback);
				_externalDataQueueIn.enqueue(ev);
			}
			else {
				ExternalDataEvent ev(memoryId, nullptr, 0, ref->second->getType(), ExternalDataEvent::EventAction::SetExternalData, callback);
			}
		}
	}

	void Engine::releaseExternalData(ExternalDataId memoryId) {
		UntypedDataBuffer untyped;
		ExternalDataEvent ev(memoryId, nullptr, 0, untyped, ExternalDataEvent::EventAction::ReleaseExternalData, nullptr);
		_externalDataQueueIn.enqueue(ev);
	}

	void Engine::setExternalDataHandler(ExternalDataHandler* handler) {
		_externalDataHandler = handler;
	}

	void Engine::setPresetSync(UniquePresetPtr preset) {
		ProcessLocker lock(this);

		if (!lock.didLock()) {
			RNBO_ASSERT(false);
		}
		else {
			EngineCore::setPresetSync(std::move(preset));
		}
	}

	ConstPresetPtr Engine::getPresetSync() {
		ProcessLocker lock(this);

		if (!lock.didLock()) {
			RNBO_ASSERT(false);
			// return an empty preset
			PresetPtr preset = std::make_shared<Preset>();
			return preset;
		}
		else {
			return EngineCore::getPresetSync();
		}
	}

	void Engine::updateExternalDataRefs() {
		// we are at a very specific point in time, when rebuilding the patcher
		// so we can actually access the DataRefs directly
		RNBO_ASSERT(!_inAudioProcess && !_inProcess);

		DataRefIndex numDataRefs = _patcher->getNumDataRefs();

		for (auto ref = _externalDataRefMap.begin(); ref != _externalDataRefMap.end(); ref++) {
			ref->second->invalidate();
		}

		_externalDataRefs.clear();
		_externalDataRefs.reserve(static_cast<size_t>(numDataRefs));

		// iterate through all DataRefs the patcher exposes
		for (DataRefIndex i = 0; i < numDataRefs; i++) {
			DataRef* ref = _patcher->getDataRef(i);

			if (!ref->isInternal() && ref->getName()) {
				auto it = _externalDataRefMap.find(ref->getName());
				if (it != _externalDataRefMap.end()) {
					it->second->revalidate(i, ref);
				}
				else {
					bool inserted;
					tie(it, inserted) = _externalDataRefMap.insert({ ref->getName(), std::make_shared<ExternalDataRef>(i, ref) });
				}

				_externalDataRefs.push_back(it->second.get());
			}
		}

		for (auto ref = _externalDataRefMap.begin(); ref != _externalDataRefMap.end();) {
			if (!ref->second->isValid()) {
				auto callback = ref->second->getCallback();
				if (callback && ref->second->getData()) {
					callback(ref->second->getMemoryId(), ref->second->getData());
				}
				ref = _externalDataRefMap.erase(ref);
			}
			else
				ref++;
		}

		RNBO_ASSERT(_externalDataRefs.size() == _externalDataRefMap.size());
	}

	void Engine::beginProcessDataRefs() {
		if (_externalDataHandler && !_externalDataRefs.empty()) {
			_externalDataHandler->processBeginCallback(static_cast<DataRefIndex>(_externalDataRefs.size()), _externalDataRefs.data(),
				   [this](DataRefIndex index, char* data, size_t sizeInBytes, DataType type) {
						RNBO_ASSERT(index >= 0 && static_cast<size_t>(index) < _externalDataRefs.size());
						auto externalRef = _externalDataRefs[static_cast<size_t>(index)];
						if (externalRef->getType().matches(type)) {
							externalRef->updateDataRef(data, sizeInBytes, type);
						}
						else {
							externalRef->updateDataRef(nullptr, 0);
						}
						scheduleEvent(DataRefEvent(externalRef->getInternalIndex(), _currentTime, DataRefEvent::UpdateDataRef, _patcher->getPatcherEventTarget()));
				   },
				   [this](DataRefIndex index) {
						Index iindex = static_cast<Index>(index);
						RNBO_ASSERT(index >= 0 && iindex < _externalDataRefs.size());
						auto externalRef = _externalDataRefs[iindex];
						externalRef->updateDataRef(nullptr, 0);
						scheduleEvent(DataRefEvent(externalRef->getInternalIndex(), _currentTime, DataRefEvent::UpdateDataRef, _patcher->getPatcherEventTarget()));
				   }
			);
		}

		ExternalDataEvent ev;
		while (_externalDataQueueIn.dequeue(ev)) {
			auto it = _externalDataRefMap.find(ev.getMemoryId());
			if (it != _externalDataRefMap.end()) {
				auto ref = it->second;
				if (ev.getAction() == ExternalDataEvent::EventAction::SetExternalData) {
					ref->updateDataRef(ev.getData(), ev.getSizeInBytes(), ev.getType(), ev.getCallback());
				}
				else if (ev.getAction() == ExternalDataEvent::EventAction::ReleaseExternalData) {
					ref->updateDataRef(nullptr, 0, nullptr);
				}

				scheduleEvent(DataRefEvent(ref->getInternalIndex(), _currentTime, DataRefEvent::UpdateDataRef, _patcher->getPatcherEventTarget()));
			}
		}
	}

	void Engine::endProcessDataRefs() {
		if (_externalDataHandler && !_externalDataRefs.empty()) {
			_externalDataHandler->processEndCallback(static_cast<DataRefIndex>(_externalDataRefs.size()), _externalDataRefs.data());

			for (auto ref : _externalDataRefs) {
				ref->setTouched(false);
			}
		}
	}

}  // namespace RNBO

