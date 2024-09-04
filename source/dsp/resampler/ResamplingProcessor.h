#pragma once

#include <samplerate.h>
#include <JuceHeader.h>

class ResamplingProcessor {
public:
    ResamplingProcessor();
    ~ResamplingProcessor() {
        releaseResources();
    }

    void prepare(double inputSampleRate, double outputSampleRate);
    void processBlock(juce::AudioBuffer<float>& inputBufferMono, juce::AudioBuffer<float>& outBufferMono);


private:
    SRC_STATE* converter;
    juce::AudioBuffer<float>* inputBuffer;
    juce::AudioBuffer<float>* outputBuffer;

    double srcRatio;

    void setSamplerateRatio(float inputSampleRate, float outputSampleRate) {
        srcRatio = outputSampleRate/inputSampleRate;

        // to resample to even samples
        // int numberOfTargetSamples = static_cast<int>(srcRatio * bufferSize);
        // srcRatio = numberOfSamplesTargetSampleRate / hostbuffersize;

        // for debugging
        // timePerBlock = inputBuffer / inputSampleRate;
        // correctedSampleRate = numberOfTargetSamples / timePerBlockInSec;
    }

    void releaseResources();
};



