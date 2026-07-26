#ifndef SCYCLONE_INFERENCEBACKEND_H
#define SCYCLONE_INFERENCEBACKEND_H

#include "JuceHeader.h"
#include "OnnxModel.h"
#include <functional>
#include <memory>

class InferenceBackend {
public:
    virtual ~InferenceBackend() = default;

    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    virtual void processBlock(juce::AudioBuffer<float>& buffer) = 0;
    virtual int getLatencyInSamples() const = 0;

    /// @return true on success. On failure the previous model stays loaded and
    /// onModelLoad(false, ...) still fires — callers must never be left suspended.
    virtual bool loadExternalModel(const juce::File& path) = 0;
    virtual bool setInternalModel() = 0;

    /// Muted backends skip inference entirely and emit silence (the wet path is
    /// discarded downstream anyway); unmuting must not leak pre-mute audio.
    virtual void setMuted(bool shouldBeMuted) = 0;

    virtual void releaseResources() {}

    std::function<void(bool initLoading, juce::String modelName)> onModelLoad;
};

#ifndef SCYCLONE_INFERENCE_STUB
namespace anira { struct ContextConfig; }
std::unique_ptr<InferenceBackend> makeInferenceBackend(
    RaveModel model,
    anira::ContextConfig& contextConfig);
#else
std::unique_ptr<InferenceBackend> makeInferenceBackend(RaveModel model);
#endif

#endif // SCYCLONE_INFERENCEBACKEND_H
