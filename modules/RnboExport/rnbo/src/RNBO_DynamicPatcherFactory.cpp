//
//  RNBO_DynamicPatcherFactory.cpp
//
//  Created by Rob Sussman on 12/4/15.
//
//

#include "RNBO_DynamicPatcherFactory.h"

#ifdef USE_DYNAMIC_COMPILATION

#include <fstream>
#include <streambuf>
#include <sstream>

#ifndef RNBO_NO_JUCE
#include "JuceHeader.h"
#endif

//#define RNBO_MEASURECOMPILETIME
#ifdef RNBO_MEASURECOMPILETIME
#include <chrono>
#endif

namespace RNBO {

	// It is important for the RNBOClang instance to live longer than the
	// PatcherInterface returned from it via clang/llvm.
	// To insure this we keep the RNBOClang in a shared_ptr<> so that
	// the DynamicPatcherFactory can be freed before the patcher it returns is freed.

	class DynamicObjectWrapper : public PatcherInterface
	{
	public:

		// it would be nice if we could avoid having to wrap the object
		// as is we have to maintain this to match up with the PatcherInterface

		DynamicObjectWrapper(std::shared_ptr<ClangInterface> clanger,
							 std::unique_ptr<PatcherInterface> objectToWrap)
		: _clanger(clanger)
		, _wrappedObject(std::move(objectToWrap))
		{

		}

		void destroy() override
		{
			_wrappedObject->destroy();
		}

		void initialize(PatcherStateInterface& state) override
		{
			_wrappedObject->setEngineAndPatcher(getEngine(), getPatcherEventTarget());
			_wrappedObject->initialize(state);
		}

		Index getNumMidiInputPorts() const override
		{
			return _wrappedObject->getNumMidiInputPorts();
		}

		Index getNumMidiOutputPorts() const override
		{
			return _wrappedObject->getNumMidiOutputPorts();
		}

		ParameterIndex getNumParameters() const override
		{
			return _wrappedObject->getNumParameters();
		}

		ConstCharPointer getParameterName(ParameterIndex index) const override
		{
			return _wrappedObject->getParameterName(index);
		}

		ConstCharPointer getParameterId(ParameterIndex index) const override
		{
			return _wrappedObject->getParameterId(index);
		}

		void getParameterInfo(ParameterIndex index, ParameterInfo* info) const override
		{
			return _wrappedObject->getParameterInfo(index, info);
		}

		ParameterValue getParameterValue(ParameterIndex index) override
		{
			return _wrappedObject->getParameterValue(index);
		}

		void setParameterValue(ParameterIndex index, ParameterValue value, MillisecondTime time) override
		{
			_wrappedObject->setParameterValue(index, value, time);
		}

		Index getMaxBlockSize() const override
		{
			return _wrappedObject->getMaxBlockSize();
		}

		number getSampleRate() const override
		{
			return _wrappedObject->getSampleRate();
		}

		bool hasFixedVectorSize() const override
		{
			return _wrappedObject->hasFixedVectorSize();
		}

		void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) override
		{
			_wrappedObject->prepareToProcess(sampleRate, maxBlockSize, force);
		}

		void process(const SampleValue* const* audioInputs, Index numInputs,
					 SampleValue* const* audioOutputs, Index numOutputs,
					 Index sampleFrames) override
		{
			_wrappedObject->process(audioInputs, numInputs, audioOutputs, numOutputs, sampleFrames);
		}

		Index getNumInputChannels() const override
		{
			return _wrappedObject->getNumInputChannels();
		}

		Index getNumOutputChannels() const override
		{
			return _wrappedObject->getNumOutputChannels();
		}

		void dump() override
		{
			_wrappedObject->dump();
		}

		Index getProbingChannels(MessageTag outletId) const override {
			return _wrappedObject->getProbingChannels(outletId);
		}

