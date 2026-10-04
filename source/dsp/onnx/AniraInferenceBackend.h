#ifndef SCYCLONE_ANIRAINFERENCEBACKEND_H
#define SCYCLONE_ANIRAINFERENCEBACKEND_H

#include "InferenceBackend.h"
#include <anira/anira.h>
#include <atomic>
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
    bool isMuted() const override;
    void setNonRealtime(bool isNonRealtime) noexcept override;
    void releaseResources() override;

    /// The backend anira is currently running, or CUSTOM (passthrough) when no pipeline exists.
    anira::InferenceBackend activeAniraBackend() const;

private:
    /// The model the pipeline is built from. External models are read into memory once, so a
    /// rebuild (every prepare after releaseResources) never depends on the file still existing.
    struct ModelSource
    {
        /// Owns an external model's bytes; null for the embedded models (BinaryData). Declared
        /// before config, whose binary anira::ModelData only points into it.
        std::shared_ptr<const juce::MemoryBlock> bytes;
        anira::InferenceConfig config;
    };

    void rebuildPipeline();

    /// Replaces the model with @p next. Tears the live session down before touching
    /// currentModel, and rolls back to the previous model on failure.
    bool swapPipeline(ModelSource next);
    void restorePipeline(ModelSource previous);

    static anira::HostConfig makeHostConfig(const juce::dsp::ProcessSpec& spec);

    RaveModel raveModel;
    anira::ContextConfig& contextConfig;
    ModelSource currentModel; ///< never reassigned while a handler is alive (anira holds config&)
    std::unique_ptr<anira::PrePostProcessor> prePostProcessor;
    std::unique_ptr<anira::InferenceHandler> handler;
    juce::dsp::ProcessSpec lastSpec{};
    int latencyInSamples = 0;

    std::atomic<bool> muted{false};
    int flushSamplesRemaining = 0; ///< audio-thread only

    std::atomic<bool> nonRealtime{false}; ///< what the host asked for (any thread)
    bool appliedNonRealtime = false;      ///< what the current session runs with
};

#endif // SCYCLONE_ANIRAINFERENCEBACKEND_H
