#include "RNBO_Config.h"
#include "RNBO_CoreObject.h"
#ifdef RNBO_SIMPLEENGINE
#include "RNBO_EngineCore.h"
#else
#include "RNBO_Engine.h"
#endif
#include "RNBO_PatcherFactory.h"

#include <cassert>

#ifdef USE_DYNAMIC_COMPILATION

#include "RNBO_FileChangeWatcher.h"
#include "RNBO_DynamicPatcherFactory.h"

#endif

#include "RNBO_PresetEvent.h"

namespace RNBO {

	CoreObject::CoreObject(EventHandler* handler)
	{
		initializeEngine(handler);
		setPatcher();
	}

	CoreObject::CoreObject(UniquePtr<PatcherInterface> patcher, EventHandler* handler)
	{
		initializeEngine(handler);
		setPatcher(std::move(patcher));
	}

	CoreObject::~CoreObject()
	{

	}

ParameterIndex CoreObject::getNumParameters() const
	{
		return _engine->getNumParameters();
	}

	ConstCharPointer CoreObject::getParameterName(ParameterIndex index) const
	{
		return _engine->getParameterName(index);
	}

	ConstCharPointer CoreObject::getParameterId(ParameterIndex index) const
	{
		return _engine->getParameterId(index);
	}

	void CoreObject::getParameterInfo(ParameterIndex index, ParameterInfo* info) const
	{
		_engine->getParameterInfo(index, info);
	}

	ParameterValue CoreObject::getParameterValue(ParameterIndex index)
	{
		return _parameterInterface->getParameterValue(index);
	}

	void CoreObject::setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time)
	{
		_parameterInterface->setParameterValue(index, value, time);
	}

	void CoreObject::setParameterValueNormalized(ParameterIndex index, ParameterValue normalizedValue, MillisecondTime time)
	{
		_parameterInterface->setParameterValueNormalized(index, normalizedValue, time);
	}

	ParameterValue CoreObject::convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const
	{
		return _parameterInterface->convertToNormalizedParameterValue(index, value);
	}

	ParameterValue CoreObject::convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const
	{
		return _parameterInterface->convertFromNormalizedParameterValue(index, normalizedValue);
	}

	ParameterValue CoreObject::constrainParameterValue(ParameterIndex index, ParameterValue value) const
	{
		return _parameterInterface->constrainParameterValue(index, value);
	}

	ParameterIndex CoreObject::getParameterIndexForID(ConstCharPointer paramid) const
	{
		return _engine->getParameterIndexForID(paramid);
	}

	Index CoreObject::getProbingChannels(MessageTag outletId) const
	{
		return _engine->getProbingChannels(outletId);
	}

	Index CoreObject::getNumInputChannels() const
	{
		return _engine->getNumInputChannels();
	}

	Index CoreObject::getNumOutputChannels() const
	{
		return _engine->getNumOutputChannels();
	}

	ParameterIndex CoreObject::getNumSignalInParameters() const
	{
		return _engine->getNumSignalInParameters();
	}

	ParameterIndex CoreObject::getNumSignalOutParameters() const
	{
		return _engine->getNumSignalOutParameters();
	}

	number CoreObject::getSampleRate() const
	{
		return _engine->getSampleRate();
	}

	Index CoreObject::getSamplesPerBlock() const
	{
		return _engine->getSamplesPerBlock();
	}

	MillisecondTime CoreObject::getCurrentTime()
	{
		return _engine->getCurrentTime();
	}

	void CoreObject::setCurrentTime(MillisecondTime time)
	{
		_engine->setCurrentTime(time);
	}

	bool CoreObject::prepareToProcess(number sampleRate, Index maxBlockSize, bool force)
	{
		return _engine->prepareToProcess(sampleRate, maxBlockSize, force);
	}

	void CoreObject::process(const SampleValue* const* audioInputs, Index numInputs,
				 SampleValue* const* audioOutputs, Index numOutputs,
				 Index sampleFrames,
				 const MidiEventList* midiInput, MidiEventList* midiOutput)
	{
		_engine->process(audioInputs, numInputs,
			audioOutputs, numOutputs,
			sampleFrames,
			midiInput, midiOutput);
	}

#ifndef RNBO_FEATURE_NO_SETPATCHER
	void CoreObject::setPatcherChangedHandler(PatcherChangedHandler* handler)
	{
		_engine->setPatcherChangedHandler(handler);
	}
