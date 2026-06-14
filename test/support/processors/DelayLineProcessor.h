#pragma once

// Configurable FIFO delay line at the processing (typically 48 kHz) rate.

#include <vector>
#include <JuceHeader.h>
#include "dsp/IProcessor.h"

namespace resampling_test {

class DelayLineProcessor : public IProcessor {
public:
    void configure(int fifoLengthSamples, int reportedLatencySamples = -1) {
        fifoLength = fifoLengthSamples;
        reportedLatency = (reportedLatencySamples >= 0) ? reportedLatencySamples : fifoLengthSamples;
    }

    void prepare(const juce::dsp::ProcessSpec& spec) override {
        juce::ignoreUnused(spec);
        delayLine.assign(static_cast<size_t>(std::max(fifoLength, 0)), 0.0f);
        writeIndex = 0;
        filled = 0;
    }

    void processBlock(juce::AudioBuffer<float>& buffer) override {
        const int numSamples = buffer.getNumSamples();
        if (fifoLength <= 0) {
            return;
        }
        for (int i = 0; i < numSamples; ++i) {
            const float in = buffer.getSample(0, i);
            const float out = delayLine[static_cast<size_t>(writeIndex)];
            delayLine[static_cast<size_t>(writeIndex)] = in;
            writeIndex = (writeIndex + 1) % fifoLength;
            if (filled < fifoLength) {
                ++filled;
                buffer.setSample(0, i, 0.0f);
            } else {
                buffer.setSample(0, i, out);
            }
        }
    }

    int getLatencyInSamples() const override { return reportedLatency; }

private:
    int fifoLength = 0;
    int reportedLatency = 0;
    int writeIndex = 0;
    int filled = 0;
    std::vector<float> delayLine;
};

} // namespace resampling_test
