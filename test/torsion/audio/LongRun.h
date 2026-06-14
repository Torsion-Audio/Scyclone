#pragma once

/// @file LongRun.h
/// @brief Sample/frame conservation helpers for long-run structural tests.
///
/// Encapsulates libsamplerate terminate tolerance math and a reusable block-count
/// loop. Tests compare `inputTotal` vs `outputTotal` (or ratio-adjusted expected)
/// after N steady-state blocks.
///
/// @namespace torsion::test

#include <cmath>
#include <functional>

namespace torsion::test
{

    /// Accumulated input/output sample or frame counts over a long-run loop.
    struct LongRunCounts
    {
        long inputTotal = 0;
        long outputTotal = 0;
    };

    /// Acceptable |in − out| for long-run conservation (libsamplerate terminate rule).
    /// @param ratio Effective output/input ratio observed over the run.
    /// @return 2 * ceil(max(ratio, 1/ratio)).
    inline int longRunTolerance(double ratio)
    {
        const int terminate = static_cast<int>(std::ceil((ratio >= 1.0) ? ratio : 1.0 / ratio));
        return 2 * terminate;
    }

    /// Runs @p nBlocks iterations; @p processBlock updates @p counts per block.
    /// Use in tests to avoid duplicating manual accumulation loops.
    inline LongRunCounts runLongBlockLoop(int nBlocks,
                                          const std::function<void(int blockIndex, LongRunCounts &)> &processBlock)
    {
        LongRunCounts counts;
        for (int n = 0; n < nBlocks; ++n)
        {
            processBlock(n, counts);
        }
        return counts;
    }

} // namespace torsion::test
