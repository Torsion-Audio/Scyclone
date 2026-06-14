#pragma once

/// @file ResamplingSignalUtils.h
/// @brief Resampling-specific SNR case presets for parameterized signal tests.
///
/// General signal generators and metrics live in `test/torsion/audio/`.
/// FFT peak SNR floors are calibrated per OS via DISABLED `PrintSnrMeasurements`
/// (measured − 3 dB). The same resampler config can report materially different SNR
/// on Windows vs macOS/Linux (toolchain / JUCE FFT / latency-skip alignment);
///
/// @namespace scyclone::test::resampling

#include <random>
#include <vector>

#include <JuceHeader.h>

namespace scyclone::test::resampling
{

    /// Optional corruption applied to resampler output before FFT SNR measurement.
    enum class SnrCorruptionKind
    {
        None,
        HardClip,
        AdditiveNoise
    };

    inline void applySnrCorruption(std::vector<float> &samples, SnrCorruptionKind kind)
    {
        if (kind == SnrCorruptionKind::None || samples.empty())
        {
            return;
        }
        if (kind == SnrCorruptionKind::HardClip)
        {
            for (float &sample : samples)
            {
                sample = juce::jlimit(-1.0f, 1.0f, sample * 50.0f);
            }
            return;
        }
        std::mt19937 rng(42);
        std::uniform_real_distribution<float> dist(-0.05f, 0.05f);
        for (float &sample : samples)
        {
            sample += dist(rng);
        }
    }

    /// Parameter for `ResamplingSignalTest` host/block SNR matrix.
    struct SnrCase
    {
        double hostSR;
        int hostBlock;
        double minUpSnrDb;
        double minDownSnrDb;
        int passBandPeaks;
    };

    /// Authoritative CI SNR floors (update after CalibrationProbe re-run on each OS).
    inline std::vector<SnrCase> defaultCiSnrCases()
    {
#if JUCE_WINDOWS
        // MSVC x64 — PrintSnrMeasurements on Windows (worst 48k/512 ≈ 106.3 dB).
        return {
            {44100.0, 128, 79.0, 100.0, 1},
            {44100.0, 512, 79.0, 92.0, 1},
            {48000.0, 128, 95.0, 95.0, 1},
            {48000.0, 512, 103.0, 103.0, 1},
        };
#elif JUCE_MAC
        // Apple Clang arm64 CI — same configs measure lower than Windows (48k/512 ≈ 83.4 dB).
        return {
            {44100.0, 128, 79.0, 100.0, 1},
            {44100.0, 512, 79.0, 87.0, 1},
            {48000.0, 128, 95.0, 95.0, 1},
            {48000.0, 512, 80.0, 80.0, 1},
        };
#else
        // Linux — conservative macOS-aligned floors until ubuntu PrintSnrMeasurements run.
        return {
            {44100.0, 128, 79.0, 100.0, 1},
            {44100.0, 512, 79.0, 87.0, 1},
            {48000.0, 128, 95.0, 95.0, 1},
            {48000.0, 512, 80.0, 80.0, 1},
        };
#endif
    }

} // namespace scyclone::test::resampling
