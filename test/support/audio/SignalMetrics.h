#pragma once

/// @file SignalMetrics.h
/// @brief Sample-level metrics for audio behavioral tests.
///
/// Pure analysis helpers (no gtest, no processor I/O). FFT SNR implementation
/// lives in SignalMetrics.cpp to keep compile times down for non-SNR tests.
///
/// @namespace scyclone::test

#include <algorithm>
#include <cmath>
#include <vector>

namespace scyclone::test
{

    /// RMS of @p count samples starting at @p start in @p samples.
    inline float rms(const std::vector<float> &samples, int start, int count)
    {
        double sum = 0.0;
        for (int i = 0; i < count; ++i)
        {
            const float s = samples[static_cast<size_t>(start + i)];
            sum += static_cast<double>(s) * static_cast<double>(s);
        }
        return static_cast<float>(std::sqrt(sum / static_cast<double>(count)));
    }

    /// RMS of @p count samples from a raw pointer.
    inline float rms(const float *samples, int count)
    {
        double sum = 0.0;
        for (int i = 0; i < count; ++i)
        {
            sum += static_cast<double>(samples[i]) * static_cast<double>(samples[i]);
        }
        return static_cast<float>(std::sqrt(sum / static_cast<double>(count)));
    }

    /// Index of maximum |sample| in [@p searchStart, @p searchEnd); -1 if empty.
    /// Used for impulse and dirac peak localization.
    inline int findImpulsePeak(const std::vector<float> &collected, int searchStart, int searchEnd)
    {
        int peakPos = -1;
        float peakVal = 0.0f;
        searchStart = std::max(0, searchStart);
        searchEnd = std::min(static_cast<int>(collected.size()), searchEnd);
        for (int i = searchStart; i < searchEnd; ++i)
        {
            const float v = std::abs(collected[static_cast<size_t>(i)]);
            if (v > peakVal)
            {
                peakVal = v;
                peakPos = i;
            }
        }
        return peakPos;
    }

    /// Max |x| outside [@p windowStart, @p windowEnd) — detects duplicate impulse lobes.
    inline float maxAbsOutsideWindow(const std::vector<float> &samples, int windowStart, int windowEnd)
    {
        float maxOutside = 0.0f;
        windowStart = std::max(0, windowStart);
        windowEnd = std::min(static_cast<int>(samples.size()), windowEnd);
        for (int i = 0; i < static_cast<int>(samples.size()); ++i)
        {
            if (i >= windowStart && i < windowEnd)
            {
                continue;
            }
            maxOutside = std::max(maxOutside, std::abs(samples[static_cast<size_t>(i)]));
        }
        return maxOutside;
    }

    /// FFT peak SNR in dB (libsamplerate `calculate_snr` port).
    /// @param expectedPeaks Number of pass-band peaks to find before measuring noise floor.
    /// @return SNR in dB, or -1.0 on measurement failure (len < 64 or insufficient peaks).
    double calculateSnrDb(const float *data, int len, int expectedPeaks);

} // namespace scyclone::test
