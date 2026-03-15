#pragma once

#include <JuceHeader.h>

class IProcessor {
public:
    virtual ~IProcessor() = default;
    virtual void prepare(const juce::dsp::ProcessSpec& /* spec */) {}  // optional; default no-op
    virtual void processBlock(juce::AudioBuffer<float>& buffer) = 0;
    virtual int getLatencyInSamples() const { return 0; }
};
