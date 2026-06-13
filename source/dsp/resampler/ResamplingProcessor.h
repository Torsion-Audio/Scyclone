#pragma once

#include <optional>
#include <samplerate.h>
#include <JuceHeader.h>

class ResamplingProcessor {
public:
    ResamplingProcessor();
    ~ResamplingProcessor() {
        releaseResources();
    }

    int prepare(const juce::dsp::ProcessSpec &inputSpec,
                double outputSampleRate,
                std::string name,
                std::optional<int> forcedOutputBlockSize = std::nullopt);
    juce::AudioBuffer<float>& processBlock(juce::AudioBuffer<float>& inputBufferMono);
    /** Latency in input-rate samples. */
    int getLatencyInSamples() const { return latencyInSamples; }

    int getInputBufferSize() const { return inputBufferSize; }
    int getOutputBufferSize() const { return outputBufferSize; }
    double getSrcRatio() const { return srcRatio; }
    long getLastOutputFramesGenerated() const { return lastOutputFramesGenerated; }
    long getLastInputFramesUsed() const { return lastInputFramesUsed; }

private:
    int latencyInSamples = 0;
    long lastOutputFramesGenerated = 0;
    long lastInputFramesUsed = 0;
    std::string id_string;

    double inputSampleRate;
    double outputSampleRate;
    int inputBufferSize;
    int outputBufferSize;

    void setSamplerateRatio();
    double srcRatio;  // set in setSamplerateRatio()

    SRC_STATE* converter;

    juce::AudioBuffer<float> outputBufferMono;
    void releaseResources();
};



