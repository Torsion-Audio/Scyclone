#pragma once

/// @file SimulatedOnnxProcessor.h
/// @brief ONNX-shaped latency preset for production-chain tests.
///
/// Wraps `DelayLineProcessor` with `computeOnnxLatencyInSamples` so chain
/// contracts validate the same latency formula the plugin reports at 48 kHz.
///
/// @namespace scyclone::test::resampling

#include "DelayLineProcessor.h"
#include "dsp/utils/utils.h"

namespace scyclone::test::resampling
{

    constexpr int kDefaultInferenceLatency = 16384 + 4096;

    class SimulatedOnnxProcessor : public scyclone::test::DelayLineProcessor
    {
    public:
        void prepare(const juce::dsp::ProcessSpec &spec) override
        {
            const int blockSize = static_cast<int>(spec.maximumBlockSize);
            const int latency = utils::computeOnnxLatencyInSamples(kDefaultInferenceLatency, blockSize);
            configure(latency, latency);
            DelayLineProcessor::prepare(spec);
        }
    };

} // namespace scyclone::test::resampling
