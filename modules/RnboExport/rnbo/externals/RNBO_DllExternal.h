//
//  ext_rnboclient.h
//
//  Created by Stefan Brunner on 13.11.15.
//
//

#ifndef _EXT_RNBOCLIENT_H_
#define _EXT_RNBOCLIENT_H_

#include "RNBO_External.h"
#include "ext_rnbo.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unordered_map>
#include <sstream>

// the current design only allows a single DLL external in one project
// to change that we would need to somehow encapsulate different externals
// into different namespaces or similar
#define REGISTER_RNBO_DLLEXTERNAL(externalName) 	RNBO::ExternalBase *RNBO::externalFactory() \
{ \
return new externalName (); \
}

namespace RNBO {

	// host functions
	static f_getFunction hostFunctionGetter = 0;
	static h_hostHandle hostHandle = 0;

	f_host_getInterfaceVersion			host_GetInterfaceVersion = 0;
	f_host_scheduleClockEvent			host_scheduleClockEvent = 0;
	f_host_scheduleClockEventWithValue	host_scheduleClockEventWithValue = 0;
	f_host_flushClockEvents				host_flushClockEvents = 0;
	f_host_flushClockEventsWithValue	host_flushClockEventsWithValue = 0;
	f_host_sendMidiEvent				host_sendMidiEvent = 0;
	f_host_sendMidiEventList			host_sendMidiEventList = 0;
	f_host_getCurrentTime				host_getCurrentTime = 0;
	f_host_sendOutlet					host_sendOutlet = 0;
	f_host_deleteClockEvents			host_deleteClockEvents = 0;
	f_host_updateEventTarget			host_updateEventTarget = 0;
	f_host_log							host_log = 0;
	f_host_getPlatformInterface			host_getPlatformInterface = 0;

	PlatformInterface* platform;

	/**
	 * @private
	 *
	 * Implement logging through the host
	 */
	class ExternalLogger : public LoggerInterface
	{
	public:
		void log(LogLevel level, const char* message)
		{
			host_log(level, message);
		}
	};

	ExternalLogger s_logger_instance;
	LoggerInterface* console = &s_logger_instance;

	/**
	 * @private
	 */
	class ExternalEngineInterface : public EngineInterface
	{
	public:
		ExternalEngineInterface(h_hostHandle hostHandle)
		: _hostHandle(hostHandle)
		{}

		virtual ~ExternalEngineInterface() {}

		void scheduleClockEvent(EventTarget* eventTarget, ClockId clockIndex, MillisecondTime delay) override
		{
			host_scheduleClockEvent(_hostHandle, clockIndex, delay);
		}

		void scheduleClockEventWithValue(EventTarget* eventTarget, ClockId clockIndex, MillisecondTime delay, ParameterValue value) override
		{
			host_scheduleClockEventWithValue(_hostHandle, clockIndex, delay, value);
		}

		void flushClockEvents(EventTarget* eventTarget, ClockId clockIndex, bool execute) override
		{
			host_flushClockEvents(_hostHandle, clockIndex, execute);
		}

		void flushClockEventsWithValue(EventTarget* eventTarget, ClockId clockIndex, ParameterValue value, bool execute) override
		{
			host_flushClockEventsWithValue(_hostHandle, clockIndex, value, execute);
		}

		void sendMidiEvent(int port, int b1, int b2, int b3) override
		{
			host_sendMidiEvent(_hostHandle, port, b1, b2, b3);
		}

		void sendMidiEventList(int port, const list& data) override
		{
			host_sendMidiEventList(_hostHandle, port, data);
		}

		void notifyParameterValueChanged(Index index, ParameterValue value, bool ignoreSource) override
		{
			// not yet sure how this would work
		}

		void scheduleParameterChange(Index index, ParameterValue value, MillisecondTime offset) override
		{
			// not yet sure how this would work
		}

		MillisecondTime getCurrentTime() override
		{
			return host_getCurrentTime(_hostHandle);
		}

		void sendOutlet(EngineLink* sender, Index index, ParameterValue value, SampleOffset sampleOffset) override
		{
			host_sendOutlet(_hostHandle, sender, index, value, sampleOffset);
		}

	private:

		// holding DataRefs is not implementd for externals
		void sendDataRefUpdated(DataRefIndex index) override
		{
			assert(false);
		}

		void sendNumMessage(MessageTag tag, MessageTag objectId, number payload) override
		{
			// not implemented, yet
			assert(false);
		}

		void sendListMessage(MessageTag tag, MessageTag objectId, const list& payload) override
		{
			// not implemented, yet
			assert(false);
		}

