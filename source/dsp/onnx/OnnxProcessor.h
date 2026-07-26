//
// Created by valentin.ackva on 10.03.2023.
//

#ifndef VAESYNTH_ONNXPROCESSOR_H
#define VAESYNTH_ONNXPROCESSOR_H

#include "JuceHeader.h"
#include "../IProcessor.h"
#include "InferenceBackend.h"
#include "OnnxModel.h"
#include "../../PluginParameters.h"

#ifndef SCYCLONE_INFERENCE_STUB
namespace anira { struct ContextConfig; }
#endif

class OnnxProcessor : public IProcessor {
public:
#ifndef SCYCLONE_INFERENCE_STUB
    OnnxProcessor(juce::AudioProcessorValueTreeState& apvts,
                  int no,
                  RaveModel raveModel,
                  anira::ContextConfig& contextConfig);
#else
    OnnxProcessor(juce::AudioProcessorValueTreeState& apvts, int no, RaveModel raveModel);
#endif

    void parameterChanged(const juce::String& parameterID, float newValue);
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processBlock(juce::AudioBuffer<float>& buffer) override;
    int getLatencyInSamples() const override;
    /// @return false if the file could not be loaded; the previous model stays active.
    bool loadExternalModel(juce::File path);
    void releaseResources();

    std::function<void(bool initLoading, juce::String modelName)> onOnnxModelLoad;

private:
    juce::AudioProcessorValueTreeState& parameters;
    std::unique_ptr<InferenceBackend> backend;
    int latencyInSamples = 0;
    int number = 0;
};

#endif // VAESYNTH_ONNXPROCESSOR_H
