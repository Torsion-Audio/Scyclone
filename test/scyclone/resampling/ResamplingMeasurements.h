#pragma once

/// @file ResamplingMeasurements.h
/// @brief Chain-specific measurement helpers for Scyclone resampling tests.
///
/// Follows the measure/assert split: these functions run stimulus + processing and return
/// scalar metrics or structs. They never call `EXPECT_*` — use
/// ResamplingContractAssertions.h for gtest contracts. Steady input/output capture uses
/// ResamplingChainDriver.h; pre-roll, warmup, and impulse collection use ResamplingRunner.h.
///
/// Calibration probes call `measure*` directly and print suggested tolerances.
///
/// @namespace scyclone::test::resampling

#include <vector>

#include "ImpulseMetrics.h"
#include "ResamplingChainDriver.h"
#include "ResamplingRunner.h"
#include "ResamplingSignalUtils.h"
#include "SignalFidelity.h"
#include "SignalGenerators.h"
#include "SignalMetrics.h"
#include "TestTiming.h"

namespace scyclone::test::resampling
{

    using torsion::test::ImpulseResponse;
    using torsion::test::fillImpulsePeakMetrics;
    using torsion::test::kPreRollBlocks;
    using torsion::test::kSteadyBlocks;
    using torsion::test::latencyPreRollBlocks;
    using torsion::test::sineWarmupBlocks;
    using torsion::test::worstSweptSineRmsError;

    inline float measureRoundTripRmsError(RoundTripChain &chain, double hostSR, int hostBlock,
                                          IProcessor &middle)
    {
        runSilencePreRoll(chain, kPreRollBlocks);

        const int latency = expectedRoundTripPeakSamples(chain, hostSR);
        const int warmupBlocks = sineWarmupBlocks(latency, hostBlock);
        runRoundTripSineWarmup(chain, hostSR, hostBlock, middle, warmupBlocks, 0);

        const int requiredOutput = latency + kSteadyBlocks * hostBlock;
        const auto signals = collectMiddleChainInputOutput(
            chain, middle, hostSR, hostBlock, warmupBlocks, requiredOutput);

        return worstSweptSineRmsError(signals.input, signals.output, hostBlock, latency);
    }

    inline float measureProductionRmsErrorAtLatency(ProductionChain &chain, double hostSR, int hostBlock,
                                                    int latency)
    {
        runProductionSilencePreRoll(chain, latencyPreRollBlocks(latency, hostBlock));

        const int warmupBlocks = sineWarmupBlocks(latency, hostBlock);
        runProductionSineWarmup(chain, hostSR, hostBlock, warmupBlocks, 0);

        const int requiredOutput = latency + kSteadyBlocks * hostBlock;
        const auto signals = collectProductionInputOutput(
            chain, hostSR, hostBlock, warmupBlocks, requiredOutput);

        return worstSweptSineRmsError(signals.input, signals.output, hostBlock, latency);
    }

    inline float measureProductionRmsError(ProductionChain &chain, double hostSR, int hostBlock)
    {
        return measureProductionRmsErrorAtLatency(
            chain, hostSR, hostBlock, productionChainTotalLatency(chain, hostSR));
    }

    inline double measureUpSnrDb(double hostSR, int hostBlock, int passBandPeaks = 1,
                                   SnrCorruptionKind corruption = SnrCorruptionKind::None)
    {
        constexpr int kInputBlocks = 64;
        const int inputLen = hostBlock * kInputBlocks;
        std::vector<float> input(static_cast<size_t>(inputLen));
        const double freq = 0.01111111111;
        torsion::test::genWindowedSines(1, &freq, 1.0, input.data(), inputLen);

        ResamplingProcessor up;
        prepareUpOnly(hostSR, hostBlock, up);
        juce::AudioBuffer<float> buf(1, hostBlock);
        auto output = collectUpOutput(up, buf, input.data(), inputLen, kPreRollBlocks);
        applySnrCorruption(output, corruption);

        const int skip = static_cast<int>(std::round(static_cast<double>(up.getLatencyInSamples()) * up.getSrcRatio()))
                         + up.getOutputBufferSize();
        if (skip + 512 >= static_cast<int>(output.size()))
        {
            return -1.0;
        }
        return torsion::test::calculateSnrDb(output.data() + skip, static_cast<int>(output.size()) - skip, passBandPeaks);
    }

    inline double measureDownSnrDb(double hostSR, int hostBlock, int passBandPeaks = 1,
                                    SnrCorruptionKind corruption = SnrCorruptionKind::None)
    {
        const auto ratioCase = makeProductionRatioCase(hostSR, hostBlock);
        constexpr int kInputBlocks = 64;
        const int onnxBlock = ratioCase.expectedUpOut;
        const int inputLen = onnxBlock * kInputBlocks;
        std::vector<float> input(static_cast<size_t>(inputLen));
        const double freq = 0.01111111111;
        torsion::test::genWindowedSines(1, &freq, 1.0, input.data(), inputLen);

        ResamplingProcessor down;
        prepareDownOnly(hostSR, onnxBlock, hostBlock, down, true);
        juce::AudioBuffer<float> buf(1, onnxBlock);
        auto output = collectDownOutput(down, buf, input.data(), inputLen, kPreRollBlocks);
        applySnrCorruption(output, corruption);

        const int skip = down.getLatencyInSamples() + hostBlock;
        if (skip + 512 >= static_cast<int>(output.size()))
        {
            return -1.0;
        }
        return torsion::test::calculateSnrDb(output.data() + skip, static_cast<int>(output.size()) - skip, passBandPeaks);
    }

    inline ImpulseResponse measureMiddleChainImpulse(RoundTripChain &chain, IProcessor &middle,
                                                     double hostSR, int hostBlock,
                                                     int impulseSample, int preRollBlocks)
    {
        ImpulseResponse response;
        const int latency = expectedMiddleChainPeakSamples(chain, middle, hostSR);
        response.expectedPeak = impulseSample + latency;
        response.hostBlock = hostBlock;

        runMiddleChainSilencePreRoll(chain, middle, preRollBlocks);
        chain.hostBuffer.clear();
        chain.hostBuffer.setSample(0, impulseSample, 1.0f);
        const int collectLen = response.expectedPeak + hostBlock + 64;
        response.samples = collectMiddleChainOutput(chain, middle, collectLen);
        fillImpulsePeakMetrics(response);
        return response;
    }

    inline ImpulseResponse measureMiddleChainImpulse(RoundTripChain &chain, IProcessor &middle,
                                                     double hostSR, int hostBlock,
                                                     int impulseSample = 0)
    {
        const int latency = expectedMiddleChainPeakSamples(chain, middle, hostSR);
        return measureMiddleChainImpulse(
            chain, middle, hostSR, hostBlock, impulseSample,
            latencyPreRollBlocks(latency, hostBlock));
    }

    inline ImpulseResponse measureRoundTripImpulse(RoundTripChain &chain, double hostSR, int hostBlock,
                                                   int impulseSample = 0)
    {
        PassthroughProcessor passthrough;
        return measureMiddleChainImpulse(
            chain, passthrough, hostSR, hostBlock, impulseSample, kPreRollBlocks);
    }

    inline ImpulseResponse measureProductionImpulse(ProductionChain &chain, double hostSR, int hostBlock,
                                                    int impulseSample = 0)
    {
        return measureMiddleChainImpulse(chain.resamplers, chain.onnx, hostSR, hostBlock, impulseSample);
    }

} // namespace scyclone::test::resampling
