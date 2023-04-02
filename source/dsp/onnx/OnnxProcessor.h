//
// Created by valentin.ackva on 10.03.2023.
//

#ifndef VAESYNTH_ONNXPROCESSOR_H
#define VAESYNTH_ONNXPROCESSOR_H

#include "JuceHeader.h"
#include "RingBuffer.h"
#include "InferenceThread.h"
#include "../../PluginParameters.h"
#include "WarningWindow.h"

class OnnxProcessor {
public:
    OnnxProcessor(juce::AudioProcessorValueTreeState &apvts, int no, RaveModel raveModel);

    void parameterChanged(const juce::String &parameterID, float newValue);
    void prepare(const juce::dsp::ProcessSpec& spec);
    void processBlock(juce::AudioBuffer<float>& buffer);
    int getLatency() const;
    void loadExternalModel(juce::File path);

    std::function<void(bool initLoading, juce::String modelName)> onOnnxModelLoad;

private:
    void processOutput(juce::AudioBuffer<float>& buffer, int numSamples);
    void calculateLatency(int maxSamplesPerBuffer);


private:
    juce::AudioProcessorValueTreeState& parameters;

    InferenceThread inferenceThread;
    int latencyInSamples = 0;
    RingBuffer receiveRingBuffer;
    juce::AudioBuffer<float> monoBuffer;
    int inferenceCounter = 0;
    std::unique_ptr<juce::FileChooser> fc;
    WarningWindow warningWindow;
    int number;
};

#endif //VAESYNTH_ONNXPROCESSOR_H