		void updatePatcherEventTarget(const EventTarget *oldTarget, PatcherEventTarget *newTarget) override
		{
			// makes no sense for externals
			assert(false);
		}

		void rescheduleEventTarget(const EventTarget *target) override
		{
			// makes no sense for externals
			assert(false);
		}

	private:
		void	*_hostHandle;
	};


	std::unordered_map<h_extHandle, ExternalEngineInterface*>	engineInterfacesList;

	// the external object factory, needs to be defined by the external
	ExternalBase *externalFactory();

	int getInterfaceVersion(int* major, int* minor)
	{
		*major = ext_rnbo_major;
		*minor = ext_rnbo_minor;
		return 0;
	}

	int createExternal(h_extHandle *extHandle)
	{
		*extHandle = externalFactory();
		return 0;
	}

	int deleteExternal(h_extHandle extHandle)
	{
		delete (ExternalBase *)extHandle;
		engineInterfacesList.erase(extHandle);
		return 0;
	}

	void initialize(h_extHandle handle, h_hostHandle hostHandle)
	{
		ExternalEngineInterface *engineInterface = new ExternalEngineInterface(hostHandle);
		engineInterfacesList.insert(std::pair<h_extHandle, ExternalEngineInterface*>(handle, engineInterface));
		((ExternalBase *)handle)->setEngineAndPatcher(engineInterface, nullptr);
		((ExternalBase *)handle)->initialize();
	}

	Index getNumParameters(h_extHandle handle)
	{
		return ((ExternalBase *)handle)->getNumParameters();
	}

	ConstCharPointer getParameterName(h_extHandle handle, Index index)
	{
		return ((ExternalBase *)handle)->getParameterName(index);
	}

	ConstCharPointer getParameterId(h_extHandle handle, Index index)
	{
		return ((ExternalBase *)handle)->getParameterId(index);
	}

	void getParameterInfo(h_extHandle handle, Index index, ParameterInfo *info)
	{
		return ((ExternalBase *)handle)->getParameterInfo(index, info);
	}

	ParameterValue getParameterValue(h_extHandle handle, Index index)
	{
		return ((ExternalBase *)handle)->getParameterValue(index);
	}

	void setParameterValue(h_extHandle handle, Index index, ParameterValue value, MillisecondTime time)
	{
		return ((ExternalBase *)handle)->setParameterValue(index, value, time);
	}

ParameterValue convertToNormalizedParameterValue(h_extHandle handle, Index index, ParameterValue value)
	{
		return ((ExternalBase *)handle)->convertToNormalizedParameterValue(index, value);
	}

ParameterValue convertFromNormalizedParameterValue(h_extHandle handle, Index index, ParameterValue normalizedValue)
	{
		return ((ExternalBase *)handle)->convertFromNormalizedParameterValue(index, normalizedValue);
	}

	ParameterValue constrainParameterValue(h_extHandle handle, Index index, ParameterValue value)
	{
		return ((ExternalBase *)handle)->constrainParameterValue(index, value);
	}


	void prepareToProcess(h_extHandle handle, double sampleRate, size_t blockSize)
	{
		return ((ExternalBase *)handle)->prepareToProcess(sampleRate, blockSize);
	}

	void process(h_extHandle handle, const SampleValue* const* audioInputs, size_t numInputs, SampleValue* const* audioOutputs, size_t numOutputs, size_t sampleFrames)
	{
		return ((ExternalBase *)handle)->process(audioInputs, numInputs, audioOutputs, numOutputs, sampleFrames);
	}

	Index getNumInputChannels(h_extHandle handle)
	{
		return ((ExternalBase *)handle)->getNumInputChannels();
	}

	Index getNumOutputChannels(h_extHandle handle)
	{
		return ((ExternalBase *)handle)->getNumOutputChannels();
	}

	void processMidiEvent(h_extHandle handle, MillisecondTime time, int port, ConstByteArray data, Index length)
	{
		// not sure if i like this cast construction, maybe we find a better way to deal with handles for externals
		EventTarget *target = dynamic_cast<EventTarget *>((RNBO::ExternalBase *)handle);
		target->processMidiEvent(time, port, data, length);
	}

	void processClockEvent(h_extHandle handle, MillisecondTime time, ClockId index, bool hasValue, ParameterValue value)
	{
		// not sure if i like this cast construction, maybe we find a better way to deal with handles for externals
		EventTarget *target = dynamic_cast<EventTarget *>((RNBO::ExternalBase *)handle);
		target->processClockEvent(time, index, hasValue, value);
	}

