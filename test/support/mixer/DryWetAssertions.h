#pragma once

/// @file DryWetAssertions.h
/// @brief gtest contract for user-audible dry/wet time alignment.
///
/// Assert-phase wrapper around `measureDryWetDiracPeak`. Tolerance constants
/// are tuned via CalibrationProbe DISABLED tests.
///
/// @namespace scyclone::test::mixer

#include <gtest/gtest.h>
#include "DryWetMeasurements.h"

namespace scyclone::test::mixer
{

    /// CI dirac peak search half-width (samples).
    constexpr int kDiracAlignmentTolerance = 2;
    /// Wider search for calibration jitter probes.
    constexpr int kCalibrationDiracSearchHalfWindow = 20;

    /// Mixed dirac peak must land at diracPos + totalLatency within tolerance.
    inline void assertDryWetDiracAligned(double hostSR, uint32_t blockSize)
    {
        const auto measurement = measureDryWetDiracPeak(
            hostSR, static_cast<int>(blockSize), kDiracAlignmentTolerance);

        const int searchStart = measurement.expectedPeak - kDiracAlignmentTolerance;
        const int searchEnd = measurement.expectedPeak + kDiracAlignmentTolerance + 1;

        EXPECT_GE(measurement.peakPos, searchStart)
            << "wet-aligned peak not found; expectedPeak=" << measurement.expectedPeak;
        EXPECT_LE(measurement.peakPos, searchEnd - 1)
            << "wet-aligned peak not found; expectedPeak=" << measurement.expectedPeak;
        if (measurement.peakPos >= 0)
        {
            EXPECT_GE(measurement.peakAmplitude, 0.01f)
                << "mixed signal should have a detectable peak; expectedPeak=" << measurement.expectedPeak;
        }
    }

} // namespace scyclone::test::mixer

namespace resampling_test
{
    using scyclone::test::mixer::assertDryWetDiracAligned;
    using scyclone::test::mixer::kCalibrationDiracSearchHalfWindow;
    using scyclone::test::mixer::kDiracAlignmentTolerance;
} // namespace resampling_test
