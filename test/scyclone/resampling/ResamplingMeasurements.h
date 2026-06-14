#pragma once

/// @file ResamplingMeasurements.h
/// @brief Chain-specific measurement helpers for Scyclone resampling tests.
///
/// Follows the measure/assert split: these functions run stimulus + processing and return
/// scalar metrics or structs. They never call `EXPECT_*` — use
/// ResamplingContractAssertions.h for gtest contracts.
///
/// Calibration probes call `measure*` directly and print suggested tolerances.
///
/// @namespace scyclone::test::resampling

#include <vector>

#include "ImpulseMetrics.h"
#include "ResamplingRunner.h"
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

    /// Full round-trip swept-sine RMS error (pre-roll, warmup, steady capture).
    inline float measureRoundTripRmsError(RoundTripChain &chain, double hostSR, int hostBlock,
                                          IProcessor &middle)
    {
        runSilencePreRoll(chain, kPreRollBlocks);

        const int latency = expectedRoundTripPeakSamples(chain, hostSR);
        const int warmupBlocks = sineWarmupBlocks(latency, hostBlock);
        runRoundTripSineWarmup(chain, hostSR, hostBlock, middle, warmupBlocks, 0);

        const int requiredOutput = latency + kSteadyBlocks * hostBlock;
        std::vector<float> inputSignal;
        std::vector<float> outputSignal;

        int blockIndex = warmupBlocks;
        while (static_cast<int>(outputSignal.size()) < requiredOutput)
        {
            for (int i = 0; i < hostBlock; ++i)
            {
                chain.hostBuffer.setSample(0, i, torsion::test::generateSweptSine(hostSR, hostBlock, blockIndex, i));
            }
            for (int i = 0; i < hostBlock; ++i)
            {
                inputSignal.push_back(chain.hostBuffer.getSample(0, i));
            }
            juce::AudioBuffer<float> &upOut = chain.up.processBlock(chain.hostBuffer);
            middle.processBlock(upOut);
            juce::AudioBuffer<float> &downOut = chain.down.processBlock(upOut);
            for (int i = 0; i < downOut.getNumSamples(); ++i)
            {
                outputSignal.push_back(downOut.getSample(0, i));
            }
            ++blockIndex;
        }

        return worstSweptSineRmsError(inputSignal, outputSignal, hostBlock, latency);
    }

    /// Production-chain RMS at an explicit @p latency (for calibration lag sweeps).
    inline float measureProductionRmsErrorAtLatency(ProductionChain &chain, double hostSR, int hostBlock,
                                                    int latency)
    {
        runProductionSilencePreRoll(chain, latencyPreRollBlocks(latency, hostBlock));

        const int warmupBlocks = sineWarmupBlocks(latency, hostBlock);
        runProductionSineWarmup(chain, hostSR, hostBlock, warmupBlocks, 0);

        const int requiredOutput = latency + kSteadyBlocks * hostBlock;
        std::vector<float> inputSignal;
        std::vector<float> outputSignal;
        juce::AudioBuffer<float> onnxBuf;

        int blockIndex = warmupBlocks;
        while (static_cast<int>(outputSignal.size()) < requiredOutput)
        {
            for (int i = 0; i < hostBlock; ++i)
            {
                chain.resamplers.hostBuffer.setSample(
                    0, i, torsion::test::generateSweptSine(hostSR, hostBlock, blockIndex, i));
            }
            for (int i = 0; i < hostBlock; ++i)
            {
                inputSignal.push_back(chain.resamplers.hostBuffer.getSample(0, i));
            }
            processProductionBlock(chain, onnxBuf, outputSignal);
            ++blockIndex;
        }

        return worstSweptSineRmsError(inputSignal, outputSignal, hostBlock, latency);
    }

    /// Production-chain RMS using `productionChainTotalLatency`.
    inline float measureProductionRmsError(ProductionChain &chain, double hostSR, int hostBlock)
    {
        return measureProductionRmsErrorAtLatency(
            chain, hostSR, hostBlock, productionChainTotalLatency(chain, hostSR));
    }

    /// FFT peak SNR (dB) for up-only windowed sine. Returns -1.0 on failure.
    inline double measureUpSnrDb(double hostSR, int hostBlock, int passBandPeaks = 1)
    {
        constexpr int kInputBlocks = 64;
        const int inputLen = hostBlock * kInputBlocks;
        std::vector<float> input(static_cast<size_t>(inputLen));
        const double freq = 0.01111111111;
        torsion::test::genWindowedSines(1, &freq, 1.0, input.data(), inputLen);

        ResamplingProcessor up;
        prepareUpOnly(hostSR, hostBlock, up);
        juce::AudioBuffer<float> buf(1, hostBlock);
        const auto output = collectUpOutput(up, buf, input.data(), inputLen, kPreRollBlocks);

        const int skip = static_cast<int>(std::round(static_cast<double>(up.getLatencyInSamples()) * up.getSrcRatio()))
                         + up.getOutputBufferSize();
        if (skip + 512 >= static_cast<int>(output.size()))
        {
            return -1.0;
        }
        return torsion::test::calculateSnrDb(output.data() + skip, static_cast<int>(output.size()) - skip, passBandPeaks);
    }

    /// FFT peak SNR (dB) for down-only windowed sine (production coupled sizing).
    inline double measureDownSnrDb(double hostSR, int hostBlock, int passBandPeaks = 1)
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
        const auto output = collectDownOutput(down, buf, input.data(), inputLen, kPreRollBlocks);

        const int skip = down.getLatencyInSamples() + hostBlock;
        if (skip + 512 >= static_cast<int>(output.size()))
        {
            return -1.0;
        }
        return torsion::test::calculateSnrDb(output.data() + skip, static_cast<int>(output.size()) - skip, passBandPeaks);
    }

    /// Impulse at @p impulseSample through middle chain with explicit pre-roll.
    inline ImpulseResponse measureMiddleChainImpulse(RoundTripChain &chain, IProcessor &middle,
                                                     double hostSR, int hostBlock,
                                                     int impulseSample, int preRollBlocks)
    {
        ImpulseResponse response;
        response.expectedPeak = expectedMiddleChainPeakSamples(chain, middle, hostSR);
        response.hostBlock = hostBlock;

        runMiddleChainSilencePreRoll(chain, middle, preRollBlocks);
        chain.hostBuffer.clear();
        chain.hostBuffer.setSample(0, impulseSample, 1.0f);
        const int collectLen = response.expectedPeak + hostBlock + 64;
        response.samples = collectMiddleChainOutput(chain, middle, collectLen);
        fillImpulsePeakMetrics(response);
        return response;
    }

    /// Impulse with latency-derived pre-roll block count.
    inline ImpulseResponse measureMiddleChainImpulse(RoundTripChain &chain, IProcessor &middle,
                                                     double hostSR, int hostBlock,
                                                     int impulseSample = 0)
    {
        const int expectedPeak = expectedMiddleChainPeakSamples(chain, middle, hostSR);
        return measureMiddleChainImpulse(
            chain, middle, hostSR, hostBlock, impulseSample,
            latencyPreRollBlocks(expectedPeak, hostBlock));
    }

    /// Round-trip impulse (passthrough middle, default pre-roll).
    inline ImpulseResponse measureRoundTripImpulse(RoundTripChain &chain, double hostSR, int hostBlock,
                                                   int impulseSample = 0)
    {
        PassthroughProcessor passthrough;
        return measureMiddleChainImpulse(
            chain, passthrough, hostSR, hostBlock, impulseSample, kPreRollBlocks);
    }

    /// Production-chain impulse (SimulatedOnnx middle).
    inline ImpulseResponse measureProductionImpulse(ProductionChain &chain, double hostSR, int hostBlock,
                                                    int impulseSample = 0)
    {
        return measureMiddleChainImpulse(chain.resamplers, chain.onnx, hostSR, hostBlock, impulseSample);
    }

} // namespace scyclone::test::resampling
