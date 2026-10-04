#include "AniraInferenceBackend.h"
#include "ScycloneModelConfig.h"

namespace {

/// anira's latency calculator throws std::invalid_argument for a non-positive block size or
/// sample rate, so a spec has to pass this before it reaches InferenceHandler::prepare().
bool isPreparable(const juce::dsp::ProcessSpec& spec)
{
    return spec.sampleRate > 0.0 && spec.maximumBlockSize > 0;
}

/// Upper bound for a user model read into memory. The bundled RAVE models are about 16.5 MB.
constexpr juce::int64 kMaxModelFileBytes = 512 * 1024 * 1024;

/// Reads a user model into memory. The bytes are owned by the backend from here on, so the file
/// may be moved or deleted afterwards. juce::File reads through the wide-character Windows API,
/// so paths with non-ASCII characters work too.
std::shared_ptr<const juce::MemoryBlock> readModelFile(const juce::File& path, std::string& errorOut)
{
    const auto fileSize = path.getSize();
    if (fileSize <= 0)
    {
        errorOut = "the file is empty or cannot be read";
        return nullptr;
    }
    if (fileSize > kMaxModelFileBytes)
    {
        errorOut = "the file is larger than 512 MB";
        return nullptr;
    }

    auto bytes = std::make_shared<juce::MemoryBlock>();
    if (!path.loadFileAsData(*bytes) || bytes->getSize() == 0)
    {
        errorOut = "the file cannot be read";
        return nullptr;
    }
    return bytes;
}

} // namespace

AniraInferenceBackend::AniraInferenceBackend(RaveModel model, anira::ContextConfig& contextConfigIn)
    : raveModel(model),
      contextConfig(contextConfigIn),
      currentModel{nullptr, makeScycloneInferenceConfig(model)}
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
    prePostProcessor = std::make_unique<anira::PrePostProcessor>(currentModel.config);
    handler = std::make_unique<anira::InferenceHandler>(
        *prePostProcessor, currentModel.config, contextConfig);

    // Select the model explicitly. Before anira v2.3.0 a session started on the CUSTOM backend,
    // whose default processor copies input to output, so without this the RAVE model was loaded
    // but never run. v2.3.0 defaults to the first configured model; don't rely on either default.
    handler->set_inference_backend(anira::InferenceBackend::ONNX);

    // A new session starts in realtime mode, so carry the host's offline flag over. anira only
    // accepts set_non_realtime() once the session and its thread pool exist, i.e. from here on.
    appliedNonRealtime = false;
    if (nonRealtime.load(std::memory_order_relaxed))
    {
        handler->set_non_realtime(true);
        appliedNonRealtime = true;
    }

    if (isPreparable(lastSpec))
    {
        handler->prepare(makeHostConfig(lastSpec));
        latencyInSamples = static_cast<int>(handler->get_latency());
    }

    // A fresh pipeline is pre-filled with silence, so there is nothing stale to flush.
    flushSamplesRemaining = 0;
}