		void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) override
		{
			_wrappedObject->processParameterEvent(index, value, time);
		}

		void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) override
		{
			_wrappedObject->processNormalizedParameterEvent(index, value, time);
		}

		void processOutletEvent(EngineLink* sender, OutletIndex index, ParameterValue value,  MillisecondTime time) override
		{
			_wrappedObject->processOutletEvent(sender, index, value, time);
		}

		void processOutletAtCurrentTime(EngineLink* sender, OutletIndex index, ParameterValue value) override
		{
			_wrappedObject->processOutletAtCurrentTime(sender, index, value);
		}

		void processNumMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, number payload) override
		{
			_wrappedObject->processNumMessage(tag, objectId, time, payload);
		}

		void processListMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, const list& payload) override
		{
			_wrappedObject->processListMessage(tag, objectId, time, payload);
		}

		void processBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) override
		{
			_wrappedObject->processBangMessage(tag, objectId, time);
		}

		void processTempoEvent(MillisecondTime time, Tempo tempo) override
		{
			_wrappedObject->processTempoEvent(time, tempo);
		}

		void processTransportEvent(MillisecondTime time, TransportState state) override
		{
			_wrappedObject->processTransportEvent(time, state);
		}

		void processBeatTimeEvent(MillisecondTime time, BeatTime beatTime) override
		{
			_wrappedObject->processBeatTimeEvent(time, beatTime);
		}

		void processTimeSignatureEvent(MillisecondTime time, int numerator, int denominator) override
		{
			_wrappedObject->processTimeSignatureEvent(time, numerator, denominator);
		}

		void getState(PatcherStateInterface& state) override
		{
			_wrappedObject->getState(state);
		}

		DataRefIndex getNumDataRefs() const override
		{
			return _wrappedObject->getNumDataRefs();
		}

		ParameterIndex getNumSignalInParameters() const override
		{
			return _wrappedObject->getNumSignalInParameters();
		}

		ParameterIndex getNumSignalOutParameters() const override
		{
			return _wrappedObject->getNumSignalOutParameters();
		}

		MessageTagInfo resolveTag(MessageTag tag) const override
		{
			return _wrappedObject->resolveTag(tag);
		}

		MessageIndex getNumMessages() const override
		{
			return _wrappedObject->getNumMessages();
		}

		const MessageInfo& getMessageInfo(MessageIndex index) const override {
			return _wrappedObject->getMessageInfo(index);
		}

		DataRef* getDataRef(DataRefIndex index) override
		{
			return _wrappedObject->getDataRef(index);
		}

		void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) override
		{
			_wrappedObject->processMidiEvent(time, port, data, length);
		}

		void processClockEvent(MillisecondTime time, ClockId index, bool hasValue, ParameterValue value) override
		{
			_wrappedObject->processClockEvent(time, index, hasValue, value);
		}

		void processDataViewUpdate(DataRefIndex index, MillisecondTime time) override
		{
			_wrappedObject->processDataViewUpdate(index, time);
		}

		ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const override
		{
			return _wrappedObject->convertToNormalizedParameterValue(index, value);
		}

		ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue normalizedValue) const override
		{
			return _wrappedObject->convertFromNormalizedParameterValue(index, normalizedValue);
		}

		ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const override
		{
			return _wrappedObject->constrainParameterValue(index, value);
		}

		void getPreset(PatcherStateInterface& preset) override
		{
			_wrappedObject->getPreset(preset);
		}

		void setPreset(MillisecondTime time, PatcherStateInterface& preset) override
		{
			_wrappedObject->setPreset(time, preset);
		}

	private:
		// c++ creates member objects in the order they are defined
		// and deletes member objects in the reverse order
		// so the _wrappedObject will be deleted before the _clanger.
		std::shared_ptr<ClangInterface> _clanger;
		std::unique_ptr<PatcherInterface> _wrappedObject;
	};

	String DynamicPatcherFactory::defaultFullPathToCppSource()
	{
#ifdef RNBO_NO_JUCE
		return RNBO_TEST_FILE;
#else
		juce::File code(juce::String(RNBO_TEST_FILE));
		return String(code.getFullPathName().toRawUTF8());
#endif
	}

	DynamicPatcherFactory::DynamicPatcherFactory(const char* fullPathToCppSource,
                                                 const char* fullPathToRNBOHeaders)
	{
		_fullPathToCppSource = fullPathToCppSource ? fullPathToCppSource : defaultFullPathToCppSource();
		std::ifstream t(_fullPathToCppSource.c_str());
		std::stringstream buffer;
		buffer << t.rdbuf();
		String src = buffer.str().c_str();
		initWithNameAndSource(
			_fullPathToCppSource.c_str(),
			src.c_str(),
			fullPathToRNBOHeaders,
#ifdef _DEBUG
			ClangInterface::O0
#else
			ClangInterface::O3
#endif
		);
	}

	DynamicPatcherFactory::DynamicPatcherFactory(const char* name,
                                                 const char* source,
                                                 const char* fullPathToRNBOHeaders,
												 ClangInterface::OLevel oLevel)
	{
		initWithNameAndSource(name, source, fullPathToRNBOHeaders, oLevel);
	}

	void DynamicPatcherFactory::initWithNameAndSource(const char* name,
                                                      const char* source,
                                                      const char* fullPathToRNBOHeaders,
													  ClangInterface::OLevel oLevel)
	{
        _clanger = ClangInterface::create();
		_didCompileSucceed = false;
		if (_clanger) {
#ifdef RNBO_USE_FLOAT32
            _clanger->addPreprocessorDefinition("RNBO_USE_FLOAT32"); // use float* buffers if the client is using them
#endif
            _clanger->addPreprocessorDefinition("RNBO_NOSTDLIB"); // enable stdlib overrides in RNBO code
            if (fullPathToRNBOHeaders) {
                _clanger->addIncludePath(fullPathToRNBOHeaders);
            }

            // add any registered symbols
            // upon creating the execution engine we add any symbols
            // registered with DynamicSymbolRegistry
            RNBO::DynamicSymbolRegistry::DynamicSymbolList& symbols = RNBO::DynamicSymbolRegistry::getRegisteredSymbols();
            for (auto& symbol : symbols) {
                _clanger->addSymbol(symbol._name, symbol._location);
            }

			_clanger->setOptimizationLevel(oLevel);

#ifdef RNBO_MEASURECOMPILETIME
			std::chrono::steady_clock::time_point beginCompile = std::chrono::steady_clock::now();
#endif
			bool success = _clanger->compile(name, source);
#ifdef RNBO_MEASURECOMPILETIME
			std::chrono::steady_clock::time_point endCompile = std::chrono::steady_clock::now();
#endif
			if (success) {
#ifdef _DEBUG
                RNBO::console->log("compilation successful.");
#endif
				// let's only call it successful if we can in fact get the patcher factory function
				// this also force it to do the JIT which we want to happen when we make the factory (on secondary thread)
				// rather than on main thread
#ifdef RNBO_MEASURECOMPILETIME
				std::chrono::steady_clock::time_point beginGetFactoryFunction = std::chrono::steady_clock::now();
#endif
				auto factory = getPatcherFactoryFunction();
#ifdef RNBO_MEASURECOMPILETIME
				std::chrono::steady_clock::time_point endGetFactoryFunction = std::chrono::steady_clock::now();

				std::cout << "Compile time: " << std::chrono::duration_cast<std::chrono::milliseconds>(endCompile - beginCompile).count() << "[ms]" << std::endl;
				std::cout << "getFactoryFunction time: " << std::chrono::duration_cast<std::chrono::milliseconds>(endGetFactoryFunction - beginGetFactoryFunction).count() << "[ms]" << std::endl;
#endif
				if (factory) {
					_didCompileSucceed = true;
				}
			}
		}
	}

	void DynamicPatcherFactory::getLastErrors(_dictionary **lastErrors)
	{
		if (_clanger) {
			_clanger->getLastErrors(lastErrors);
		}
	}

	const char* DynamicPatcherFactory::getFullPathToCppSource()
	{
		return _fullPathToCppSource.c_str();
	}

    PatcherFactoryFunctionPtr DynamicPatcherFactory::getPatcherFactoryFunction()
	{
		// now we  are ready to get the object factory
		PatcherFactoryFunctionPtr factoryFunction = nullptr;
		if (_clanger) {
			GetPatcherFactoryFunctionPtr getPatcherFactoryFunction =
				(GetPatcherFactoryFunctionPtr) _clanger->getFunctionAddress("GetPatcherFactoryFunction");
			if (!getPatcherFactoryFunction) {
				console->log("error: GetPatcherFactoryFunction function not found!");
			}
			else {
				factoryFunction = getPatcherFactoryFunction(Platform::get());
				if (!factoryFunction) {
					console->log("error: GetPatcherFactoryFunction returned nullptr!");
				}
			}
		}
		return factoryFunction;
	}

	UniquePtr<PatcherInterface> DynamicPatcherFactory::createInstance()
	{
		PatcherFactoryFunctionPtr factory = getPatcherFactoryFunction();
		UniquePtr<PatcherInterface> instance;
		if (factory) {
			std::unique_ptr<PatcherInterface> rnboObject(factory());

			// the DynamicObjectWrapper now owns the rnboObject and rnboObject should be nullptr (as it was moved into DynamicObjectObjectWrapper)
			instance = UniquePtr<PatcherInterface>(new DynamicObjectWrapper(_clanger, std::move(rnboObject)));
		}

		return instance;
	}

#if 0 // TODO: fix debugging
	void DynamicPatcherFactory::SetNotifyDebuggerCallback(RNBOClang::NotifyDebuggerCallback callback)
	{
		_clanger->SetNotifyDebuggerCallback(callback);
	}
#endif

} // namespace RNBO

#endif // #ifdef USE_DYNAMIC_COMPILATION