#endif

	ParameterEventInterfaceUniquePtr CoreObject::createParameterInterface(ParameterEventInterface::Type type, EventHandler* handler)
	{
		return _engine->createParameterInterface(type, handler);
	}

	Index CoreObject::getNumMidiInputPorts() const
	{
		return _engine->getNumMidiInputPorts();
	}

	Index CoreObject::getNumMidiOutputPorts() const
	{
		return _engine->getNumMidiOutputPorts();
	}

	void CoreObject::scheduleEvent(EventVariant event)
	{
		_parameterInterface->scheduleEvent(event);
	}

	void CoreObject::sendMessage(MessageTag tag, number payload, MessageTag objectId, MillisecondTime eventTime)
	{
#ifdef RNBO_NOMESSAGEEVENT
		RNBO_ASSERT(false); // not supported without std:lib
#else
		_parameterInterface->sendMessage(tag, payload, objectId, eventTime);
#endif
	}

	void CoreObject::sendMessage(MessageTag tag, UniqueListPtr payload, MessageTag objectId, MillisecondTime eventTime)
	{
#ifdef RNBO_NOMESSAGEEVENT
		RNBO_ASSERT(false); // not supported without std:lib
#else
		_parameterInterface->sendMessage(tag, std::move(payload), objectId, eventTime);
#endif
	}

	void CoreObject::sendMessage(MessageTag tag, MessageTag objectId, MillisecondTime eventTime)
	{
#ifdef RNBO_NOMESSAGEEVENT
		RNBO_ASSERT(false); // not supported without std:lib
#else
		_parameterInterface->sendMessage(tag, objectId, eventTime);
#endif
	}

	MessageTagInfo CoreObject::resolveTag(MessageTag tag) const
	{
		return _engine->resolveTag(tag);
	}

	MessageIndex CoreObject::getNumMessages() const
	{
		return _engine->getNumMessages();
	}

	const MessageInfo& CoreObject::getMessageInfo(MessageIndex index) const
	{
		return _engine->getMessageInfo(index);
	}

	void CoreObject::setPreset(UniquePresetPtr preset)
	{
#ifdef RNBO_NOPRESETS
		RNBO_ASSERT(false); // not supported without std:lib
#else
		_parameterInterface->scheduleEvent(
			PresetEvent(RNBOTimeNow, PresetEvent::Set, std::move(preset), nullptr));
#endif
	}

	void CoreObject::setPresetSync(UniquePresetPtr preset)
	{
#ifdef RNBO_NOPRESETS
		RNBO_ASSERT(false); // not supported without std:lib
#else
		return _engine->setPresetSync(std::move(preset));
#endif
	}

	void CoreObject::getPreset(PresetCallback callback)
	{
#ifdef RNBO_NOPRESETS
		RNBO_ASSERT(false); // not supported without std:lib
#else
		RNBO_ASSERT(callback);
		_parameterInterface->scheduleEvent(PresetEvent(RNBOTimeNow, PresetEvent::Get, nullptr, callback));
#endif
	}

	ConstPresetPtr CoreObject::getPresetSync()
	{
#ifdef RNBO_NOPRESETS
		RNBO_ASSERT(false); // not supported without std:lib
#else
		return _engine->getPresetSync();
#endif
	}


	void CoreObject::initializeEngine(EventHandler* handler)
	{
#ifdef RNBO_SIMPLEENGINE
		_engine = UniquePtr<EngineCore>(new EngineCore());
		_parameterInterface = _engine->createParameterInterface(ParameterEventInterface::NotThreadSafe, handler);
#else
		_engine = UniquePtr<Engine>(new Engine());
		_parameterInterface = _engine->createParameterInterface(ParameterEventInterface::MultiProducer, handler);
#endif
	}

#ifdef USE_DYNAMIC_COMPILATION
	void CoreObject::initializeFileWatcher(const char* fullPathToCPPSource)
	{
		_fileWatcher = make_unique<FileChangeWatcher>(fullPathToCPPSource, [this](FileChangeWatcher*) {
			this->fileChanged();
		});
	}

	void CoreObject::fileChanged()
	{
		setPatcher();
	}
#endif

	bool CoreObject::setPatcher()
	{
		UniquePtr<PatcherInterface> patcher { nullptr };

#ifndef RNBO_NO_PATCHERFACTORY

#ifdef USE_DYNAMIC_COMPILATION
#ifdef USE_TEST_FILEWATCHER
		_fileWatcher.reset();
		DynamicPatcherFactory factoryObject;
		patcher = factoryObject.createInstance();
		initializeFileWatcher(factoryObject.getFullPathToCppSource());
#endif	// USE_TEST_FILEWATCHER

#else	// USE_DYNAMIC_COMPILATION
		PatcherFactoryFunctionPtr factory = GetPatcherFactoryFunction(Platform::get());
		patcher = std::move(factory ? factory() : nullptr);
#endif	// USE_DYNAMIC_COMPILATION

#endif	// RNBO_NO_PATCHERFACTORY

		return setPatcher(std::move(patcher));
	}

#ifndef RNBO_FEATURE_NO_SETPATCHER
	bool CoreObject::setPatcher(UniquePtr<PatcherInterface> patcher)
	{
		return _engine->setPatcher(std::move(patcher));
	}
#endif

	ExternalDataIndex CoreObject::getNumExternalDataRefs() const
	{
#ifdef RNBO_SIMPLEENGINE
		return 0;
#else
		return _engine->getNumExternalDataRefs();
#endif
	}

	ExternalDataId CoreObject::getExternalDataId(ExternalDataIndex index) const
	{
#ifdef RNBO_SIMPLEENGINE
		return 0;
#else
		return _engine->getExternalDataId(index);
#endif
	}

	const ExternalDataInfo CoreObject::getExternalDataInfo(ExternalDataIndex index) const {
#ifdef RNBO_SIMPLEENGINE
		return {
			DataType::Untyped,
			nullptr,
			nullptr
		};
#else
		return _engine->getExternalDataInfo(index);
#endif
	}

	void CoreObject::setExternalData(ExternalDataId memoryId, char *data, size_t sizeInBytes, DataType type, ReleaseCallback callback)
	{
#ifndef RNBO_SIMPLEENGINE
		_engine->setExternalData(memoryId, data, sizeInBytes, type, callback);
#endif
	}

	void CoreObject::setExternalDataHandler(ExternalDataHandler* handler)
	{
#ifndef RNBO_SIMPLEENGINE
		_engine->setExternalDataHandler(handler);
#endif
	}

	void CoreObject::releaseExternalData(ExternalDataId memoryId)
	{
#ifndef RNBO_SIMPLEENGINE
		_engine->releaseExternalData(memoryId);
#endif
	}

} // namespace RNBO
