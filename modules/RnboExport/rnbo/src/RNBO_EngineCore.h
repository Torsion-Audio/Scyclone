#ifndef _RNBO_EngineCore_H_
#define _RNBO_EngineCore_H_

#include <type_traits>

#include "RNBO_Types.h"
#include "RNBO_EngineInterface.h"
#include "RNBO_ParameterEventInterface.h"
#include "RNBO_ParameterInterfaceSync.h"
#include "RNBO_ProbingInterface.h"
#include "RNBO_EventList.h"
#include "RNBO_EventVariant.h"
#include "RNBO_MidiEvent.h"
#include "RNBO_PatcherInterface.h"
#include "RNBO_TimeConverter.h"
#include "RNBO_EngineLink.h"
#include "RNBO_NullPatcher.h"
#include "RNBO_PatcherStateDummy.h"
#include "RNBO_PatcherEventSender.h"
#include "RNBO_Utils.h"
#include "RNBO_StartupEvent.h"
#include "RNBO_ParamNameHash.h"

namespace RNBO {

	class PatcherEventTarget;

	/**
	 * @private
	 *
	 * NOOP/patcher passthrough engine
	 */
	class EngineCore : public EngineInterface,
					   public ParameterEventInterface,
					   public ProbingInterface
	{

	public:

		using CurrentEventListType = EventList<EventVariant>;

		EngineCore()
		: _patcher(new NullPatcher())
		, _currentTime(0)
		, _inAudioProcess(false)
		{
			// prevent allocation on audio thread
			_activeParameterInterfaces.reserve(InitialActiveParameterInterfaces());
		}

		~EngineCore() override {}

		virtual PatcherInterface& getPatcher() const { return *_patcher; }

		virtual bool prepareToProcess(number sampleRate, Index maxBlockSize, bool force = false)
		{
			// at the moment we hard-wire the _minSchedInterval to be 1 sample, we might want to change that in the future
			TimeConverter timeConverter(sampleRate, _currentTime);
			_minSchedInterval = timeConverter.convertSampleIndexToMilliseconds(1);

			_patcher->prepareToProcess(sampleRate, maxBlockSize, force);

			return true;
		}

		Index getNumInputChannels() const {
			return _patcher->getNumInputChannels();
		}

		Index getNumOutputChannels() const {
			return _patcher->getNumOutputChannels();
		}

		ParameterIndex getNumSignalInParameters() const
		{
			return _patcher->getNumSignalInParameters();
		}

		ParameterIndex getNumSignalOutParameters() const
		{
			return _patcher->getNumSignalOutParameters();
		}

		// TODO: required by CoreObject, but not in any interface
		number getSampleRate() { return _patcher->getSampleRate(); }
		Index getSamplesPerBlock() { return _patcher->getMaxBlockSize(); }

		// EngineInterface

		void scheduleClockEvent(EventTarget* eventTarget, ClockId clockIndex, MillisecondTime time) override {
			scheduleEvent(ClockEvent(clockIndex, time, eventTarget));
			if (_scheduleCallback) _scheduleCallback(time);
		}

		void scheduleClockEventWithValue(EventTarget* eventTarget, ClockId clockIndex, MillisecondTime time, ParameterValue value) override {
			scheduleEvent(ClockEvent(clockIndex, time, value, eventTarget));
			if (_scheduleCallback) _scheduleCallback(time);
		}

		void flushClockEvents(EventTarget* eventTarget, ClockId clockIndex, bool execute) override
		{
			doFlushClockEvents(execute, [eventTarget, clockIndex](const EventVariant& ev) {
				return ev.isClockEvent() && ev.getClockEvent().matchesEventTargetAndClockIndex(eventTarget, clockIndex);
			});
		}

		void flushClockEventsWithValue(EventTarget* eventTarget, ClockId clockIndex, ParameterValue value, bool execute) override
		{
			doFlushClockEvents(execute, [eventTarget, clockIndex, value](const EventVariant& ev) {
				return ev.isClockEvent() && ev.getClockEvent().matchesEventTargetClockIndexAndValue(eventTarget, clockIndex, value);
			});
		}

		// remove all clock events from the scheduler, optionally executing the clocks
		template<class Predicate>
		void doFlushClockEvents(bool execute, Predicate pred) {
			PatcherEventSender sender(_patcher.get(), getCurrentTime());

			if (_eventContext) {
				auto nextEvent = next(_eventContext->_currentEvent);

				_currentEvents.erase(remove_if(nextEvent, _currentEvents.end(),
													[execute, sender, pred](const EventVariant& ev) {
														// TODO: avoid copy in getClockEvent()?
														bool matches = pred(ev);
														if (execute && matches) {
															sender.sendEvent(ev);
														}
														return matches;
													}),
									 _currentEvents.end());
			}

			int i = 0;
			for (auto it = _scheduledEvents.begin(); it != _scheduledEvents.end(); ) {
				const EventVariant& ev = *it;
				bool matches = pred(ev);
				if (matches) {
					if (execute) {
						sender.sendEvent(ev);
					}
					_scheduledEvents.erase(it);
				}
				else {
					i++;
				}
				it = _scheduledEvents.begin() + i; // just to be safe
			}
		}

		void doSendMidiEvent(MidiEvent event)
		{
			if (_midiOutput) _midiOutput->addEvent(event);
			sendOutgoingEvent(event);
		}

		void sendMidiEvent(int port, int b1, int b2, int b3) override
		{
			doSendMidiEvent(MidiEvent(_currentTime, port, b1, b2, b3));
		}

		void sendMidiEventList(int port, const list& data) override
		{
			size_t length = data.length;
			size_t i;

			for (i = 0; i + 2 < length; i += 3) {
				uint8_t bytes[] = {
					static_cast<uint8_t>(data[i]),
					static_cast<uint8_t>(data[i + 1]),
					static_cast<uint8_t>(data[i + 2]),
				};
				doSendMidiEvent(MidiEvent(_currentTime, port, bytes, 3));
			}
			if (i < length) {
				length -= i;
				uint8_t bytes[] = {
					static_cast<uint8_t>(data[i]),
					static_cast<uint8_t>(length > 1 ? data[i + 1] : 0),
					static_cast<uint8_t>(length > 2 ? data[i + 2] : 0),
				};
				// length is always <= 3 here, so it fits into an int
				doSendMidiEvent(MidiEvent(_currentTime, port, bytes, length));
			}
		}

		void sendOutlet(EngineLink* sender, OutletIndex index, ParameterValue value, SampleOffset sampleOffset) override {
			RNBO_ASSERT(sender->getPatcherEventTarget());

			if (_inAudioProcess) {
				// we are currently processing audio, we need to schedule this for the next audio vector
				TimeConverter timeConverter(_patcher->getSampleRate(), _currentTime);
				MillisecondTime timeToSchedule = timeConverter.convertSampleOffsetToMilliseconds(SampleOffset(_patcher->getMaxBlockSize()) + sampleOffset);
				scheduleEvent(OutletEvent(timeToSchedule, sender, index, value, sender->getPatcherEventTarget()));
			} else {
				// we are inside event processing, immediately call back into the patcher
				sender->getPatcherEventTarget()->processOutletAtCurrentTime(sender, index, value);
			}
		}

		void sendDataRefUpdated(DataRefIndex index) override {
			auto currentEvent = getCurrentEvent();
			if (currentEvent) {
				_patcher->processDataViewUpdate(index, (*currentEvent)->getTime());
			}
			else {
				scheduleEvent(DataRefEvent(index, _currentTime, DataRefEvent::UpdateDataRef, _patcher->getPatcherEventTarget()));
			}
		}

		void sendOutgoingEvent(EventVariant event) {
			for (auto pi : _activeParameterInterfaces) {
				pi->pushOutgoingEvent(event);
			}
		}

		void sendNumMessage(MessageTag tag, MessageTag objectId, number payload, MillisecondTime time) override {
#ifndef RNBO_NOMESSAGEEVENT
			sendOutgoingEvent(MessageEvent(tag, time, payload, objectId, _patcher->getPatcherEventTarget()));
#endif
		}

		void sendListMessage(MessageTag tag, MessageTag objectId, const list& payload, MillisecondTime time) override {
#ifndef RNBO_NOMESSAGEEVENT
			for (auto pi : _activeParameterInterfaces) {
				// we deliberately do not use sendOutgoingEvent because we want to create a unique list
				// for each parameter interface
				UniqueListPtr tmp = RNBO::make_unique<list>(payload);
				pi->pushOutgoingEvent(MessageEvent(tag, time, std::move(tmp), objectId, _patcher->getPatcherEventTarget()));
			}
#endif
		}

		void sendBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) override {
#ifndef RNBO_NOMESSAGEEVENT
			for (auto pi : _activeParameterInterfaces) {
				pi->pushOutgoingEvent(MessageEvent(tag, time, objectId, _patcher->getPatcherEventTarget()));
			}
#endif
		}

