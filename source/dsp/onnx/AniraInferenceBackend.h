#ifndef SCYCLONE_ANIRAINFERENCEBACKEND_H
#define SCYCLONE_ANIRAINFERENCEBACKEND_H

#include "InferenceBackend.h"
#include <anira/anira.h>
#include <memory>

class AniraInferenceBackend : public InferenceBackend {
public:
    AniraInferenceBackend(RaveModel model, anira::ContextConfig& contextConfig);
    ~AniraInferenceBackend() override;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processBlock(juce::AudioBuffer<float>& buffer) override;
    int getLatencyInSamples() const override;
    void loadExternalModel(const juce::File& path) override;
    void setInternalModel() override;
    void releaseResources() override;

private:
    void rebuildPipeline();

    RaveModel raveModel;
    anira::ContextConfig& contextConfig;
    anira::InferenceConfig inferenceConfig;
    std::unique_ptr<anira::PrePostProcessor> prePostProcessor;
    std::unique_ptr<anira::InferenceHandler> handler;
    juce::dsp::ProcessSpec lastSpec{};
    int latencyInSamples = 0;
};

#endif // SCYCLONE_ANIRAINFERENCEBACKEND_H
