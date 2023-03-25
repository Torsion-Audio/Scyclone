//
//  ext_rnbo.h
//  C interface for RNBO externals
//
//  Created by Stefan Brunner on 13.11.15.
//

#ifndef _EXT_RNBO_H_
#define _EXT_RNBO_H_

#define ext_rnbo_major 1
#define ext_rnbo_minor 0

namespace RNBO {

	typedef void*		h_hostHandle;
	typedef void*		h_extHandle;

	constexpr const char *s_ext_mainFunctionName = "ext_initialize";

	typedef int (*f_ext_getInterfaceVersion)(int* major, int* minor);
	typedef int (*f_host_getInterfaceVersion)(int* major, int* minor);

	typedef void* (*f_getFunction)(const char* functionName);

	typedef f_getFunction (*f_ext_main)(f_getFunction hostFunctionGetter);

	typedef int (*f_ext_createExternal)(h_extHandle *extHandle);
	typedef int (*f_ext_deleteExternal)(h_extHandle handle);

	typedef void (*f_ext_initialize)(h_extHandle handle, h_hostHandle hostHandle);

	// as we see here, in a pure C interface we have no access to the RNBO_Types, which live in a C++ world
	// maybe something to think about
	typedef Index (*f_ext_getNumMidiInputPorts)(h_extHandle handle);
	typedef Index (*f_ext_getNumMidiOutputPorts)(h_extHandle handle);
	typedef ParameterIndex (*f_ext_getNumParameters)(h_extHandle handle);
	typedef ConstCharPointer (*f_ext_getParameterName)(h_extHandle handle, ParameterIndex index);
	typedef ConstCharPointer (*f_ext_getParameterId)(h_extHandle handle, ParameterIndex index);
	typedef void (*f_ext_getParameterInfo)(h_extHandle handle, ParameterIndex index, ParameterInfo *info);

	typedef ParameterValue (*f_ext_getParameterValue)(h_extHandle handle, ParameterIndex index);
	typedef void (*f_ext_setParameterValue)(h_extHandle handle, ParameterIndex index, ParameterValue value, MillisecondTime time);
	typedef ParameterValue (*f_ext_convertToNormalizedParameterValue)(h_extHandle handle, ParameterIndex index, ParameterValue value);
	typedef ParameterValue (*f_ext_convertFromNormalizedParameterValue)(h_extHandle handle, ParameterIndex index, ParameterValue normalizedValue);
	typedef ParameterValue (*f_ext_constrainParameterValue)(h_extHandle handle, ParameterIndex index, ParameterValue value);

	typedef void (*f_ext_prepareToProcess)(h_extHandle handle, double sampleRate, Index blockSize, bool force);
	typedef void (*f_ext_process)(h_extHandle handle, const SampleValue* const* audioInputs, Index numInputs, SampleValue* const* audioOutputs, Index numOutputs, Index sampleFrames);

	typedef Index (*f_ext_getNumInputChannels)(h_extHandle handle);
	typedef Index (*f_ext_getNumOutputChannels)(h_extHandle handle);

	typedef void (*f_ext_processMidiEvent)(h_extHandle handle, MillisecondTime time, int port, ConstByteArray data, Index length);
	typedef void (*f_ext_processClockEvent)(h_extHandle handle, MillisecondTime time, ClockId index, bool hasValue, ParameterValue value);

	typedef void (*f_host_scheduleClockEvent)(h_hostHandle hostHandle, ClockId clockIndex, MillisecondTime delay);
	typedef void (*f_host_scheduleClockEventWithValue)(h_hostHandle hostHandle, ClockId clockIndex, MillisecondTime delay, ParameterValue value);
	typedef void (*f_host_flushClockEvents)(h_hostHandle hostHandle, ClockId clockIndex, bool execute);
	typedef void (*f_host_flushClockEventsWithValue)(h_hostHandle hostHandle, ClockId clockIndex, ParameterValue value, bool execute);
	typedef void (*f_host_sendMidiEvent)(h_hostHandle hostHandle, int port, int b1, int b2, int b3);
	typedef void (*f_host_sendMidiEventList)(h_hostHandle hostHandle, int port, const list& data);

	typedef MillisecondTime (*f_host_getCurrentTime)(h_hostHandle hostHandle);

	typedef void (*f_host_sendOutlet)(h_hostHandle hostHandle, void* sender, OutletIndex index, ParameterValue value, SampleOffset sampleOffset);
	typedef void (*f_host_deleteClockEvents)(h_hostHandle hostHandle, void* eventTarget);
	typedef void (*f_host_updateEventTarget)(h_hostHandle hostHandle, void *oldTarget, void *newTarget);

	typedef void (*f_host_log)(LogLevel level, const char *message);
	typedef void (*f_host_getPlatformInterface)(void** platformInterface);

}

#endif // _EXT_RNBO_H_
