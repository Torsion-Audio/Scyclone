#pragma once

/// @file DryWetMeasurements.h
/// @brief Dry/wet dirac alignment measurements (mixer domain).
///
/// Runs the production resampling chain internally, then mixes dry/wet buffers
/// through `DryWetMixer`. Returns peak position for assert-phase checks.
/// Does not call gtest macros.
///
/// @namespace scyclone::test::mixer

#include <climits>
#include <vector>
#include <JuceHeader.h>

#include "ResamplingRunner.h"
#include "SignalMetrics.h"
#include "dsp/mixer/DryWetMixer.h"

namespace scyclone::test::mixer
{

    /// Result of dirac stimulus through production chain + DryWetMixer.
    struct DryWetDiracMeasurement
    {
        int peakPos = -1;       ///< Index of mixed peak, or -1 if not found.
        int expectedPeak = 0;   ///< diracPos + productionChainTotalLatency.
        float peakAmplitude = 0.0f;
    };

    /// Dirac at block center (dry) + full chain (wet) → search mixed peak.
    /// @param searchHalfWindow Half-width of peak search around expectedPeak.
    inline DryWetDiracMeasurement measureDryWetDiracPeak(double hostSR, int hostBlock,
                                                           int searchHalfWindow)
    {
        using namespace scyclone::test::resampling;

        DryWetDiracMeasurement result;
        auto prod = prepareProductionChain(hostSR, hostBlock);
        const int totalLatency = productionChainTotalLatency(prod, hostSR);
        const int diracPos = hostBlock / 2;
        result.expectedPeak = diracPos + totalLatency;

        runProductionSilencePreRoll(prod, productionPreRollBlocks(totalLatency, hostBlock));

        const int collectSamples = totalLatency + hostBlock + diracPos;
        std::vector<float> wetCollected;
        std::vector<float> dryCollected;
        wetCollected.reserve(static_cast<size_t>(collectSamples));
        dryCollected.reserve(static_cast<size_t>(collectSamples));

        prod.resamplers.hostBuffer.setSample(0, diracPos, 1.0f);

        juce::AudioBuffer<float> onnxBuf;
        while (static_cast<int>(wetCollected.size()) < collectSamples)
        {
            for (int i = 0; i < prod.resamplers.hostBuffer.getNumSamples(); ++i)
            {
                dryCollected.push_back(prod.resamplers.hostBuffer.getSample(0, i));
            }

            juce::AudioBuffer<float> &upOut = prod.resamplers.up.processBlock(prod.resamplers.hostBuffer);
            onnxBuf.makeCopyOf(upOut);
            prod.onnx.processBlock(onnxBuf);
            juce::AudioBuffer<float> &wetOut = prod.resamplers.down.processBlock(onnxBuf);
            for (int i = 0; i < wetOut.getNumSamples(); ++i)
            {
                wetCollected.push_back(wetOut.getSample(0, i));
            }

            prod.resamplers.hostBuffer.clear();
        }

        const int mixLen = std::min(static_cast<int>(wetCollected.size()),
                                    static_cast<int>(dryCollected.size()));
        juce::AudioBuffer<float> dryBuf(1, mixLen);
        juce::AudioBuffer<float> mixBuf(1, mixLen);
        for (int i = 0; i < mixLen; ++i)
        {
            dryBuf.setSample(0, i, dryCollected[static_cast<size_t>(i)]);
            mixBuf.setSample(0, i, wetCollected[static_cast<size_t>(i)]);
        }

        DryWetMixer mixer;
        mixer.prepare(juce::dsp::ProcessSpec{hostSR, static_cast<uint32_t>(mixLen), 1});
        mixer.setWetLatency(totalLatency);
        mixer.setDryWetProportion(0.5f);
        mixer.setDrySamples(dryBuf);
        mixer.setWetSamples(mixBuf);

        const int searchStart = std::max(0, result.expectedPeak - searchHalfWindow);
        const int searchEnd = std::min(mixLen, result.expectedPeak + searchHalfWindow + 1);

        std::vector<float> mixed(mixBuf.getReadPointer(0), mixBuf.getReadPointer(0) + mixLen);
        result.peakPos = scyclone::test::findImpulsePeak(mixed, searchStart, searchEnd);
        if (result.peakPos >= 0 && result.peakPos < mixLen)
        {
            result.peakAmplitude = mixed[static_cast<size_t>(result.peakPos)];
        }
        return result;
    }

    /// |peakPos − expectedPeak| for calibration probes; INT_MAX when peak not found.
    inline int measureDryWetDiracJitter(double hostSR, int hostBlock, int searchHalfWindow)
    {
        const auto measurement = measureDryWetDiracPeak(hostSR, hostBlock, searchHalfWindow);
        if (measurement.peakPos < 0)
        {
            return INT_MAX;
        }
        return std::abs(measurement.peakPos - measurement.expectedPeak);
    }

} // namespace scyclone::test::mixer

namespace resampling_test
{
    using namespace scyclone::test::mixer;
} // namespace resampling_test
