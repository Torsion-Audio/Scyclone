#pragma once

/// @file ResamplingSignalUtils.h
/// @brief Resampling-specific SNR case presets for parameterized signal tests.
///
/// General signal generators and metrics live in `test/torsion/audio/`.
/// SNR floors are derived from CalibrationProbe DISABLED tests (measured − 3 dB).
///
/// @namespace scyclone::test::resampling

#include <vector>

namespace scyclone::test::resampling
{

    /// Parameter for `ResamplingSignalTest` host/block SNR matrix.
    struct SnrCase
    {
        double hostSR;
        int hostBlock;
        double minUpSnrDb;
        double minDownSnrDb;
        int passBandPeaks;
    };

    /// Authoritative CI SNR floors (update after CalibrationProbe re-run).
    inline std::vector<SnrCase> defaultCiSnrCases()
    {
        return {
            {44100.0, 128, 79.0, 100.0, 1},
            {44100.0, 512, 79.0, 92.0, 1},
            {48000.0, 128, 95.0, 95.0, 1},
            {48000.0, 512, 103.0, 103.0, 1},
        };
    }

} // namespace scyclone::test::resampling