		void sendTempoEvent(Tempo tempo) override {
			for (auto pi : _activeParameterInterfaces) {
				pi->pushOutgoingEvent(TempoEvent(_currentTime, tempo));
			}
		}

		void sendTransportEvent(TransportState state) override {
			for (auto pi : _activeParameterInterfaces) {
				pi->pushOutgoingEvent(TransportEvent(_currentTime, (TransportState)state));
			}
		}

		void sendBeatTimeEvent(BeatTime beattime) override {
			for (auto pi : _activeParameterInterfaces) {
				pi->pushOutgoingEvent(BeatTimeEvent(_currentTime, beattime));
			}
		}

		void sendTimeSignatureEvent(int numerator, int denominator) override {
			for (auto pi : _activeParameterInterfaces) {
				pi->pushOutgoingEvent(TimeSignatureEvent(_currentTime, numerator, denominator));
			}
		}

		void notifyParameterValueChanged(ParameterEvent sourceEvent, ParameterIndex index, ParameterValue value) {
			for (auto pi : _activeParameterInterfaces) {
				if (_settingPreset) {
					pi->pushOutgoingEvent(ParameterEvent(index, _currentTime, value, sourceEvent.getSource()));
				}
				else {
					pi->notifyParameterValueChanged(sourceEvent, index, value);
				}
			}
		}

