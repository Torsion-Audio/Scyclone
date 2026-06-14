#pragma once

/// @file DryWetContract.h
/// @brief Dry/wet dirac alignment fixture, measurements, and gtest contracts.
///
/// Follows the measure/assert split: `measureDryWetDirac*` run production chain +
/// DryWetMixer and return peak position/amplitude; `assertDryWetDiracAligned` applies
/// gtest expectations. Host capture uses ResamplingChainDriver.h.
///
/// @namespace scyclone::test::mixer

#include <climits>
#include <vector>
#include <gtest/gtest.h>
#include <JuceHeader.h>

#include "HostConfigFixtures.h"
#include "ResamplingChainDriver.h"
#include "SignalMetrics.h"
#include "TestTiming.h"
#include "dsp/mixer/DryWetMixer.h"

namespace scyclone::test::mixer
{

    class DryWetHostConfigTest : public torsion::test::HostConfigParamTest
    {
    };

    /// CI dirac peak search half-width (samples).
    constexpr int kDiracAlignmentTolerance = 2;
    /// Wider search for calibration jitter probes.
    constexpr int kCalibrationDiracSearchHalfWindow = 20;

    /// Result of dirac stimulus through production chain + DryWetMixer.
    struct DryWetDiracMeasurement
    {
        int peakPos = -1;
        int expectedPeak = 0;
        float peakAmplitude = 0.0f;
    };

    /// Dirac at block center (dry) + full chain (wet) → search mixed peak.
    inline DryWetDiracMeasurement measureDryWetDiracPeak(double hostSR, int hostBlock,
                                                         int searchHalfWindow)
    {
        using namespace scyclone::test::resampling;

        DryWetDiracMeasurement result;
        auto prod = prepareProductionChain(hostSR, hostBlock);
        const int totalLatency = productionChainTotalLatency(prod, hostSR);
        const int diracPos = hostBlock / 2;
        result.expectedPeak = diracPos + totalLatency;

        runProductionSilencePreRoll(prod, torsion::test::latencyPreRollBlocks(totalLatency, hostBlock));

        const int collectSamples = totalLatency + hostBlock + diracPos;
        prod.resamplers.hostBuffer.setSample(0, diracPos, 1.0f);

        const auto captured = collectProductionHostAndOutput(prod, collectSamples);
        const auto &dryCollected = captured.host;
        const auto &wetCollected = captured.chainOutput;

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
        result.peakPos = torsion::test::findImpulsePeak(mixed, searchStart, searchEnd);
        if (result.peakPos >= 0 && result.peakPos < mixLen)
        {
            result.peakAmplitude = mixed[static_cast<size_t>(result.peakPos)];
        }
        return result;
    }

    inline int measureDryWetDiracJitter(double hostSR, int hostBlock, int searchHalfWindow)
    {
        const auto measurement = measureDryWetDiracPeak(hostSR, hostBlock, searchHalfWindow);
        if (measurement.peakPos < 0)
        {
            return INT_MAX;
        }
        return std::abs(measurement.peakPos - measurement.expectedPeak);
    }

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