bool AniraInferenceBackend::swapPipeline(ModelSource next)
{
    // Shares the previous model's bytes, so they stay alive for the rollback below.
    ModelSource previous = currentModel;

    try
    {
        // anira's SessionElement, InferenceManager, InferenceHandler and PrePostProcessor hold
        // InferenceConfig&, and the thread pool keeps reading it after the host callback is
        // suspended — the live session must be gone before currentModel is reassigned.
        handler.reset();
        prePostProcessor.reset();
        currentModel = std::move(next);

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

    restorePipeline(std::move(previous));
    return false;
}

void AniraInferenceBackend::restorePipeline(ModelSource previous)
{
    handler.reset();
    prePostProcessor.reset();
    currentModel = std::move(previous);

    try
    {
        rebuildPipeline();
    }
    catch (...)
    {
        // The previous model is still in memory, so this can only be a systemic failure (out of
        // memory, ONNX Runtime refusing to start). Stay inert and silent; prepare() retries.
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
    if (!isPreparable(spec))
        return;

    lastSpec = spec;

    // This runs inside the host's prepareToPlay, so nothing may escape. A pipeline that cannot be
    // built leaves the backend inert (silent, zero latency); the next prepare() tries again.
    try
    {
        if (handler == nullptr)
        {
            rebuildPipeline();
            return;
        }

        handler->prepare(makeHostConfig(spec));
        latencyInSamples = static_cast<int>(handler->get_latency());
        flushSamplesRemaining = 0;
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("Inference pipeline could not be prepared: " + juce::String(e.what()));
        releaseResources();
    }
    catch (...)
    {
        juce::Logger::writeToLog("Inference pipeline could not be prepared: unknown error");
        releaseResources();
    }
}

void AniraInferenceBackend::processBlock(juce::AudioBuffer<float>& buffer)
{
    // No pipeline (not prepared yet, or a failed rebuild): emit silence. Passing the input
    // through would put undelayed audio on the wet path while the dry path is delayed by the
    // reported latency.
    if (handler == nullptr)
    {
        buffer.clear();
        return;
    }
    if (buffer.getNumChannels() < 1)
        return;

    // Apply a change of the host's offline flag here rather than in setNonRealtime(): JUCE's VST3
    // wrapper calls that on every block, before its suspended check, so it can run while a model
    // swap on the message thread is replacing the handler. This is one call per mode change.
    if (const bool wanted = nonRealtime.load(std::memory_order_relaxed); wanted != appliedNonRealtime)
    {
        handler->set_non_realtime(wanted);
        appliedNonRealtime = wanted;
    }

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

anira::InferenceBackend AniraInferenceBackend::activeAniraBackend() const
{
    return handler != nullptr ? handler->get_inference_backend() : anira::InferenceBackend::CUSTOM;
}

void AniraInferenceBackend::setMuted(bool shouldBeMuted)
{
    muted.store(shouldBeMuted, std::memory_order_relaxed);
}

bool AniraInferenceBackend::isMuted() const
{
    return muted.load(std::memory_order_relaxed);
}

void AniraInferenceBackend::setNonRealtime(bool isNonRealtime) noexcept
{
    nonRealtime.store(isNonRealtime, std::memory_order_relaxed);
}

bool AniraInferenceBackend::loadExternalModel(const juce::File& path)
{
    const juce::String modelName = path.getFileNameWithoutExtension();
    bool loaded = false;

    if (onModelLoad)
        onModelLoad(true, modelName);

    // Fire on every path, including an exception below — the host stays suspended (silent)
    // until it does. An empty name on failure keeps the displayed name on what is loaded.
    const juce::ScopeGuard notifyCompletion{[&] {
        if (onModelLoad)
            onModelLoad(false, loaded ? modelName : juce::String());
    }};

    try
    {
        // Validate before anira sees the model: a wrong element type or tensor shape would load
        // and then produce silence or garbage (see validateRaveModel), and rejecting here gives
        // the user a clear reason instead of a failed rollback.
        std::string error;
        auto bytes = readModelFile(path, error);
        if (bytes != nullptr && validateRaveModel(bytes->getData(), bytes->getSize(), error))
        {
            auto config = makeScycloneInferenceConfig(bytes->getData(), bytes->getSize());
            loaded = swapPipeline(ModelSource{std::move(bytes), std::move(config)});
        }
        else
        {
            juce::Logger::writeToLog("Rejected model \"" + modelName + "\": " + error);
        }
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("Model load failed: " + juce::String(e.what()));
    }
    catch (...)
    {
        juce::Logger::writeToLog("Model load failed: unknown error");
    }

    return loaded;
}

bool AniraInferenceBackend::setInternalModel()
{
    bool loaded = false;

    if (onModelLoad)
        onModelLoad(true, "");

    const juce::ScopeGuard notifyCompletion{[&] {
        if (onModelLoad)
            onModelLoad(false, "");
    }};

    try
    {
        loaded = swapPipeline(ModelSource{nullptr, makeScycloneInferenceConfig(raveModel)});
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("Model load failed: " + juce::String(e.what()));
    }
    catch (...)
    {
        juce::Logger::writeToLog("Model load failed: unknown error");
    }

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
