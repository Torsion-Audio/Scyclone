#pragma once

/// @file TestTiming.h
/// @brief Block-count timing constants and latency-to-pre-roll helpers.
///
/// Timing constants formerly lived in ResamplingTopology.h; shared across Torsion tests.
///
/// @namespace torsion::test

namespace torsion::test
{

    /// Default silence blocks before steady-state assertions.
    constexpr int kPreRollBlocks = 4;
    /// Steady-state blocks for block-size and RMS contract windows.
    constexpr int kSteadyBlocks = 8;
    /// Long-run conservation test block count.
    constexpr int kLongRunBlocks = 100;

    /// Pre-roll block count from bulk reported latency (latency blocks + @ref kPreRollBlocks).
    inline int latencyPreRollBlocks(int totalLatencyHost, int hostBlock,
                                    int extraBlocks = kPreRollBlocks)
    {
        return (totalLatencyHost + hostBlock - 1) / hostBlock + extraBlocks;
    }

} // namespace torsion::test
