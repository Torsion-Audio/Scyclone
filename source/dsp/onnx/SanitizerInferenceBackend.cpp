#include "SanitizerInferenceBackend.h"
#include "OnnxInferenceLatency.h"

SanitizerInferenceBackend::SanitizerInferenceBackend(RaveModel model) : raveModel(model) {}

void SanitizerInferenceBackend::prepare(const juce::dsp::ProcessSpec& spec)
{
    juce::ignoreUnused(spec);
    latencyInSamples = kOnnxInferenceLatencySamples;
    fifoLength = kOnnxInferenceLatencySamples;
    delayLine.assign(static_cast<size_t>(std::max(fifoLength, 0)), 0.0f);
    writeIndex = 0;
    filled = 0;
}

void SanitizerInferenceBackend::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    if (fifoLength <= 0 || buffer.getNumChannels() < 1)
        return;

    // Mirror AniraInferenceBackend: muting freezes the delay line rather than draining it,
    // so the contract tests see the same mute/unmute behaviour as production.
    if (muted.load(std::memory_order_relaxed))
    {
        buffer.clear();
        filled = 0;
        return;
    }

    for (int i = 0; i < numSamples; ++i)
    {
        const float in = buffer.getSample(0, i);
        const float out = delayLine[static_cast<size_t>(writeIndex)];
        delayLine[static_cast<size_t>(writeIndex)] = in;
        writeIndex = (writeIndex + 1) % fifoLength;

        if (filled < fifoLength)
        {
            ++filled;
            buffer.setSample(0, i, 0.0f);
        }
        else
        {
            buffer.setSample(0, i, out);
        }
    }
}

int SanitizerInferenceBackend::getLatencyInSamples() const
{
    return latencyInSamples;
}

bool SanitizerInferenceBackend::loadExternalModel(const juce::File& path)
{
    if (onModelLoad)
        onModelLoad(true, path.getFileNameWithoutExtension());
    if (onModelLoad)
        onModelLoad(false, path.getFileNameWithoutExtension());
    return true;
}

bool SanitizerInferenceBackend::setInternalModel()
{
    juce::ignoreUnused(raveModel);
    if (onModelLoad)
        onModelLoad(true, "");
    if (onModelLoad)
        onModelLoad(false, "");
    return true;
}

void SanitizerInferenceBackend::setMuted(bool shouldBeMuted)
{
    muted.store(shouldBeMuted, std::memory_order_relaxed);
}

void SanitizerInferenceBackend::releaseResources()
{
    delayLine.clear();
    filled = 0;
    writeIndex = 0;
}

#ifdef SCYCLONE_INFERENCE_STUB
std::unique_ptr<InferenceBackend> makeInferenceBackend(RaveModel model)
{
    return std::make_unique<SanitizerInferenceBackend>(model);
}
#endif
