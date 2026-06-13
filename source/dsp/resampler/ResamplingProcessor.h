#pragma once

#include <samplerate.h>
#include <JuceHeader.h>

class ResamplingProcessor {
public:
    ResamplingProcessor();
    ~ResamplingProcessor() {
        releaseResources();
    }

    int prepare(const juce::dsp::ProcessSpec &inputSpec, double targetSampleRate, const std::string name,
                                 int providedOutputBufferSize = -1);
    juce::AudioBuffer<float>& processBlock(juce::AudioBuffer<float>& inputBufferMono);
    /** Latency in input-rate samples. */
    int getLatencyInSamples() const { return latencyInSamples; }

private:
    int latencyInSamples = 0;
    std::string id_string;

    double inputSampleRate = 0.0;
    double outputSampleRate = 0.0;
    int inputBufferSize = 0;
    int outputBufferSize = 0;

    void calculateOutputBufferSize();
    void setSamplerateRatio();
    double srcRatio;  // set in setSamplerateRatio()

    SRC_STATE* converter;

    juce::AudioBuffer<float> outputBufferMono;
    void releaseResources();
};



