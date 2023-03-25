#include "RNBO.h"
#include "RNBO_ExternalLoader.h"

// taken from http://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor

#ifdef _WIN32

#define OS_WINDOWS

#ifdef _WIN64

#define OS_WINDOWS_64

#endif

#elif __APPLE__
#include "TargetConditionals.h"

#define OS_MAC

#if TARGET_IPHONE_SIMULATOR

#define OS_MAC_IOS

#elif TARGET_OS_IPHONE

#define OS_MAC_IOS

#elif TARGET_OS_MAC

#define OS_MAC_OSX

#else
#   error "Unknown Apple platform"
#endif
#elif __linux__

#define OS_LINUX

#elif __unix__ // all unices not caught above

#define OS_UNIX

#elif defined(_POSIX_VERSION)

#define OS_POSIX

#else
#   error "Unknown compiler"
#endif

#include <iostream>

#ifdef OS_MAC
#include <dlfcn.h>
#endif // OS_MAC

namespace RNBO {

	int getInterfaceVersion(int* major, int* minor);
	void scheduleClockEvent(h_hostHandle hostHandle, ClockId clockIndex, MillisecondTime delay);
	void scheduleClockEventWithValue(h_hostHandle hostHandle, ClockId clockIndex, MillisecondTime delay, ParameterValue value);
	void flushClockEvents(h_hostHandle hostHandle, ClockId clockIndex, bool execute);
	void flushClockEventsWithValue(h_hostHandle hostHandle, ClockId clockIndex, ParameterValue value, bool execute);
	void sendMidiEvent(h_hostHandle hostHandle, int port, int b1, int b2, int b3);
	void sendMidiEventList(h_hostHandle hostHandle, int port, const list& data);
	MillisecondTime getCurrentTime(h_hostHandle hostHandle);
	void sendOutlet(h_hostHandle hostHandle, void* sender, OutletIndex index, ParameterValue value, SampleOffset sampleOffset);
	void consolelog(LogLevel level, const char *message);
	void getPlatformInterface(void** platformInterface);
	void *hostFunctionGetter(const char *funcName);
	void dummyExtFunction(void);

	int getInterfaceVersion(int* major, int* minor)
	{
		*major = ext_rnbo_major;
		*minor = ext_rnbo_minor;
		return 0;
	}

	void scheduleClockEvent(h_hostHandle hostHandle, ClockId clockIndex, MillisecondTime delay)
	{
		ExternalLoader *externalLoader = static_cast<ExternalLoader*>(hostHandle);
		externalLoader->getEngine()->scheduleClockEvent(externalLoader->getEventTarget(), clockIndex, delay);
	}

	void scheduleClockEventWithValue(h_hostHandle hostHandle, ClockId clockIndex, MillisecondTime delay, ParameterValue value)
	{
		ExternalLoader *externalLoader = static_cast<ExternalLoader*>(hostHandle);
		externalLoader->getEngine()->scheduleClockEventWithValue(externalLoader->getEventTarget(), clockIndex, delay, value);
	}

	void flushClockEvents(h_hostHandle hostHandle, ClockId clockIndex, bool execute)
	{
		ExternalLoader *externalLoader = static_cast<ExternalLoader*>(hostHandle);
		externalLoader->getEngine()->flushClockEvents(externalLoader->getEventTarget(), clockIndex, execute);
	}

	void flushClockEventsWithValue(h_hostHandle hostHandle, ClockId clockIndex, ParameterValue value, bool execute)
	{
		ExternalLoader *externalLoader = static_cast<ExternalLoader*>(hostHandle);
		externalLoader->getEngine()->flushClockEventsWithValue(externalLoader->getEventTarget(), clockIndex, value, execute);
	}

	void sendMidiEvent(h_hostHandle hostHandle, int port, int b1, int b2, int b3)
	{
		ExternalLoader *externalLoader = static_cast<ExternalLoader*>(hostHandle);
		externalLoader->getEngine()->sendMidiEvent(port, b1, b2, b3);
	}

	void sendMidiEventList(h_hostHandle hostHandle, int port, const list& data)
	{
		ExternalLoader *externalLoader = static_cast<ExternalLoader*>(hostHandle);
		externalLoader->getEngine()->sendMidiEventList(port, data);
	}

	MillisecondTime getCurrentTime(h_hostHandle hostHandle)
	{
		ExternalLoader *externalLoader = static_cast<ExternalLoader*>(hostHandle);
		return externalLoader->getEngine()->getCurrentTime();
	}

	void sendOutlet(h_hostHandle hostHandle, void* sender, OutletIndex index, ParameterValue value, SampleOffset sampleOffset)
	{
		RNBO_UNUSED(sender);
		ExternalLoader *externalLoader = static_cast<ExternalLoader*>(hostHandle);
		externalLoader->getEngine()->sendOutlet(externalLoader, index, value, sampleOffset);
	}

	void consolelog(LogLevel level, const char *message)
	{
		console->log(level, message);
	}

	void getPlatformInterface(void** platformInterface)
	{
		*platformInterface = Platform::get();
	}

