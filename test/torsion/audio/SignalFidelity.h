#pragma once

/// @file SignalFidelity.h
/// @brief Swept-sine RMS fidelity metrics for latency-aligned round-trip tests.
///
/// Extracted from resampling measurements; shared across Torsion plugin test suites.
///
/// @namespace torsion::test

#include <algorithm>
#include <vector>

#include "SignalMetrics.h"
#include "TestTiming.h"

namespace torsion::test
{

    /// Extra blocks beyond latency-aligned warmup before RMS steady window.
    constexpr int kSineWarmupMarginBlocks = 2;

    /// Block count to flush reported latency before swept-sine RMS capture.
    inline int sineWarmupBlocks(int latencySamples, int hostBlock)
    {
        return (latencySamples + hostBlock - 1) / hostBlock + kSineWarmupMarginBlocks;
    }

    /// Worst normalized RMS error over @p nBlocks aligned at @p latency (output vs input).
    inline float worstSweptSineRmsError(const std::vector<float> &inputSignal,
                                        const std::vector<float> &outputSignal,
                                        int hostBlock, int latency, int nBlocks = kSteadyBlocks)
    {
        float worst = 0.0f;
        for (int block = 0; block < nBlocks; ++block)
        {
            const int inStart = block * hostBlock;
            const int outStart = inStart + latency;
            if (outStart + hostBlock > static_cast<int>(outputSignal.size()))
            {
                break;
            }
            const float inRms = rms(inputSignal, inStart, hostBlock);
            if (inRms <= 1.0e-6f)
            {
                continue;
            }
            std::vector<float> diff(static_cast<size_t>(hostBlock));
            for (int i = 0; i < hostBlock; ++i)
            {
                diff[static_cast<size_t>(i)] =
                    outputSignal[static_cast<size_t>(outStart + i)] - inputSignal[static_cast<size_t>(inStart + i)];
            }
            worst = std::max(worst, rms(diff.data(), hostBlock) / inRms);
        }
        return worst;
    }

} // namespace torsion::test
