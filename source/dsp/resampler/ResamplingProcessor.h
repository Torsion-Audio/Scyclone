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


private:
    std::string id_string; // for debugging printout

    double inputSampleRate;
    double outputSampleRate;
    int inputBufferSize;
    int outputBufferSize;

    void setSamplerateRatio();
    double srcRatio; // input / output

    SRC_STATE* converter;

    juce::AudioBuffer<float> outputBufferMono;
    void releaseResources();
};