		void notifyParameterValueChanged(ParameterIndex index, ParameterValue value, bool ignoreSource) override {
			// Log() << "notifyParameterValueChanged: index=" << index << "value=" << value;
			CurrentEventListType::Iterator *currentEvent = getCurrentEvent();
			ParameterEvent sourceEvent = currentEvent && (*currentEvent)->isParameterEvent() && !ignoreSource ? (*currentEvent)->getParameterEvent() : ParameterEvent();
			notifyParameterValueChanged(sourceEvent, index, value);
		}

		void scheduleParameterChange(ParameterIndex index, ParameterValue value, MillisecondTime offset) override {
			ParameterEvent pe(index, getCurrentTime() + offset, value, nullptr);
			scheduleEvent(pe);
		}

		void updatePatcherEventTarget(const EventTarget *oldTarget, PatcherEventTarget *newTarget) override {
			RNBO_UNUSED(oldTarget)
			RNBO_UNUSED(newTarget)
			// the core engine will just clear all scheduled events and there is no
			// state restoration which should ever call into here
			RNBO_ASSERT(false);
		}

		void rescheduleEventTarget(const EventTarget *target) override {
			RNBO_UNUSED(target)
			// the core engine will just clear all scheduled events and there is no
			// state restoration which should ever call into here
			RNBO_ASSERT(false);
		}

		void notifyOutgoingEvents() {
			for (auto pi : _activeParameterInterfaces) {
				pi->pushDirtyParameters(_currentTime);
				pi->notifyOutgoingEvents();
			}
		}

