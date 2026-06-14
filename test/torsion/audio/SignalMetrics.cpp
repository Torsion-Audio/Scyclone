#include "SignalMetrics.h"

/// FFT SNR implementation (libsamplerate calc_snr.c port).
/// Kept in .cpp so non-SNR tests avoid heavy FFT template instantiation.

#include <algorithm>
#include <cmath>
#include <vector>
#include <JuceHeader.h>

namespace torsion::test
{

    namespace detail
    {

        struct PeakData
        {
            double peak = 0.0;
            int index = 0;
        };

        int nextFftOrder(int len)
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

        void logMagSpectrum(const float *input, int len, std::vector<double> &magnitude)
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

        void linearSmooth(std::vector<double> &mag, PeakData &larger, PeakData &smaller)
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

        void smoothMagSpectrum(std::vector<double> &mag)
        {
            const int len = static_cast<int>(mag.size());
            PeakData peaks[2]{};

            for (int k = 1; k < len - 1; ++k)
            {
                if (mag[static_cast<size_t>(k - 1)] < mag[static_cast<size_t>(k)] &&
                    mag[static_cast<size_t>(k)] >= mag[static_cast<size_t>(k + 1)])
                {
                    peaks[0].peak = mag[static_cast<size_t>(k)];
                    peaks[0].index = k;
                    break;
                }
            }

            for (int k = peaks[0].index + 1; k < len - 1; ++k)
            {
                if (mag[static_cast<size_t>(k - 1)] < mag[static_cast<size_t>(k)] &&
                    mag[static_cast<size_t>(k)] >= mag[static_cast<size_t>(k + 1)])
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

        double findSnrFromMagnitude(const std::vector<double> &magnitude, int expectedPeaks)
        {
            constexpr int kMaxPeaks = 10;
            PeakData peaks[kMaxPeaks]{};
            int peakCount = 0;

            for (int k = 1; k < static_cast<int>(magnitude.size()) - 1; ++k)
            {
                if (magnitude[static_cast<size_t>(k - 1)] < magnitude[static_cast<size_t>(k)] &&
                    magnitude[static_cast<size_t>(k)] >= magnitude[static_cast<size_t>(k + 1)])
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

    double calculateSnrDb(const float *data, int len, int expectedPeaks)
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

} // namespace torsion::test
