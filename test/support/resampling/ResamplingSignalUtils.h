#pragma once

// Signal generation and quality metrics (libsamplerate util.c / calc_snr.c patterns).
// Include after TestInfrastructure.h.

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>
#include <JuceHeader.h>
#include "TestInfrastructure.h"

namespace resampling_test
{

    constexpr float kSweepLowHz = 100.0f;
    constexpr float kSweepHighHz = 8000.0f;
    constexpr float kSweepDurationSec = 1.0f;

    // Default SNR floors for ad-hoc checks; defaultCiSnrCases() is authoritative.
    // Values are CalibrationProbe measured minus 3 dB (not a relaxed substitute for unreachable 100 dB).
    constexpr double kDefaultUpSnrDb = 79.0;
    constexpr double kDefaultDownSnrDb = 92.0;

    inline float generateSweptSine(double hostSR, int hostBlock, int blockIndex, int sampleIndex)
    {
        const int globalSample = blockIndex * hostBlock + sampleIndex;
        const float t = static_cast<float>(globalSample) / static_cast<float>(hostSR);
        const float logRatio = std::log(kSweepHighHz / kSweepLowHz);
        const float phase = kSweepLowHz * kSweepDurationSec / logRatio * (std::exp(logRatio * t / kSweepDurationSec) - 1.0f) * 2.0f * juce::MathConstants<float>::pi;
        return std::sin(phase);
    }

    /** Hanning-windowed multi-tone signal (normalized freq in (0, 0.5)). Port of libsamplerate gen_windowed_sines. */
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

    inline float rms(const float *samples, int count)
    {
        double sum = 0.0;
        for (int i = 0; i < count; ++i)
        {
            sum += static_cast<double>(samples[i]) * static_cast<double>(samples[i]);
        }
        return static_cast<float>(std::sqrt(sum / static_cast<double>(count)));
    }

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

    /** Max |x| outside [windowStart, windowEnd) — detects duplicate/truncated impulse lobes. */
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

    namespace detail
    {

        struct PeakData
        {
            double peak = 0.0;
            int index = 0;
        };

        inline int nextFftOrder(int len)
        {
            int order = 0;
            int size = 1;
            while (size < len)
            {
                ++order;
                size <<= 1;
            }
            return order;
        }

        inline void logMagSpectrum(const float *input, int len, std::vector<double> &magnitude)
        {
            const int order = nextFftOrder(len);
            const int fftSize = 1 << order;
            std::vector<float> fftData(static_cast<size_t>(2 * fftSize), 0.0f);
            std::copy(input, input + len, fftData.begin());

            juce::dsp::FFT fft(order);
            fft.performRealOnlyForwardTransform(fftData.data(), false);

            magnitude.assign(static_cast<size_t>(fftSize / 2), 0.0);
            double maxval = 0.0;
            for (int k = 1; k < fftSize / 2; ++k)
            {
                const double re = fftData[static_cast<size_t>(2 * k)];
                const double im = fftData[static_cast<size_t>(2 * k + 1)];
                const double mag = std::sqrt(re * re + im * im);
                magnitude[static_cast<size_t>(k)] = mag;
                maxval = std::max(maxval, mag);
            }

            for (size_t k = 0; k < magnitude.size(); ++k)
            {
                magnitude[k] = (maxval > 0.0) ? magnitude[k] / maxval : 0.0;
                magnitude[k] = (magnitude[k] < 1e-15) ? -200.0 : 20.0 * std::log10(magnitude[k]);
            }
        }

        inline void linearSmooth(std::vector<double> &mag, PeakData &larger, PeakData &smaller)
        {
            if (smaller.index < larger.index)
            {
                for (int k = smaller.index + 1; k < larger.index; ++k)
                {
                    if (mag[static_cast<size_t>(k)] < mag[static_cast<size_t>(k - 1)])
                    {
                        mag[static_cast<size_t>(k)] = 0.999 * mag[static_cast<size_t>(k - 1)];
                    }
                }
            }
            else
            {
                for (int k = smaller.index - 1; k >= larger.index; --k)
                {
                    if (mag[static_cast<size_t>(k)] < mag[static_cast<size_t>(k + 1)])
                    {
                        mag[static_cast<size_t>(k)] = 0.999 * mag[static_cast<size_t>(k + 1)];
                    }
                }
            }
        }

