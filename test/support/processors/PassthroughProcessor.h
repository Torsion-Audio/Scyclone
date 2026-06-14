#pragma once

/// @file PassthroughProcessor.h
/// @brief Zero-latency injectable middle stage for resampler chain tests.
///
/// Implements `IProcessor` with no buffer mutation and `getLatencyInSamples() == 0`.
/// Use as the middle node in round-trip chain contracts so resampler regressions
/// surface without ONNX-shaped latency.
///
/// @namespace scyclone::test

#include "dsp/IProcessor.h"

namespace scyclone::test
{

    class PassthroughProcessor : public IProcessor
    {
    public:
        void processBlock(juce::AudioBuffer<float> &buffer) override { juce::ignoreUnused(buffer); }
    };

} // namespace scyclone::test