	void *hostFunctionGetter(const char *funcName)
	{
		void *rv = nullptr;

		if (strcmp(funcName, "getInterfaceVersion") == 0) {
			rv = reinterpret_cast<void*>(&getInterfaceVersion);
		} else if (strcmp(funcName, "scheduleClockEvent") == 0) {
			rv = reinterpret_cast<void*>(&scheduleClockEvent);
		} else if (strcmp(funcName, "scheduleClockEventWithValue") == 0) {
			rv = reinterpret_cast<void*>(&scheduleClockEventWithValue);
		} else if (strcmp(funcName, "flushClockEvents") == 0) {
			rv = reinterpret_cast<void*>(&flushClockEvents);
		} else if (strcmp(funcName, "flushClockEventsWithValue") == 0) {
			rv = reinterpret_cast<void*>(&flushClockEventsWithValue);
		} else if (strcmp(funcName, "sendMidiEvent") == 0) {
			rv = reinterpret_cast<void*>(&sendMidiEvent);
		} else if (strcmp(funcName, "sendMidiEventList") == 0) {
			rv = reinterpret_cast<void*>(&sendMidiEventList);
		} else if (strcmp(funcName, "getCurrentTime") == 0) {
			rv = reinterpret_cast<void*>(&getCurrentTime);
		} else if (strcmp(funcName, "sendOutlet") == 0) {
			rv = reinterpret_cast<void*>(&sendOutlet);
		} else if (strcmp(funcName, "log") == 0) {
			rv = reinterpret_cast<void*>(&consolelog);
		} else if (strcmp(funcName, "getPlatformInterface") == 0) {
			rv = reinterpret_cast<void*>(&getPlatformInterface);
		}

		return rv;
	}

	void dummyExtFunction(void)
	{
		// an empty function - the default for unknown functions
		// not sure if we want to provide this for everything, but i guess this is ok for the start
	}

	ExternalModule::ExternalModule(const char *dllName, const char *pathToDll)
	: _dllName(dllName)
	, _pathToDll(pathToDll)
	{
		_isValid = open();
		getExtFunctions();
	}

	bool ExternalModule::open()
	{
		bool ok = false;
		String dllFullPath;

		dllFullPath.append(_pathToDll);
		dllFullPath.append(const_cast<char*>("/"));
		dllFullPath.append(_dllName);

#ifdef OS_MAC

		// add .rxo to get the bundle name
		dllFullPath.append(const_cast<char*>(".rxo"));
		// we need to look inside the bundle
		dllFullPath.append(const_cast<char*>("/Contents/MacOS/"));
		// and get the external
		dllFullPath.append(_dllName);

		void *ext_handle = dlopen((const char *)dllFullPath.c_str(), RTLD_LAZY);
		if (ext_handle) {
			f_ext_main mainFunction = f_ext_main(dlsym(ext_handle, s_ext_mainFunctionName));
			if (mainFunction) {
				_extFunctionGetter = mainFunction(&hostFunctionGetter);
				if (_extFunctionGetter) {
					int minor = 0, major = 0;

					_ext_getInterfaceVersion = f_ext_getInterfaceVersion(getExtFunction("getInterfaceVersion"));
					_ext_getInterfaceVersion(&major, &minor);

					if (major != ext_rnbo_major) {
						_extFunctionGetter = nullptr;
					} else {
						ok = true;
					}
				}
			}
		}
#endif // OS_MAC

		return ok;
	}

	h_extHandle ExternalModule::createExternal()
	{
		h_extHandle handle;
		_ext_createExternal(&handle);
		return handle;
	}

	bool ExternalModule::isValid()
	{
		return _isValid;
	}

	void *ExternalModule::getExtFunction(const char *name)
	{
		void *func = nullptr;

		if (_extFunctionGetter) {
			func = _extFunctionGetter(name);
		}

		if (!func) {
			func = reinterpret_cast<void*>(&dummyExtFunction);
		}

		return func;
	}

