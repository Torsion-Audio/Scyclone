#ifndef SCYCLONE_ANIRAINFERENCEBACKEND_H
#define SCYCLONE_ANIRAINFERENCEBACKEND_H

#include "InferenceBackend.h"
#include <anira/anira.h>
#include <atomic>
#include <functional>
#include <memory>

class AniraInferenceBackend : public InferenceBackend {
public:
    AniraInferenceBackend(RaveModel model, anira::ContextConfig& contextConfig);
    ~AniraInferenceBackend() override;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processBlock(juce::AudioBuffer<float>& buffer) override;
    int getLatencyInSamples() const override;
    bool loadExternalModel(const juce::File& path) override;
    bool setInternalModel() override;
    void setMuted(bool shouldBeMuted) override;
    void releaseResources() override;

private:
    void rebuildPipeline();

    /// Replaces the model with the one produced by @p makeConfig. Tears the live session
    /// down before touching inferenceConfig, and rolls back on failure.
    bool swapPipeline(const std::function<anira::InferenceConfig()>& makeConfig);
    void restorePipeline(anira::InferenceConfig previousConfig);

    static anira::HostConfig makeHostConfig(const juce::dsp::ProcessSpec& spec);

    RaveModel raveModel;
    anira::ContextConfig& contextConfig;
    anira::InferenceConfig inferenceConfig;
    std::unique_ptr<anira::PrePostProcessor> prePostProcessor;
    std::unique_ptr<anira::InferenceHandler> handler;
    juce::dsp::ProcessSpec lastSpec{};
    int latencyInSamples = 0;

    std::atomic<bool> muted{false};
    int flushSamplesRemaining = 0; ///< audio-thread only
};

#endif // SCYCLONE_ANIRAINFERENCEBACKEND_H