		MillisecondTime getCurrentTime() override { return _currentTime; }
		void setCurrentTime(MillisecondTime time) { _currentTime = time; }

		CurrentEventListType& getCurrentEvents() { return _currentEvents; }

		// ParameterInterface

		Index getNumMidiInputPorts() const {
			return _patcher->getNumMidiInputPorts();
		}

		Index getNumMidiOutputPorts() const {
			return _patcher->getNumMidiOutputPorts();
		}

		ParameterIndex getNumParameters() const override {
			return _patcher->getNumParameters();
		}

		ConstCharPointer getParameterName(ParameterIndex index) const override {
			return _patcher->getParameterName(index);
		}

		ConstCharPointer getParameterId(ParameterIndex index) const override {
			return _patcher->getParameterId(index);
		}

		void getParameterInfo(ParameterIndex index, ParameterInfo* info) const override {
			_patcher->getParameterInfo(index, info);
		}

		ParameterValue getParameterValue(ParameterIndex index) override {
			return _patcher->getParameterValue(index);
		}

		void setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time = RNBOTimeNow) override {
			RNBO_UNUSED(index)
			RNBO_UNUSED(value)
			RNBO_UNUSED(time)
			// do not use the engine directly - use it via the CoreObject !
			RNBO_ASSERT(false);
		}

		ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const override
		{
			return _patcher->convertToNormalizedParameterValue(index, value);
		}

		ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const override
		{
			return _patcher->convertFromNormalizedParameterValue(index, normalizedValue);
		}

		ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const override
		{
			return _patcher->constrainParameterValue(index, value);
		}

		MessageTagInfo resolveTag(MessageTag tag) const
		{
			return _patcher->resolveTag(tag);
		}

		MessageIndex getNumMessages() const
		{
			return _patcher->getNumMessages();
		}

		const MessageInfo& getMessageInfo(MessageIndex index) const
		{
			return _patcher->getMessageInfo(index);
		}

		virtual bool setPatcher(UniquePtr<PatcherInterface> patcher) {
			if (!patcher) {
				patcher.reset(new NullPatcher());
			}

			// first erase all events with an EventTarget, they not be valid anymore in the new patcher
			_scheduledEvents.erase(remove_if(_scheduledEvents.begin(), _scheduledEvents.end(),
											 [](const EventVariant& ev) {
												 return ev.getEventTarget() != nullptr;
											 }),
								   _scheduledEvents.end());

			_patcher = std::move(patcher);

			PatcherStateDummy emptyState;
			_patcher->setEngineAndPatcher(this, nullptr);
			scheduleEvent(StartupEvent(_currentTime, StartupEvent::Begin));
			_patcher->initialize(emptyState);
			scheduleEvent(StartupEvent(_currentTime, StartupEvent::End));

			_paramNameHash.update(_patcher.get());

			if (_patcherChangedHandler) {
				_patcherChangedHandler->patcherChanged();
			}

			return true;
		}

		virtual size_t InitialActiveParameterInterfaces() {
			return 4;
		}

		virtual ParameterEventInterfaceUniquePtr createParameterInterface(ParameterEventInterface::Type type, EventHandler* handler) {
			RNBO_ASSERT(type == ParameterEventInterface::NotThreadSafe || type == ParameterEventInterface::Trigger);  // only support NotThreadSafe for now
			if (type == ParameterEventInterface::Trigger) {
				return ParameterEventInterfaceUniquePtr(new ParameterInterfaceTrigger(*this, handler));
			}
			else {
				return ParameterEventInterfaceUniquePtr(new ParameterInterfaceSync(*this, handler));
			}
		}

		void addToActiveInterfaces(ParameterEventInterfaceImpl* pi) {
			_activeParameterInterfaces.push_back(pi);
		}
		void removeFromActiveInterfaces(ParameterEventInterfaceImpl* pi) {
			_activeParameterInterfaces.erase(
					remove(_activeParameterInterfaces.begin(),
						   _activeParameterInterfaces.end(), pi),
					_activeParameterInterfaces.end());
		}

		// ParameterEventInterface

		void scheduleEvent(EventVariant event) override { // TODO: rewrite using reference to actual event, rather than a copy -- modify in place possible?
			// you must not schedule to the past
			// scheduling outside an event context is okay, so _eventContext can be nullptr
			RNBO_ASSERT(!_eventContext || event.getTime() >= _eventContext->_currentEvent->getTime());

			// for clock events we need to take care that we do not schedule faster than our minimum scheduling interval
			// since we cannot look at the events from the last audio buffer, this algorithm will
			// only reliably work for a min scheduling interval of 1 sample, which is hard-wired for now
			if (event.isClockEvent() && _eventContext && event.getTime() - _eventContext->_currentEvent->getTime() < _minSchedInterval) {
				ClockEvent ce = event.getClockEvent();
				// get a reverse iterator from the current event
				CurrentEventListType::ReverseIterator rit(_eventContext->_currentEvent);
				// the reverse iterator will point to the event before our current event, but we want to start
				// on the current event, so decrease it
				--rit;

				// now look for clock events that have the same index as ours and are closer than the min scheduling
				// interval, if you find one, adapt our time and break
				while (rit != _currentEvents.rend() && event.getTime() - rit->getTime() <= _minSchedInterval) {
					// TODO: avoid copy in getClockEvent()?
					if (rit->isClockEvent() && rit->getClockEvent().matchesEventTargetAndClockIndex(ce)) {
						// modify the event
						event.setTime(rit->getTime() + _minSchedInterval);
						break;
					}

					++rit;
				}
			}

			// either we have to schedule into our current audio buffer, or into the future buffers
			if (_eventContext && event.getTime() <= _eventContext->_currentBufferEnd) {
				// we are currently iterating, so we need to save the current index to
				// re-validate the iterator
				auto currentIndex = _eventContext->_currentEvent - _currentEvents.begin();
				_currentEvents.addEvent(event);
				_eventContext->setIteratorTo(currentIndex);
			} else {
				_scheduledEvents.addEvent(event);
			}
		}

		// ProbingInterface
		ParameterIndex getParameterIndexForID(const char *paramid) const {
			return _paramNameHash.get(paramid);
		}

		Index getProbingChannels(MessageTag outletId) const override {
			return _patcher->getProbingChannels(outletId);
		}

		void processEventsUntil(MillisecondTime endTime)
		{
			if (endTime < _currentTime) endTime = _currentTime;

			// then we move all events that fit into our signal vector from _scheduledEvents to our _currentEvents list
			// maybe this is not an ideal design at the moment, we should test if it is faster to iterate directly over
			// the _scheduledEvents list
			moveScheduledEventsToCurrent(endTime);

			// iterate and process each event on _currentEvents and send into the patcher
			PatcherEventSender sender(_patcher.get(), getCurrentTime());

			for (auto it = _currentEvents.begin(); it != _currentEvents.end(); it++) {
				RNBO_ASSERT(_eventContext == nullptr);
				EventContext eventContext(*this, endTime, it);
				if (it->getType() == Event::Preset) {
					handlePresetEvent(it->getPresetEvent());
				}
				else if (it->getType() == Event::Startup) {
					sendOutgoingEvent(it->getStartupEvent());
					_settingPreset = it->getStartupEvent().getType() == StartupEvent::Begin;
				}
				else {
					sender.sendEvent(*it);
				}
			}

			// done with current events, clear them out
			_currentEvents.clear();
		}

		virtual void process(const SampleValue* const* audioInputs, Index numInputs,
							 SampleValue* const* audioOutputs, Index numOutputs,
							 Index sampleFrames,
							 const MidiEventList* midiInput = nullptr, MidiEventList* midiOutput = nullptr)
		{
			TimeConverter timeConverter(_patcher->getSampleRate(), _currentTime);
			MillisecondTime endTime = timeConverter.convertSampleOffsetToMilliseconds(SampleOffset(sampleFrames));

			if (sampleFrames == _patcher->getMaxBlockSize() || !_patcher->hasFixedVectorSize()) {
				// note that we want to avoid memory allocations from the audio thread
				// this is tricky so it is good to test that malloc isn't really being called
				// one way to do this is with a thread-specific breakpoint on malloc
				// first break somewhere around here, see what thread it is, and then in xcode's debugger you can type:
				//    breakpoint set -x <thread index> -b malloc
				// replace <thread index> with the thread number shown in the xcode debugger call stack panel

				beginProcessDataRefs();

				// take midiInput events and fill them into _scheduledEvents
				if (midiInput) {
					for (MidiEventList::ConstIterator it = midiInput->begin(); it != midiInput->end(); it++) {
						_scheduledEvents.addEvent(MidiEvent(*it));
					}
				}
				_midiOutput = midiOutput;

				// get all events from the queues and schedule them to _scheduledEvents
				drainParameterQueues();

				processEventsUntil(endTime);

				processCore(audioInputs, numInputs, audioOutputs, numOutputs, sampleFrames);

				notifyOutgoingEvents();
				endProcessDataRefs();
			}
			else {
				// we have a non matching vector size, just zero out the outputs
				AudioBufferConverterBase::clearAudioBuffers(audioOutputs, numOutputs, sampleFrames);
			}

			_midiOutput = nullptr;

			// advance time
			_currentTime = endTime;
		}

		void setPatcherChangedHandler(PatcherChangedHandler* handler)
		{
			_patcherChangedHandler = handler;
		}

		void handlePresetEvent(const PresetEvent& pe) {
#ifndef RNBO_NOPRESETS
			switch (pe.getType()) {
				case PresetEvent::Get: {
					PresetPtr preset = std::make_shared<Preset>();
					_patcher->getPreset(*preset);
					auto callback = pe.getCallback();
					callback(preset);
					break;
				}
				case PresetEvent::Set:
					// since we are touching the last preset with this restore and do
					// not need to notify the outside world, set it as touched
					_settingPreset = true;
					sendOutgoingEvent(PresetEvent(pe.getTime(), PresetEvent::SettingBegin));
					_patcher->setPreset(pe.getTime(), *pe.getPreset());
					sendOutgoingEvent(PresetEvent(pe.getTime(), PresetEvent::SettingEnd));
					_settingPreset = false;
					break;
				case PresetEvent::Invalid:
				case PresetEvent::Touched:
				case PresetEvent::SettingBegin:
				case PresetEvent::SettingEnd:
				case PresetEvent::Max_Type:
				default:
					// incoming preset events have to be of type get or set
					RNBO_ASSERT(false);
			}

#endif
		}

		virtual void setPresetSync(UniquePresetPtr preset)
		{
			_settingPreset = true;
			sendOutgoingEvent(PresetEvent(_currentTime, PresetEvent::SettingBegin));
			_patcher->setPreset(_currentTime, *preset);
			sendOutgoingEvent(PresetEvent(_currentTime, PresetEvent::SettingEnd));
			_settingPreset = false;
		}

		virtual ConstPresetPtr getPresetSync() {
			PresetPtr preset = std::make_shared<Preset>();
			_patcher->getPreset(*preset);
			return preset;
		}

		void presetTouched() override {
			if (!_settingPreset) {
				sendOutgoingEvent(PresetEvent(_currentTime, PresetEvent::Touched, nullptr, nullptr));
			}
		}

		virtual void beginProcessDataRefs() {}
		virtual void endProcessDataRefs() {}

		void setScheduleCallback(ScheduleCallback callback) override {
			_scheduleCallback = callback;
		}

	protected:

		/**
		 * @private
		 *
		 * helper class to hold information about the currently processed event
		 */
		struct EventContext {
			EventContext(EngineCore& engine, MillisecondTime currentBufferEnd, CurrentEventListType::Iterator& currentEvent)
			: _engine(engine)
			, _currentBufferEnd(currentBufferEnd)
			, _currentEvent(currentEvent)
			{
				_engine.setEventContext(this);
			}

			~EventContext() {
				_engine.setEventContext(nullptr);
			}

			void setIteratorTo(long index) {
				_currentEvent = _engine.getCurrentEvents().begin() + index;
			}

			EngineCore&						_engine;
			MillisecondTime					_currentBufferEnd;
			CurrentEventListType::Iterator&	_currentEvent;
		};

		CurrentEventListType::Iterator* getCurrentEvent() { return _eventContext ? &_eventContext->_currentEvent : nullptr; }

		UniquePtr<PatcherInterface>		_patcher;

		MillisecondTime					_currentTime;

		// _currentEvents is a time-ordered list of events to feed into the patcher
		// events include parameter value changes, midi, and clock events
		// using one heterogeneous ordered list insures events are delivered with proper sequencing
		CurrentEventListType			_currentEvents;
		// we keep this list to track the patcher's scheduled clocks
		CurrentEventListType			_scheduledEvents;

		// only to be used from audio thread
		bool							_inAudioProcess;