	void ExternalModule::getExtFunctions()
	{
		_ext_createExternal = f_ext_createExternal(getExtFunction("createExternal"));
		_ext_deleteExternal = f_ext_deleteExternal(getExtFunction("deleteExternal"));

		_ext_initialize = f_ext_initialize(getExtFunction("initialize"));

		_ext_getNumMidiInputPorts = f_ext_getNumMidiInputPorts(getExtFunction("getNumMidiInputPorts"));
		_ext_getNumMidiOutputPorts = f_ext_getNumMidiOutputPorts(getExtFunction("getNumMidiOutputPorts"));
		_ext_getNumParameters = f_ext_getNumParameters(getExtFunction("getNumParameters"));
		_ext_getParameterName = f_ext_getParameterName(getExtFunction("getParameterName"));
		_ext_getParameterId = f_ext_getParameterName(getExtFunction("getParameterId"));
		_ext_getParameterInfo = f_ext_getParameterInfo(getExtFunction("getParameterInfo"));

		_ext_getParameterValue = f_ext_getParameterValue(getExtFunction("getParameterValue"));
		_ext_setParameterValue = f_ext_setParameterValue(getExtFunction("setParameterValue"));
		_ext_convertToNormalizedParameterValue = f_ext_convertToNormalizedParameterValue(getExtFunction("convertToNormalizedParameterValue"));
		_ext_convertFromNormalizedParameterValue = f_ext_convertFromNormalizedParameterValue(getExtFunction("convertFromNormalizedParameterValue"));
		_ext_constrainParameterValue = f_ext_constrainParameterValue(getExtFunction("constrainParameterValue"));

		_ext_prepareToProcess = f_ext_prepareToProcess(getExtFunction("prepareToProcess"));
		_ext_process = f_ext_process(getExtFunction("process"));

		_ext_getNumInputChannels = f_ext_getNumInputChannels(getExtFunction("getNumInputChannels"));
		_ext_getNumOutputChannels = f_ext_getNumOutputChannels(getExtFunction("getNumOutputChannels"));

		_ext_processMidiEvent = f_ext_processMidiEvent(getExtFunction("processMidiEvent"));
		_ext_processClockEvent = f_ext_processClockEvent(getExtFunction("processClockEvent"));
	}




	ExternalLoader::ExternalLoader(const char *externalName, const char *dllName, const char *pathToDll)
	{
		RNBO_UNUSED(externalName);
		String name(dllName);

		auto externalModuleIter = s_loadedExternals.find(name);
		if (externalModuleIter == s_loadedExternals.end()) {
			s_loadedExternals.insert(std::make_pair(name, std::make_shared<ExternalModule>(dllName, pathToDll)));
		}

		_externalModule = s_loadedExternals.find(name)->second;
		_extHandle = _externalModule->createExternal();
	}

	void ExternalLoader::initialize()
	{
		// initialize the C interface to be able to call back into this class
		_externalModule->_ext_initialize(_extHandle, this);
	}

	Index ExternalLoader::getNumMidiInputPorts() const
	{
		return _externalModule->_ext_getNumMidiInputPorts(_extHandle);
	}

	Index ExternalLoader::getNumMidiOutputPorts() const
	{
		return _externalModule->_ext_getNumMidiOutputPorts(_extHandle);
	}

	ParameterIndex ExternalLoader::getNumParameters() const
	{
		return _externalModule->_ext_getNumParameters(_extHandle);
	}

	ConstCharPointer ExternalLoader::getParameterName(ParameterIndex index) const
	{
		return _externalModule->_ext_getParameterName(_extHandle, index);
	}

	ConstCharPointer ExternalLoader::getParameterId(ParameterIndex index) const
	{
		return _externalModule->_ext_getParameterId(_extHandle, index);
	}

	void ExternalLoader::getParameterInfo(ParameterIndex index, ParameterInfo* info) const
	{
		_externalModule->_ext_getParameterInfo(_extHandle, index, info);
	}

	ParameterValue ExternalLoader::getParameterValue(ParameterIndex index)
	{
		return _externalModule->_ext_getParameterValue(_extHandle, index);
	}

	void ExternalLoader::setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time)
	{
		_externalModule->_ext_setParameterValue(_extHandle, index, value, time);
	}

	ParameterValue ExternalLoader::convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const
	{
		return _externalModule->_ext_convertToNormalizedParameterValue(_extHandle, index, value);
	}

	ParameterValue ExternalLoader::convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const
	{
		return _externalModule->_ext_convertFromNormalizedParameterValue(_extHandle, index, normalizedValue);
	}

	ParameterValue ExternalLoader::constrainParameterValue(ParameterIndex index, ParameterValue value) const
	{
		return _externalModule->_ext_constrainParameterValue(_extHandle, index, value);
	}

	void ExternalLoader::prepareToProcess(number sampleRate, Index maxBlockSize, bool force)
	{
		_externalModule->_ext_prepareToProcess(_extHandle, sampleRate, maxBlockSize, force);
	}

	void ExternalLoader::process(const SampleValue* const* audioInputs, Index numInputs, SampleValue* const* audioOutputs, Index numOutputs, Index sampleFrames)
	{
		_externalModule->_ext_process(_extHandle, audioInputs, numInputs, audioOutputs, numOutputs, sampleFrames);
	}

	Index ExternalLoader::getNumInputChannels() const
	{
		return _externalModule->_ext_getNumInputChannels(_extHandle);
	}

	Index ExternalLoader::getNumOutputChannels() const
	{
		return _externalModule->_ext_getNumOutputChannels(_extHandle);
	}

	void ExternalLoader::processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length)
	{
		_externalModule->_ext_processMidiEvent(_extHandle, time, port, data, length);
	}

	void ExternalLoader::processClockEvent(MillisecondTime time, ClockId index, bool hasValue, ParameterValue value)
	{
		_externalModule->_ext_processClockEvent(_extHandle, time, index, hasValue, value);
	}

	EventTarget *ExternalLoader::getEventTarget()
	{
		return this;
	}

}
