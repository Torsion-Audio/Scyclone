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
    int getLatencyInSamples() const { return latencyInSamples; };

private:
    int latencyInSamples = 0;
    std::string processorName;

    double inputSampleRate = 0.0;
    double outputSampleRate = 0.0;
    int inputBufferSize = 0;
    int outputBufferSize = 0;

    double sampleRateRatio = 1.0;
    double bufferSizeRatio = 1.0;

    double calculateSampleRateRatio(double outputRate, double inputRate);
    int calculateOutputBufferSize(double ratio, int blockSize);
    double calculateBufferSizeRatio(int outputBufferSize, int inputBufferSize);
    void measureLatency();
    void printMetrics();

    SRC_STATE* converter = nullptr;

    juce::AudioBuffer<float> outputBuffer;
    void releaseResources();
};



