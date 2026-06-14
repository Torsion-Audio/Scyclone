#pragma once

// Measurement helpers for resampling behavioral tests and calibration probes.
// Include after ResamplingChainHelpers.h.

#include <limits>
#include <vector>
#include "ResamplingChainHelpers.h"
#include "dsp/mixer/DryWetMixer.h"

namespace resampling_test {

constexpr int kSineWarmupMarginBlocks = 2;

inline int sineWarmupBlocks(int latencySamples, int hostBlock) {
    return (latencySamples + hostBlock - 1) / hostBlock + kSineWarmupMarginBlocks;
}

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

inline float measureProductionRmsError(ProductionChain& chain, double hostSR, int hostBlock) {
    return measureProductionRmsErrorAtLatency(
        chain, hostSR, hostBlock, productionChainTotalLatency(chain, hostSR));
}

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

struct DryWetDiracMeasurement {
    int peakPos = -1;
    int expectedPeak = 0;
    float peakAmplitude = 0.0f;
};

inline DryWetDiracMeasurement measureDryWetDiracPeak(double hostSR, int hostBlock,
                                                       int searchHalfWindow) {
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
    while (static_cast<int>(wetCollected.size()) < collectSamples) {
        for (int i = 0; i < prod.resamplers.hostBuffer.getNumSamples(); ++i) {
            dryCollected.push_back(prod.resamplers.hostBuffer.getSample(0, i));
        }

        juce::AudioBuffer<float>& upOut = prod.resamplers.up.processBlock(prod.resamplers.hostBuffer);
        onnxBuf.makeCopyOf(upOut);
        prod.onnx.processBlock(onnxBuf);
        juce::AudioBuffer<float>& wetOut = prod.resamplers.down.processBlock(onnxBuf);
        for (int i = 0; i < wetOut.getNumSamples(); ++i) {
            wetCollected.push_back(wetOut.getSample(0, i));
        }

        prod.resamplers.hostBuffer.clear();
    }

    const int mixLen = std::min(static_cast<int>(wetCollected.size()),
                                static_cast<int>(dryCollected.size()));
    juce::AudioBuffer<float> dryBuf(1, mixLen);
    juce::AudioBuffer<float> mixBuf(1, mixLen);
    for (int i = 0; i < mixLen; ++i) {
        dryBuf.setSample(0, i, dryCollected[static_cast<size_t>(i)]);
        mixBuf.setSample(0, i, wetCollected[static_cast<size_t>(i)]);
    }

    DryWetMixer mixer;
    mixer.prepare(juce::dsp::ProcessSpec{ hostSR, static_cast<uint32_t>(mixLen), 1 });
    mixer.setWetLatency(totalLatency);
    mixer.setDryWetProportion(0.5f);
    mixer.setDrySamples(dryBuf);
    mixer.setWetSamples(mixBuf);

    const int searchStart = std::max(0, result.expectedPeak - searchHalfWindow);
    const int searchEnd = std::min(mixLen, result.expectedPeak + searchHalfWindow + 1);

    std::vector<float> mixed(mixBuf.getReadPointer(0), mixBuf.getReadPointer(0) + mixLen);
    result.peakPos = findImpulsePeak(mixed, searchStart, searchEnd);
    if (result.peakPos >= 0 && result.peakPos < mixLen) {
        result.peakAmplitude = mixed[static_cast<size_t>(result.peakPos)];
    }
    return result;
}

inline int measureDryWetDiracJitter(double hostSR, int hostBlock, int searchHalfWindow) {
    const auto measurement = measureDryWetDiracPeak(hostSR, hostBlock, searchHalfWindow);
    if (measurement.peakPos < 0) {
        return std::numeric_limits<int>::max();
    }
    return std::abs(measurement.peakPos - measurement.expectedPeak);
}

constexpr int kImpulsePeakWindowHalfWidth = 5;
constexpr int kImpulseWideSearchHalfWidth = 128;
constexpr int kImpulsePeakToleranceSamples = 5;

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

inline ImpulseResponse measureMiddleChainImpulse(RoundTripChain& chain, IProcessor& middle,
                                                 double hostSR, int hostBlock,
                                                 int impulseSample = 0) {
    const int expectedPeak = expectedMiddleChainPeakSamples(chain, middle, hostSR);
    return measureMiddleChainImpulse(
        chain, middle, hostSR, hostBlock, impulseSample,
        productionPreRollBlocks(expectedPeak, hostBlock));
}

inline ImpulseResponse measureRoundTripImpulse(RoundTripChain& chain, double hostSR, int hostBlock,
                                               int impulseSample = 0) {
    PassthroughProcessor passthrough;
    return measureMiddleChainImpulse(
        chain, passthrough, hostSR, hostBlock, impulseSample, kPreRollBlocks);
}

inline ImpulseResponse measureProductionImpulse(ProductionChain& chain, double hostSR, int hostBlock,
                                                int impulseSample = 0) {
    return measureMiddleChainImpulse(chain.resamplers, chain.onnx, hostSR, hostBlock, impulseSample);
}

} // namespace resampling_test
