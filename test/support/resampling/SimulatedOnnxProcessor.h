#pragma once

// ONNX-shaped latency preset for production-chain formula tests (48 kHz middle stage).

#include "DelayLineProcessor.h"
#include "dsp/utils/utils.h"

namespace resampling_test {

constexpr int kDefaultInferenceLatency = 16384 + 4096;

class SimulatedOnnxProcessor : public DelayLineProcessor {
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override {
        const int blockSize = static_cast<int>(spec.maximumBlockSize);
        const int latency = utils::computeOnnxLatencyInSamples(kDefaultInferenceLatency, blockSize);
        configure(latency, latency);
        DelayLineProcessor::prepare(spec);
    }
};

} // namespace resampling_test
