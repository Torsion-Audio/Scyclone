#pragma once

#include <samplerate.h>
#include <JuceHeader.h>

class ResamplingProcessor {
public:
    ResamplingProcessor();
    ~ResamplingProcessor() {
        releaseResources();
    }

    int prepare(const juce::dsp::ProcessSpec &inputSpec, double outputSampleRate, std::string name);
    juce::AudioBuffer<float>& processBlock(juce::AudioBuffer<float>& inputBufferMono);
    /** Latency in input-rate samples. */
    int getLatencyInSamples() const { return latencyInSamples; }

private:
    int latencyInSamples = 0;
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



