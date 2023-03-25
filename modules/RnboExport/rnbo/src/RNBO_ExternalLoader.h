#ifndef _RNBO_ExternalLoader_h_
#define _RNBO_ExternalLoader_h_

#include "RNBO_EventTarget.h"
#include "RNBO_ExternalBase.h"
#include "../externals/ext_rnbo.h"

#include <unordered_map>

namespace RNBO
{

	class ExternalModule;
	class ExternalLoader;

	using ExternalModulePtr = std::shared_ptr<ExternalModule>;
	static std::unordered_map<String, ExternalModulePtr, StringHasher> s_loadedExternals;

	/**
	 * @private
	 */
	class ExternalModule
	{
	public:
		ExternalModule(const char *dllName, const char *pathToDll);

		h_extHandle createExternal();
		bool isValid();

		f_ext_getInterfaceVersion		_ext_getInterfaceVersion;

		f_ext_createExternal			_ext_createExternal;
		f_ext_deleteExternal			_ext_deleteExternal;

		f_ext_initialize				_ext_initialize;

		f_ext_getNumMidiInputPorts		_ext_getNumMidiInputPorts;
		f_ext_getNumMidiOutputPorts		_ext_getNumMidiOutputPorts;

		f_ext_getNumParameters			_ext_getNumParameters;
		f_ext_getParameterName			_ext_getParameterName;
		f_ext_getParameterId			_ext_getParameterId;
		f_ext_getParameterInfo			_ext_getParameterInfo;

		f_ext_getParameterValue			_ext_getParameterValue;
		f_ext_setParameterValue			_ext_setParameterValue;

		f_ext_convertToNormalizedParameterValue 	_ext_convertToNormalizedParameterValue;
		f_ext_convertToNormalizedParameterValue		_ext_convertFromNormalizedParameterValue;
		f_ext_constrainParameterValue				_ext_constrainParameterValue;

		f_ext_prepareToProcess			_ext_prepareToProcess;
		f_ext_process					_ext_process;

		f_ext_getNumInputChannels		_ext_getNumInputChannels;
		f_ext_getNumOutputChannels		_ext_getNumOutputChannels;

		f_ext_processMidiEvent			_ext_processMidiEvent;
		f_ext_processClockEvent			_ext_processClockEvent;

	private:
		bool open();
		void *getExtFunction(const char *name);
		void getExtFunctions();

		String				_dllName;
		String				_pathToDll;

		f_getFunction		_extFunctionGetter = nullptr;
		bool				_isValid;
	};

	/**
	 * @private
	 */
	class ExternalLoader : public ExternalBase
	{
	public:
		ExternalLoader(const char *externalName, const char *dllName, const char *pathToDll);

		void initialize() override;

		Index getNumMidiInputPorts() const override;
		Index getNumMidiOutputPorts() const override;

		ParameterIndex getNumParameters() const override;
		ConstCharPointer getParameterName(ParameterIndex index) const override;
		ConstCharPointer getParameterId(ParameterIndex index) const override;
		void getParameterInfo(ParameterIndex index, ParameterInfo* info) const override;

		ParameterValue getParameterValue(ParameterIndex index) override;
		void setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time = RNBOTimeNow) override;

		ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const override;
		ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const override;
		ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const override;

		void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) override;
		void process(const SampleValue* const* audioInputs, Index numInputs, SampleValue* const* audioOutputs, Index numOutputs, Index sampleFrames) override;

		Index getNumInputChannels() const override;
		Index getNumOutputChannels() const override;

		void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) override;
		void processClockEvent(MillisecondTime time, ClockId index, bool hasValue, ParameterValue value) override;

		EventTarget *getEventTarget();

	private:
		ExternalModulePtr		_externalModule;
		h_extHandle				_extHandle;
	};

	extern "C" ExternalBase *ExternalLoaderFactory(const char *externalName, const char *dllName, const char *pathToDll);
	extern "C" ExternalBase *ExternalLoaderFactory(const char *externalName, const char *dllName, const char *pathToDll)
	{
		return new ExternalLoader(externalName, dllName, pathToDll);
	}

	// reinterpret_cast is potentially dangerous
	static DynamicSymbolRegistrar ExternalLoaderRegisteredSymbol("ExternalLoaderFactory" , reinterpret_cast<void*>(ExternalLoaderFactory));


} // RNBO


#endif // _RNBO_ExternalLoader_h_

