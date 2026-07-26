#pragma once

/// @file SimulatedOnnxProcessor.h
/// @brief ONNX-shaped latency preset for production-chain tests.

#include "DelayLineProcessor.h"
#include "dsp/onnx/OnnxInferenceLatency.h"

namespace scyclone::test::resampling
{

    constexpr int kDefaultInferenceLatency = kOnnxInferenceLatencySamples;

    class SimulatedOnnxProcessor : public torsion::test::DelayLineProcessor
    {
    public:
        void prepare(const juce::dsp::ProcessSpec &spec) override
        {
            juce::ignoreUnused(spec);
            configure(kDefaultInferenceLatency, kDefaultInferenceLatency);
            DelayLineProcessor::prepare(spec);
        }
    };

} // namespace scyclone::test::resampling
