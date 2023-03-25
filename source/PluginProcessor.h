#pragma once

#include <JuceHeader.h>
#include "PluginParameters.h"
#include "dsp/compressor/ProcessorCompressor.h"
#include "dsp/transientSplitter/ProcessorTransientSplitter.h"
#include "dsp/mixer/DryWetMixer.h"
#include "dsp/analyser/AudioVisualiser.h"
#include "dsp/analyser/LevelAnalyser.h"
#include "dsp/onnx/OnnxProcessor.h"
#include "dsp/gain/ProcessorGain.h"
#include "dsp/Filter/IIRCutoffFilter.h"
#include "dsp/grainDelay/GrainDelay.h"


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

    void stereoToMono(juce::AudioBuffer<float>& targetMonoBlock, juce::AudioBuffer<float>& sourceBlock);

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
    float getCurrentLevel();
    void setLevelType(LevelType newLevelType);

    std::function<void(int modelID, juce::String& modelName)> setExternalModelName;
    void setInitialMuteParameters();
    void initialiseRnbo();
    void loadExternalModel(juce::File path, int id) {
        if (id == 1) onnxProcessor1->loadExternalModel(path);
        if (id == 2) onnxProcessor2->loadExternalModel(path);
    }

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    void copyBuffer(juce::AudioBuffer<float>& target, juce::AudioBuffer<float>& source);
private:
    juce::AudioProcessorValueTreeState parameters;

    ProcessorGain inputGain;
    ProcessorGain outputGain;

    std::unique_ptr<ProcessorTransientSplitter> processorTransientSplitter1;
    std::unique_ptr<ProcessorTransientSplitter> processorTransientSplitter2;
    
    std::unique_ptr<IIRCutoffFilter> iirCutoffFilter1;
    std::unique_ptr<IIRCutoffFilter> iirCutoffFilter2;

    juce::AudioBuffer<float> dryBuffer;
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

    std::unique_ptr<OnnxProcessor> onnxProcessor1;
    std::unique_ptr<OnnxProcessor> onnxProcessor2;

    std::unique_ptr<ProcessorCompressor> processorCompressor;
    
    AudioVisualiser audioVisualiser;
    LevelAnalyser levelAnalyser;
    ProcessorGain processorGain;

    std::unique_ptr<GrainDelay> grainDelay1;
    std::unique_ptr<GrainDelay> grainDelay2;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