        inline void smoothMagSpectrum(std::vector<double> &mag)
        {
            const int len = static_cast<int>(mag.size());
            PeakData peaks[2]{};

            for (int k = 1; k < len - 1; ++k)
            {
                if (mag[static_cast<size_t>(k - 1)] < mag[static_cast<size_t>(k)] && mag[static_cast<size_t>(k)] >= mag[static_cast<size_t>(k + 1)])
                {
                    peaks[0].peak = mag[static_cast<size_t>(k)];
                    peaks[0].index = k;
                    break;
                }
            }

            for (int k = peaks[0].index + 1; k < len - 1; ++k)
            {
                if (mag[static_cast<size_t>(k - 1)] < mag[static_cast<size_t>(k)] && mag[static_cast<size_t>(k)] >= mag[static_cast<size_t>(k + 1)])
                {
                    peaks[1].peak = mag[static_cast<size_t>(k)];
                    peaks[1].index = k;
                    if (peaks[1].peak > peaks[0].peak)
                    {
                        linearSmooth(mag, peaks[1], peaks[0]);
                    }
                    else
                    {
                        linearSmooth(mag, peaks[0], peaks[1]);
                    }
                    peaks[0] = peaks[1];
                }
            }
        }

        inline double findSnrFromMagnitude(const std::vector<double> &magnitude, int expectedPeaks)
        {
            constexpr int kMaxPeaks = 10;
            PeakData peaks[kMaxPeaks]{};
            int peakCount = 0;

            for (int k = 1; k < static_cast<int>(magnitude.size()) - 1; ++k)
            {
                if (magnitude[static_cast<size_t>(k - 1)] < magnitude[static_cast<size_t>(k)] && magnitude[static_cast<size_t>(k)] >= magnitude[static_cast<size_t>(k + 1)])
                {
                    if (peakCount < kMaxPeaks)
                    {
                        peaks[peakCount].peak = magnitude[static_cast<size_t>(k)];
                        peaks[peakCount].index = k;
                        ++peakCount;
                        std::sort(peaks, peaks + peakCount,
                                  [](const PeakData &a, const PeakData &b)
                                  { return a.peak > b.peak; });
                    }
                    else if (magnitude[static_cast<size_t>(k)] > peaks[kMaxPeaks - 1].peak)
                    {
                        peaks[kMaxPeaks - 1].peak = magnitude[static_cast<size_t>(k)];
                        peaks[kMaxPeaks - 1].index = k;
                        std::sort(peaks, peaks + kMaxPeaks,
                                  [](const PeakData &a, const PeakData &b)
                                  { return a.peak > b.peak; });
                    }
                }
            }

            if (peakCount < expectedPeaks)
            {
                return -1.0;
            }

            std::sort(peaks, peaks + peakCount,
                      [](const PeakData &a, const PeakData &b)
                      { return a.peak > b.peak; });

            double snr = peaks[0].peak;
            for (int k = 1; k < peakCount; ++k)
            {
                if (std::abs(snr - peaks[k].peak) > 10.0)
                {
                    return std::abs(peaks[k].peak);
                }
            }
            return snr;
        }

    } // namespace detail

    /** FFT peak SNR in dB (port of libsamplerate calculate_snr). Returns -1 on error. */
    inline double calculateSnrDb(const float *data, int len, int expectedPeaks)
    {
        if (len < 64)
        {
            return -1.0;
        }

        int paddedLen = len;
        while ((paddedLen & 0x1F) != 0)
        {
            ++paddedLen;
        }

        std::vector<float> copy(static_cast<size_t>(paddedLen), 0.0f);
        std::copy(data, data + len, copy.begin());

        std::vector<double> magnitude;
        detail::logMagSpectrum(copy.data(), paddedLen, magnitude);
        detail::smoothMagSpectrum(magnitude);
        return detail::findSnrFromMagnitude(magnitude, expectedPeaks);
    }

    struct SnrCase
    {
        double hostSR;
        int hostBlock;
        double minUpSnrDb;
        double minDownSnrDb;
        int passBandPeaks;
    };

    inline std::vector<SnrCase> defaultCiSnrCases()
    {
        // Floors = CalibrationProbe DISABLED_PrintSnrMeasurements worst-case minus 3 dB margin.
        return {
            {44100.0, 128, 79.0, 100.0, 1},
            {44100.0, 512, 79.0, 92.0, 1},
            {48000.0, 128, 95.0, 95.0, 1},
            {48000.0, 512, 103.0, 103.0, 1},
        };
    }

} // namespace resampling_test
