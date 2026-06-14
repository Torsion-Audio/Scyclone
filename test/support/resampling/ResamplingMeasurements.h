#pragma once

/// @file ResamplingMeasurements.h
/// @brief Pure measurement helpers for resampling behavioral tests and calibration probes.
///
/// Follows the measure/assert split: these functions run stimulus + processing and return
/// scalar metrics or structs. They never call `EXPECT_*` — use
/// ResamplingContractAssertions.h for gtest contracts.
///
/// Calibration probes call `measure*` directly and print suggested tolerances.
///
/// @namespace resampling_test (compatibility alias for scyclone::test::resampling)

#include <vector>
#include "ResamplingRunner.h"
#include "SignalGenerators.h"
#include "SignalMetrics.h"
#include "TestInfrastructure.h"

namespace resampling_test {

/// Extra blocks beyond latency-aligned warmup before RMS steady window.
constexpr int kSineWarmupMarginBlocks = 2;

/// Block count to flush reported latency before swept-sine RMS capture.
inline int sineWarmupBlocks(int latencySamples, int hostBlock) {
    return (latencySamples + hostBlock - 1) / hostBlock + kSineWarmupMarginBlocks;
}

/// Worst normalized RMS error over @p nBlocks aligned at @p latency (output vs input).
inline float worstSweptSineRmsError(const std::vector<float>& inputSignal,
                                    const std::vector<float>& outputSignal,
                                    int hostBlock, int latency, int nBlocks = kSteadyBlocks) {
    float worst = 0.0f;
    for (int block = 0; block < nBlocks; ++block) {
        const int inStart = block * hostBlock;
        const int outStart = inStart + latency;
        if (outStart + hostBlock > static_cast<int>(outputSignal.size())) {
            break;
        }
        const float inRms = rms(inputSignal, inStart, hostBlock);
        if (inRms <= 1.0e-6f) {
            continue;
        }
        std::vector<float> diff(static_cast<size_t>(hostBlock));
        for (int i = 0; i < hostBlock; ++i) {
            diff[static_cast<size_t>(i)] =
                outputSignal[static_cast<size_t>(outStart + i)] - inputSignal[static_cast<size_t>(inStart + i)];
        }
        worst = std::max(worst, rms(diff.data(), hostBlock) / inRms);
    }
    return worst;
}

/// Full round-trip swept-sine RMS error (pre-roll, warmup, steady capture).
inline float measureRoundTripRmsError(RoundTripChain& chain, double hostSR, int hostBlock,
                                      IProcessor& middle) {
    runSilencePreRoll(chain, kPreRollBlocks);

    const int latency = expectedRoundTripPeakSamples(chain, hostSR);
    const int warmupBlocks = sineWarmupBlocks(latency, hostBlock);
    runRoundTripSineWarmup(chain, hostSR, hostBlock, middle, warmupBlocks, 0);

    const int requiredOutput = latency + kSteadyBlocks * hostBlock;
    std::vector<float> inputSignal;
    std::vector<float> outputSignal;

    int blockIndex = warmupBlocks;
    while (static_cast<int>(outputSignal.size()) < requiredOutput) {
        for (int i = 0; i < hostBlock; ++i) {
            chain.hostBuffer.setSample(0, i, generateSweptSine(hostSR, hostBlock, blockIndex, i));
        }
        for (int i = 0; i < hostBlock; ++i) {
            inputSignal.push_back(chain.hostBuffer.getSample(0, i));
        }
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        middle.processBlock(upOut);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        for (int i = 0; i < downOut.getNumSamples(); ++i) {
            outputSignal.push_back(downOut.getSample(0, i));
        }
        ++blockIndex;
    }

    return worstSweptSineRmsError(inputSignal, outputSignal, hostBlock, latency);
}

/// Production-chain RMS at an explicit @p latency (for calibration lag sweeps).
inline float measureProductionRmsErrorAtLatency(ProductionChain& chain, double hostSR, int hostBlock,
                                                int latency) {
    runProductionSilencePreRoll(chain, productionPreRollBlocks(latency, hostBlock));

    const int warmupBlocks = sineWarmupBlocks(latency, hostBlock);
    runProductionSineWarmup(chain, hostSR, hostBlock, warmupBlocks, 0);

    const int requiredOutput = latency + kSteadyBlocks * hostBlock;
    std::vector<float> inputSignal;
    std::vector<float> outputSignal;
    juce::AudioBuffer<float> onnxBuf;

    int blockIndex = warmupBlocks;
    while (static_cast<int>(outputSignal.size()) < requiredOutput) {
        for (int i = 0; i < hostBlock; ++i) {
            chain.resamplers.hostBuffer.setSample(
                0, i, generateSweptSine(hostSR, hostBlock, blockIndex, i));
        }
        for (int i = 0; i < hostBlock; ++i) {
            inputSignal.push_back(chain.resamplers.hostBuffer.getSample(0, i));
        }
        processProductionBlock(chain, onnxBuf, outputSignal);
        ++blockIndex;
    }

    return worstSweptSineRmsError(inputSignal, outputSignal, hostBlock, latency);
}

/// Production-chain RMS using `productionChainTotalLatency`.
inline float measureProductionRmsError(ProductionChain& chain, double hostSR, int hostBlock) {
    return measureProductionRmsErrorAtLatency(
        chain, hostSR, hostBlock, productionChainTotalLatency(chain, hostSR));
}

/// FFT peak SNR (dB) for up-only windowed sine. Returns -1.0 on failure.
inline double measureUpSnrDb(double hostSR, int hostBlock, int passBandPeaks = 1) {
    constexpr int kInputBlocks = 64;
    const int inputLen = hostBlock * kInputBlocks;
    std::vector<float> input(static_cast<size_t>(inputLen));
    const double freq = 0.01111111111;
    genWindowedSines(1, &freq, 1.0, input.data(), inputLen);

    ResamplingProcessor up;
    prepareUpOnly(hostSR, hostBlock, up);
    juce::AudioBuffer<float> buf(1, hostBlock);
    const auto output = collectUpOutput(up, buf, input.data(), inputLen, kPreRollBlocks);

    const int skip = static_cast<int>(std::round(static_cast<double>(up.getLatencyInSamples()) * up.getSrcRatio()))
                     + up.getOutputBufferSize();
    if (skip + 512 >= static_cast<int>(output.size())) {
        return -1.0;
    }
    return calculateSnrDb(output.data() + skip, static_cast<int>(output.size()) - skip, passBandPeaks);
}

/// FFT peak SNR (dB) for down-only windowed sine (production coupled sizing).
inline double measureDownSnrDb(double hostSR, int hostBlock, int passBandPeaks = 1) {
    const auto ratioCase = makeProductionRatioCase(hostSR, hostBlock);
    constexpr int kInputBlocks = 64;
    const int onnxBlock = ratioCase.expectedUpOut;
    const int inputLen = onnxBlock * kInputBlocks;
    std::vector<float> input(static_cast<size_t>(inputLen));
    const double freq = 0.01111111111;
    genWindowedSines(1, &freq, 1.0, input.data(), inputLen);

    ResamplingProcessor down;
    prepareDownOnly(hostSR, onnxBlock, hostBlock, down, true);
    juce::AudioBuffer<float> buf(1, onnxBlock);
    const auto output = collectDownOutput(down, buf, input.data(), inputLen, kPreRollBlocks);

    const int skip = down.getLatencyInSamples() + hostBlock;
    if (skip + 512 >= static_cast<int>(output.size())) {
        return -1.0;
    }
    return calculateSnrDb(output.data() + skip, static_cast<int>(output.size()) - skip, passBandPeaks);
}

constexpr int kImpulsePeakWindowHalfWidth = 5;
constexpr int kImpulseWideSearchHalfWidth = 128;
constexpr int kImpulsePeakToleranceSamples = 5;

/// Captured impulse samples plus narrow/wide peak search metrics.
struct ImpulseResponse {
    std::vector<float> samples;
    int expectedPeak = 0;
    int hostBlock = 0;
    int narrowPeakPos = -1;
    float narrowPeakVal = 0.f;
    int widePeakPos = -1;
    float widePeakVal = 0.f;
    float maxOutsideMainWindow = 0.f;
};

/// Populates narrow/wide peak fields on @p response from @p response.samples.
inline void fillImpulsePeakMetrics(ImpulseResponse& response) {
    const int expected = response.expectedPeak;
    const int hostBlock = response.hostBlock;
    const auto& collected = response.samples;
    if (collected.empty()) {
        return;
    }

    const int wideStart = std::max(0, expected - kImpulseWideSearchHalfWidth);
    const int wideEnd = std::min(static_cast<int>(collected.size()),
                                 expected + hostBlock + kImpulseWideSearchHalfWidth);
    response.widePeakPos = findImpulsePeak(collected, wideStart, wideEnd);
    if (response.widePeakPos >= 0) {
        response.widePeakVal = std::abs(collected[static_cast<size_t>(response.widePeakPos)]);
    }

    const int narrowStart = std::max(0, expected - kImpulsePeakWindowHalfWidth);
    const int narrowEnd = expected + hostBlock;
    response.narrowPeakPos = findImpulsePeak(collected, narrowStart, narrowEnd);
    if (response.narrowPeakPos >= 0) {
        response.narrowPeakVal = std::abs(collected[static_cast<size_t>(response.narrowPeakPos)]);
        const int windowStart = std::max(0, response.narrowPeakPos - kImpulsePeakWindowHalfWidth);
        const int windowEnd = std::min(static_cast<int>(collected.size()),
                                       response.narrowPeakPos + hostBlock);
        response.maxOutsideMainWindow = maxAbsOutsideWindow(collected, windowStart, windowEnd);
    }
}

/// Impulse at @p impulseSample through middle chain with explicit pre-roll.
inline ImpulseResponse measureMiddleChainImpulse(RoundTripChain& chain, IProcessor& middle,
                                                 double hostSR, int hostBlock,
                                                 int impulseSample, int preRollBlocks) {
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

/// Impulse with production-derived pre-roll block count.
inline ImpulseResponse measureMiddleChainImpulse(RoundTripChain& chain, IProcessor& middle,
                                                 double hostSR, int hostBlock,
                                                 int impulseSample = 0) {
    const int expectedPeak = expectedMiddleChainPeakSamples(chain, middle, hostSR);
    return measureMiddleChainImpulse(
        chain, middle, hostSR, hostBlock, impulseSample,
        productionPreRollBlocks(expectedPeak, hostBlock));
}

/// Round-trip impulse (passthrough middle, default pre-roll).
inline ImpulseResponse measureRoundTripImpulse(RoundTripChain& chain, double hostSR, int hostBlock,
                                               int impulseSample = 0) {
    PassthroughProcessor passthrough;
    return measureMiddleChainImpulse(
        chain, passthrough, hostSR, hostBlock, impulseSample, kPreRollBlocks);
}

/// Production-chain impulse (SimulatedOnnx middle).
inline ImpulseResponse measureProductionImpulse(ProductionChain& chain, double hostSR, int hostBlock,
                                                int impulseSample = 0) {
    return measureMiddleChainImpulse(chain.resamplers, chain.onnx, hostSR, hostBlock, impulseSample);
}

} // namespace resampling_test