#ifdef USE_STD_VECTOR
		std::vector<ParameterEventInterfaceImpl*>	_activeParameterInterfaces;
#else
		Vector<ParameterEventInterfaceImpl*>		_activeParameterInterfaces;
#endif

		PatcherChangedHandler*			_patcherChangedHandler = nullptr;
		ParamNameHash					_paramNameHash;

	private:

		void setEventContext(EventContext *context) {
			_eventContext = context;
		}

		// this is just here to satisfy the need for ParameterEventInterface, not needed in the engine
		void drainEvents() override {
			// makes no sense to call this in the engine
			RNBO_ASSERT(false);
		}

		void drainParameterQueues() {
			for (auto pi : _activeParameterInterfaces) {
				pi->drainIncomingQueueToEventList(_scheduledEvents, _currentTime);
			}
		}

		void moveScheduledEventsToCurrent(MillisecondTime endTime) {
			if (!_scheduledEvents.empty()) {
				const auto target = _scheduledEvents.getFirstEventAfterTime(endTime);
				for (auto it = _scheduledEvents.begin(); it != target; it++) {
					_currentEvents.addEvent(EventVariant(*it));
				}
				_scheduledEvents.erase(_scheduledEvents.begin(), target);
			}
		}

		void processCore(const SampleValue* const* audioInputs, Index numInputs,
						 SampleValue* const* audioOutputs, Index numOutputs,
						 Index sampleFrames)
		{
			_inAudioProcess = true;

			const Index numIns = _patcher->getNumInputChannels() + _patcher->getNumSignalInParameters();
			const Index numOuts = _patcher->getNumOutputChannels() + _patcher->getNumSignalOutParameters();

			Index inChansToProcess = numInputs <= numIns ? numInputs : numIns;
			Index outChansToProcess = numOutputs <= numOuts ? numOutputs : numOuts;

			_patcher->process(audioInputs,
							  inChansToProcess,
							  audioOutputs,
							  outChansToProcess,
							  sampleFrames);

			// clear unused audio buffers
			AudioBufferConverterBase::clearAudioBuffers(audioOutputs, numOutputs, sampleFrames, numOuts);

			_inAudioProcess = false;
		}

		// the minimum distance of a clock being scheduled
		MillisecondTime					_minSchedInterval = 0.02267573696; // one sample at 44100
		// each time an event a scheduled event is executed, we set it in a context
		EventContext*					_eventContext = nullptr;
		// the generated code can notify when a value that is preset relevant changed
		bool							_settingPreset = false;

		MidiEventList* 					_midiOutput = nullptr;

		ScheduleCallback				_scheduleCallback = nullptr;
	};

} // namespace RNBO

#endif // #ifndef _RNBO_EngineCore_H_