	// the C style interface

	static void *extFunctionGetter(const char *funcName)
	{
		void *rv = NULL;
		if (strcmp(funcName, "getInterfaceVersion") == 0) {
			rv = (void *)&getInterfaceVersion;
		} else if (strcmp(funcName, "createExternal") == 0) {
			rv = (void *)&createExternal;
		} else if (strcmp(funcName, "deleteExternal") == 0) {
			rv = (void *)&deleteExternal;
		} else if (strcmp(funcName, "initialize") == 0) {
			rv = (void *)&initialize;
		} else if (strcmp(funcName, "getNumParameters") == 0) {
			rv = (void *)&getNumParameters;
		} else if (strcmp(funcName, "getParameterName") == 0) {
			rv = (void *)&getParameterName;
		} else if (strcmp(funcName, "getParameterId") == 0) {
			rv = (void *)&getParameterId;
		} else if (strcmp(funcName, "getParameterInfo") == 0) {
			rv = (void *)&getParameterInfo;
		} else if (strcmp(funcName, "getParameterValue") == 0) {
			rv = (void *)&getParameterValue;
		} else if (strcmp(funcName, "setParameterValue") == 0) {
			rv = (void *)&setParameterValue;
		} else if (strcmp(funcName, "convertToNormalizedParameterValue") == 0) {
			rv = (void *)&convertToNormalizedParameterValue;
		} else if (strcmp(funcName, "convertFromNormalizedParameterValue") == 0) {
			rv = (void *)&convertFromNormalizedParameterValue;
		} else if (strcmp(funcName, "constrainParameterValue") == 0) {
			rv = (void *)&constrainParameterValue;
		} else if (strcmp(funcName, "prepareToProcess") == 0) {
			rv = (void *)&prepareToProcess;
		} else if (strcmp(funcName, "process") == 0) {
			rv = (void *)&process;
		} else if (strcmp(funcName, "getNumInputChannels") == 0) {
			rv = (void *)&getNumInputChannels;
		} else if (strcmp(funcName, "getNumOutputChannels") == 0) {
			rv = (void *)&getNumOutputChannels;
		} else if (strcmp(funcName, "processMidiEvent") == 0) {
			rv = (void *)&processMidiEvent;
		} else if (strcmp(funcName, "processClockEvent") == 0) {
			rv = (void *)&processClockEvent;
		}

		return rv;
	}

	static void getHostFunctions(void)
	{
		host_GetInterfaceVersion = (f_host_getInterfaceVersion)(*hostFunctionGetter)((char *) "getInterfaceVersion");
		host_scheduleClockEvent = (f_host_scheduleClockEvent)(*hostFunctionGetter)((char *) "scheduleClockEvent");
		host_scheduleClockEventWithValue = (f_host_scheduleClockEventWithValue)(*hostFunctionGetter)((char *) "scheduleClockEventWithValue");
		host_flushClockEvents = (f_host_flushClockEvents)(*hostFunctionGetter)((char *) "flushClockEvents");
		host_flushClockEventsWithValue = (f_host_flushClockEventsWithValue)(*hostFunctionGetter)((char *) "flushClockEventsWithValue");
		host_sendMidiEvent = (f_host_sendMidiEvent)(*hostFunctionGetter)((char *) "sendMidiEvent");
		host_sendMidiEventList = (f_host_sendMidiEventList)(*hostFunctionGetter)((char *) "sendMidiEventList");
		host_getCurrentTime = (f_host_getCurrentTime)(*hostFunctionGetter)((char *) "getCurrentTime");
		host_sendOutlet = (f_host_sendOutlet)(*hostFunctionGetter)((char *) "sendOutlet");
		host_deleteClockEvents = (f_host_deleteClockEvents)(*hostFunctionGetter)((char *) "deleteClockEvents");
		host_updateEventTarget = (f_host_updateEventTarget)(*hostFunctionGetter)((char *) "updateEventTarget");

		host_log = (f_host_log)(*hostFunctionGetter)((char *) "log");
		host_getPlatformInterface = (f_host_getPlatformInterface)(*hostFunctionGetter)((char *) "getPlatformInterface");
	}

	extern "C" f_getFunction ext_initialize(f_getFunction func)
	{
		int minor = 0, major = 0;

		if (!func) return nullptr;

		hostFunctionGetter = func;
		getHostFunctions();

		host_GetInterfaceVersion(&major, &minor);
		host_getPlatformInterface((void**)&platform);

		if (major != ext_rnbo_major) return nullptr;

		return (f_getFunction)extFunctionGetter;
	}
}

#endif
