#pragma once

/// @file ImpulseAssertions.h
/// @brief gtest contracts for impulse peak position and side-lobe ratio.
///
/// Assert-phase helpers for `ImpulseResponse` metrics from ImpulseMetrics.h.
/// Tolerance constants are tuned via CalibrationProbe DISABLED tests.
///
/// @namespace torsion::test

#include <gtest/gtest.h>

#include "ImpulseMetrics.h"

namespace torsion::test
{

    /// Max secondary impulse lobe as fraction of main peak.
    constexpr float kImpulseSecondaryPeakRatio = 0.15f;

    /// Impulse peak index within @p tolerance of `expectedPeak`.
    inline void assertImpulsePeakWithinTolerance(const ImpulseResponse &response,
                                                 int tolerance = kImpulsePeakToleranceSamples)
    {
        ASSERT_GE(response.narrowPeakPos, 0)
            << "no impulse peak found near expectedPeak=" << response.expectedPeak
            << " widePeakPos=" << response.widePeakPos << " wideDelta="
            << (response.widePeakPos - response.expectedPeak);
        EXPECT_GE(response.narrowPeakPos, response.expectedPeak - tolerance)
            << "expectedPeak=" << response.expectedPeak << " narrowPeakPos=" << response.narrowPeakPos
            << " widePeakPos=" << response.widePeakPos;
        EXPECT_LE(response.narrowPeakPos, response.expectedPeak + tolerance)
            << "expectedPeak=" << response.expectedPeak << " narrowPeakPos=" << response.narrowPeakPos
            << " widePeakPos=" << response.widePeakPos;
    }

    /// No duplicate impulse lobes above @p secondaryRatio × main peak.
    inline void assertImpulseSideLobesBelowThreshold(const ImpulseResponse &response,
                                                     float secondaryRatio = kImpulseSecondaryPeakRatio)
    {
        ASSERT_GE(response.narrowPeakPos, 0);
        ASSERT_GT(response.narrowPeakVal, 1.0e-6f)
            << "narrowPeakPos=" << response.narrowPeakPos
            << " widePeakPos=" << response.widePeakPos
            << " widePeakVal=" << response.widePeakVal;
        EXPECT_LE(response.maxOutsideMainWindow, secondaryRatio * response.narrowPeakVal)
            << "duplicate/truncated impulse lobe; narrowPeakPos=" << response.narrowPeakPos
            << " maxOutside=" << response.maxOutsideMainWindow
            << " narrowPeakVal=" << response.narrowPeakVal;
    }

    /// Combined peak position + side-lobe contract.
    inline void assertImpulseResponse(const ImpulseResponse &response,
                                      int peakTolerance = kImpulsePeakToleranceSamples,
                                      float secondaryRatio = kImpulseSecondaryPeakRatio)
    {
        assertImpulsePeakWithinTolerance(response, peakTolerance);
        assertImpulseSideLobesBelowThreshold(response, secondaryRatio);
    }

} // namespace torsion::test
