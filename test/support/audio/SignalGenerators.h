#pragma once

/// @file SignalGenerators.h
/// @brief Deterministic audio stimuli for behavioral tests.
///
/// Ports of libsamplerate `util.c` signal generators. Frequencies for
/// `genWindowedSines` are normalized to Nyquist (0, 0.5).
///
/// @namespace scyclone::test

#include <algorithm>
#include <cmath>
#include <vector>
#include <JuceHeader.h>

namespace scyclone::test
{

    constexpr float kSweepLowHz = 100.0f;
    constexpr float kSweepHighHz = 8000.0f;
    constexpr float kSweepDurationSec = 1.0f;

    /// Logarithmic swept sine for latency-aligned RMS round-trip tests.
    /// @param blockIndex Global block index (continuous phase across blocks).
    /// @param sampleIndex Sample index within the current block.
    inline float generateSweptSine(double sampleRate, int blockSize, int blockIndex, int sampleIndex)
    {
        const int globalSample = blockIndex * blockSize + sampleIndex;
        const float t = static_cast<float>(globalSample) / static_cast<float>(sampleRate);
        const float logRatio = std::log(kSweepHighHz / kSweepLowHz);
        const float phase = kSweepLowHz * kSweepDurationSec / logRatio *
                            (std::exp(logRatio * t / kSweepDurationSec) - 1.0f) * 2.0f *
                            juce::MathConstants<float>::pi;
        return std::sin(phase);
    }

    /// Hanning-windowed multi-tone buffer (libsamplerate `gen_windowed_sines`).
    /// @param freqs Normalized frequencies in (0, 0.5); length @p freqCount.
    /// @param output Pre-sized buffer of @p outputLen samples.
    inline void genWindowedSines(int freqCount, const double *freqs, double maxAmp,
                                 float *output, int outputLen)
    {
        const double amplitude = maxAmp / static_cast<double>(freqCount);
        std::fill(output, output + outputLen, 0.0f);

        for (int freq = 0; freq < freqCount; ++freq)
        {
            const double phase = 0.9 * juce::MathConstants<double>::pi / static_cast<double>(freqCount);
            for (int k = 0; k < outputLen; ++k)
            {
                output[k] += static_cast<float>(
                    amplitude * std::sin(freqs[freq] * (2.0 * k) * juce::MathConstants<double>::pi + phase));
            }
        }

        for (int k = 0; k < outputLen; ++k)
        {
            output[k] *= static_cast<float>(
                0.5 - 0.5 * std::cos((2.0 * k) * juce::MathConstants<double>::pi / (outputLen - 1)));
        }
    }

} // namespace scyclone::test
