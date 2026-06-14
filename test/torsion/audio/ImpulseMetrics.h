#pragma once

/// @file ImpulseMetrics.h
/// @brief Impulse response capture metrics (peak search, side-lobe analysis).
///
/// Extracted from resampling measurements; assert-phase contracts live in
/// `torsion::test::ImpulseAssertions.h`.
///
/// @namespace torsion::test

#include <cmath>
#include <vector>

#include "SignalMetrics.h"

namespace torsion::test
{

    constexpr int kImpulsePeakWindowHalfWidth = 5;
    /// Wider search when narrow window misses (debug / calibration).
    constexpr int kImpulseWideSearchHalfWidth = 128;
    /// Allowed peak index error vs reported latency (samples).
    constexpr int kImpulsePeakToleranceSamples = 5;

    /// Captured impulse samples plus narrow/wide peak search metrics.
    struct ImpulseResponse
    {
        std::vector<float> samples;
        int expectedPeak = 0;
        int hostBlock = 0;
        int narrowPeakPos = -1;
        float narrowPeakVal = 0.f;
        int widePeakPos = -1;       ///< Fallback search when narrow window misses.
        float widePeakVal = 0.f;
        float maxOutsideMainWindow = 0.f; ///< Side-lobe metric for duplicate detection.
    };

    /// Populates narrow/wide peak fields on @p response from @p response.samples.
    inline void fillImpulsePeakMetrics(ImpulseResponse &response)
    {
        const int expected = response.expectedPeak;
        const int hostBlock = response.hostBlock;
        const auto &collected = response.samples;
        if (collected.empty())
        {
            return;
        }

        const int wideStart = std::max(0, expected - kImpulseWideSearchHalfWidth);
        const int wideEnd = std::min(static_cast<int>(collected.size()),
                                     expected + hostBlock + kImpulseWideSearchHalfWidth);
        response.widePeakPos = findImpulsePeak(collected, wideStart, wideEnd);
        if (response.widePeakPos >= 0)
        {
            response.widePeakVal = std::abs(collected[static_cast<size_t>(response.widePeakPos)]);
        }

        const int narrowStart = std::max(0, expected - kImpulsePeakWindowHalfWidth);
        const int narrowEnd = expected + hostBlock;
        response.narrowPeakPos = findImpulsePeak(collected, narrowStart, narrowEnd);
        if (response.narrowPeakPos >= 0)
        {
            response.narrowPeakVal = std::abs(collected[static_cast<size_t>(response.narrowPeakPos)]);
            const int windowStart = std::max(0, response.narrowPeakPos - kImpulsePeakWindowHalfWidth);
            const int windowEnd = std::min(static_cast<int>(collected.size()),
                                           response.narrowPeakPos + hostBlock);
            response.maxOutsideMainWindow = maxAbsOutsideWindow(collected, windowStart, windowEnd);
        }
    }

} // namespace torsion::test
