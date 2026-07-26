#pragma once

#include <JuceHeader.h>
#include "PluginParameters.h"
#include "dsp/compressor/ProcessorCompressor.h"
#include "dsp/transientSplitter/ProcessorTransientSplitter.h"
#include "dsp/mixer/DryWetMixer.h"
#include "dsp/analyser/AudioVisualiser.h"
#include "dsp/analyser/LevelAnalyser.h"
#include "dsp/onnx/OnnxProcessor.h"
#include "dsp/onnx/OnnxModel.h"
#include "dsp/onnx/WarningWindow.h"

#ifndef SCYCLONE_INFERENCE_STUB
#include <anira/ContextConfig.h>
#endif
#include "dsp/gain/ProcessorGain.h"
#include "dsp/Filter/IIRCutoffFilter.h"
#include "dsp/grainDelay/GrainDelay.h"
#include "dsp/utils/utils.h"

#include "ResamplingProcessor.h"
//#include <audio_basics/buffers/juce_AudioProcessLoadMeasurer.h>


//==============================================================================
    class AudioPluginAudioProcessor  : public juce::AudioProcessor, private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::Value advancedParameterControlVisible;
    juce::Value network1Name;
    juce::Value network2Name;

    std::function<void(juce::String newName)>onNetwork1NameChange;
    std::function<void(juce::String newName)>onNetwork2NameChange;
public:
    juce::AudioVisualiserComponent& getAudioVisualiser1();
    juce::AudioVisualiserComponent& getAudioVisualiser2();
    float getCurrentLevel(int index);
    void setLevelType(LevelType newLevelType);

    std::function<void(int modelID, juce::String& modelName)> setExternalModelName;
    void setInitialMuteParameters();
    void initialiseRnbo();

    void loadExternalModel(juce::File path, int id) {
        if (id == 1) onnxProcessor1.loadExternalModel(path);
        if (id == 2) onnxProcessor2.loadExternalModel(path);
    }

    float getCpuLoad();
private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;

private:
    juce::AudioProcessorValueTreeState parameters;

    ProcessorGain inputGain;
    ProcessorGain outputGain;

    ProcessorTransientSplitter processorTransientSplitter1;
    ProcessorTransientSplitter processorTransientSplitter2;
    
    IIRCutoffFilter iirCutoffFilter1;
    IIRCutoffFilter iirCutoffFilter2;

    juce::AudioBuffer<float> fadeBuffer;
    juce::AudioBuffer<float> network1Buffer;
    juce::AudioBuffer<float> network2Buffer;
    juce::AudioBuffer<float> grain1DryBuffer;
    juce::AudioBuffer<float> grain2DryBuffer;
    juce::AudioBuffer<float> monoBuffer;


    DryWetMixer dryWetMixer;
    DryWetMixer fadeMixer;
    DryWetMixer compMixer;
    DryWetMixer grain1DryWetMixer;
    DryWetMixer grain2DryWetMixer;

    std::unique_ptr<ResamplingProcessor> upsamplerOne;
    std::unique_ptr<ResamplingProcessor> upsamplerTwo;
    std::unique_ptr<ResamplingProcessor> downsamplerOne;
    std::unique_ptr<ResamplingProcessor> downsamplerTwo;
    int prepareOnnx(const juce::dsp::ProcessSpec& inputSpec, juce::dsp::ProcessSpec& onnxSpec);
    int prepareUpsampler(const juce::dsp::ProcessSpec &inputSpec, const int targetSampleRate);
    void prepareDownsampler(const juce::dsp::ProcessSpec &inputSpec, const juce::dsp::ProcessSpec &onnxSpec);
    bool resample = false;

#ifndef SCYCLONE_INFERENCE_STUB
    // Must precede onnxProcessor1/2: they capture a reference to it, and members are
    // initialised in declaration order regardless of the constructor's init-list order.
    anira::ContextConfig aniraContextConfig;
#endif

    OnnxProcessor onnxProcessor1;
    OnnxProcessor onnxProcessor2;

    WarningWindow warningWindow;
    double lastHostSampleRate = 48000.0;
    void handleModelLoad(int modelID, bool initLoading, juce::String modelName);
    void refreshReportedLatency();


    ProcessorCompressor processorCompressor;
    
    AudioVisualiser audioVisualiser;
    LevelAnalyser levelAnalyser1;
    LevelAnalyser levelAnalyser2;
    ProcessorGain processorGain;

    GrainDelay grainDelay1;
    GrainDelay grainDelay2;

    float cpuLoad{};
    juce::AudioProcessLoadMeasurer measurer;

    //==============================================================================
    JUCE_HEAVYWEIGHT_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
