#include "AniraInferenceBackend.h"
#include "ScycloneModelConfig.h"

AniraInferenceBackend::AniraInferenceBackend(RaveModel model, anira::ContextConfig& contextConfigIn)
    : raveModel(model),
      contextConfig(contextConfigIn),
      inferenceConfig(makeScycloneInferenceConfig(model))
{
}

AniraInferenceBackend::~AniraInferenceBackend()
{
    // handler holds InferenceConfig& / PrePostProcessor& — tear it down first.
    handler.reset();
    prePostProcessor.reset();
}

void AniraInferenceBackend::rebuildPipeline()
{
    handler.reset();
    prePostProcessor = std::make_unique<anira::PrePostProcessor>(inferenceConfig);
    handler = std::make_unique<anira::InferenceHandler>(
        *prePostProcessor, inferenceConfig, contextConfig);

    if (lastSpec.sampleRate > 0.0)
    {
        handler->prepare(makeHostConfig(lastSpec));
        latencyInSamples = static_cast<int>(handler->get_latency());
    }

    // A fresh pipeline is pre-filled with silence, so there is nothing stale to flush.
    flushSamplesRemaining = 0;
}

bool AniraInferenceBackend::swapPipeline(const std::function<anira::InferenceConfig()>& makeConfig)
{
    anira::InferenceConfig previousConfig = inferenceConfig;

    try
    {
        // Build first: an invalid path or shape throws here, before anything is torn down.
        anira::InferenceConfig newConfig = makeConfig();

        // anira::SessionElement stores InferenceConfig& and its thread pool keeps reading it
        // after the host callback is suspended — the live session must be gone before we assign.
        handler.reset();
        prePostProcessor.reset();
        inferenceConfig = std::move(newConfig);

        rebuildPipeline();
        return true;
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("Model load failed: " + juce::String(e.what()));
    }
    catch (...)
    {
        juce::Logger::writeToLog("Model load failed: unknown error");
    }

    restorePipeline(std::move(previousConfig));
    return false;
}

void AniraInferenceBackend::restorePipeline(anira::InferenceConfig previousConfig)
{
    handler.reset();
    prePostProcessor.reset();
    inferenceConfig = std::move(previousConfig);

    try
    {
        rebuildPipeline();
    }
    catch (...)
    {
        // The previous model no longer builds either (e.g. an external file that was deleted).
        // Leave the backend inert rather than half-constructed; processBlock passes audio through.
        juce::Logger::writeToLog("Model load failed: could not restore previous model");
        releaseResources();
    }
}

anira::HostConfig AniraInferenceBackend::makeHostConfig(const juce::dsp::ProcessSpec& spec)
{
    return anira::HostConfig{static_cast<float>(spec.maximumBlockSize),
                             static_cast<float>(spec.sampleRate)};
}

void AniraInferenceBackend::prepare(const juce::dsp::ProcessSpec& spec)
{
    lastSpec = spec;

    if (handler == nullptr)
    {
        rebuildPipeline();
        return;
    }

    handler->prepare(makeHostConfig(spec));
    latencyInSamples = static_cast<int>(handler->get_latency());
    flushSamplesRemaining = 0;
}

void AniraInferenceBackend::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (handler == nullptr || buffer.getNumChannels() < 1)
        return;

    // Muted: skip inference entirely. The pipeline freezes, so its buffered audio
    // predates the mute and must be flushed once we resume (see below).
    if (muted.load(std::memory_order_relaxed))
    {
        buffer.clear();
        flushSamplesRemaining = latencyInSamples;
        return;
    }

    handler->process(buffer.getArrayOfWritePointers(),
                     static_cast<size_t>(buffer.getNumSamples()));

    // Drop one latency's worth of output after unmuting: the pipeline is still handing
    // back pre-mute audio. Zeroing keeps the reported latency honest.
    if (flushSamplesRemaining > 0)
    {
        const int toClear = juce::jmin(flushSamplesRemaining, buffer.getNumSamples());
        buffer.clear(0, toClear);
        flushSamplesRemaining -= toClear;
    }
}

int AniraInferenceBackend::getLatencyInSamples() const
{
    return latencyInSamples;
}

void AniraInferenceBackend::setMuted(bool shouldBeMuted)
{
    muted.store(shouldBeMuted, std::memory_order_relaxed);
}

bool AniraInferenceBackend::loadExternalModel(const juce::File& path)
{
    const juce::String modelName = path.getFileNameWithoutExtension();

    if (onModelLoad)
        onModelLoad(true, modelName);

    const std::string modelPath = path.getFullPathName().toStdString();

    // Validate before anira touches the file. A model ORT cannot load throws out of
    // Context::create_session with its active-session counter already incremented, which leaks
    // the shared thread pool and Context singleton for the rest of the process (see
    // validateRaveModelFile). Rejecting bad files here keeps the rollback below survivable.
    std::string validationError;
    const bool valid = validateRaveModelFile(modelPath, validationError);

    if (!valid)
        juce::Logger::writeToLog("Rejected model \"" + modelName + "\": " + validationError);

    const bool loaded = valid && swapPipeline([&modelPath] {
        return makeScycloneInferenceConfigFromPath(modelPath);
    });

    // Fire unconditionally — the host stays suspended if this is skipped. An empty name
    // on failure leaves the displayed model name pointing at what is actually loaded.
    if (onModelLoad)
        onModelLoad(false, loaded ? modelName : juce::String());

    return loaded;
}

bool AniraInferenceBackend::setInternalModel()
{
    if (onModelLoad)
        onModelLoad(true, "");

    const RaveModel model = raveModel;
    const bool loaded = swapPipeline([model] {
        return makeScycloneInferenceConfig(model);
    });

    if (onModelLoad)
        onModelLoad(false, "");

    return loaded;
}

void AniraInferenceBackend::releaseResources()
{
    handler.reset();
    prePostProcessor.reset();
    latencyInSamples = 0;
    flushSamplesRemaining = 0;
}

#ifndef SCYCLONE_INFERENCE_STUB
std::unique_ptr<InferenceBackend> makeInferenceBackend(
    RaveModel model,
    anira::ContextConfig& contextConfig)
{
    return std::make_unique<AniraInferenceBackend>(model, contextConfig);
}
#endif
