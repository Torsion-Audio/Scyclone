#pragma once

#include <samplerate.h>
#include <JuceHeader.h>

class ResamplingProcessor {
public:
    ResamplingProcessor() = default;
    ~ResamplingProcessor() {
        releaseResources();
    }

    int prepare(const juce::dsp::ProcessSpec &inputSpec, double targetSampleRate, const std::string name);
    void setOutputBufferSize(int size);
    juce::AudioBuffer<float>& processBlock(juce::AudioBuffer<float>& inputBufferMono);
    /** Latency in input-rate samples. */
    int getLatencyInSamples() const { return latencyInSamples; }

    int getInputBufferSize() const { return inputBufferSize; }
    int getOutputBufferSize() const { return outputBufferSize; }
    /** Effective per-block ratio passed to libsamplerate (N_out / N_in). */
    double getSrcRatio() const { return bufferSizeRatio; }
    long getLastOutputFramesGenerated() const { return lastOutputFramesGenerated; }
    long getLastInputFramesUsed() const { return lastInputFramesUsed; }

private:
    int latencyInSamples = 0;
    long lastOutputFramesGenerated = 0;
    long lastInputFramesUsed = 0;
    std::string processorName;

    double inputSampleRate = 0.0;
    double outputSampleRate = 0.0;
    int inputBufferSize = 0;
    int outputBufferSize = 0;

    double sampleRateRatio = 1.0;
    double bufferSizeRatio = 1.0;

    double calculateSampleRateRatio(double outputRate, double inputRate);
    int calculateOutputBufferSize(double ratio, int blockSize);
    double calculateBufferSizeRatio(int outBufferSize, int inBufferSize);
    void measureLatency();
    void printMetrics();

    SRC_STATE* converter = nullptr;

    juce::AudioBuffer<float> outputBuffer;
    void releaseResources();
};



