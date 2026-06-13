#pragma once

#include <cmath>
#include <gtest/gtest.h>
#include <vector>
#include "dsp/resampler/ResamplingProcessor.h"
#include "dsp/IProcessor.h"

namespace resampling_test {

constexpr double kOnnxRate = 48000.0;
constexpr int kMinOnnxBlock = 32;

inline int upOutputBlockSize(double hostSR, int hostBlock) {
    return static_cast<int>(std::ceil(kOnnxRate / hostSR * static_cast<double>(hostBlock)));
}

inline int downOutputBlockSizeUncoupled(double hostSR, int upBlock) {
    return static_cast<int>(std::ceil(hostSR / kOnnxRate * static_cast<double>(upBlock)));
}

inline bool isFeasibleHostConfig(double hostSR, int hostBlock) {
    return upOutputBlockSize(hostSR, hostBlock) >= kMinOnnxBlock;
}

/** Mirror streaming_test.c lines 128-131 tolerance. */
inline int longRunTolerance(double ratio) {
    const int terminate = static_cast<int>(std::ceil((ratio >= 1.0) ? ratio : 1.0 / ratio));
    return 2 * terminate;
}

struct RoundTripChain {
    ResamplingProcessor up;
    ResamplingProcessor down;
    juce::AudioBuffer<float> hostBuffer;
    int upOutSize = 0;
};

inline RoundTripChain prepareRoundTripChain(double hostSR, int hostBlock, bool forceDownToHost = true) {
    RoundTripChain chain;
    juce::dsp::ProcessSpec monoSpec{ hostSR, static_cast<uint32_t>(hostBlock), 1 };
    chain.upOutSize = chain.up.prepare(monoSpec, kOnnxRate, "up-test");
    juce::dsp::ProcessSpec onnxSpec{ kOnnxRate, static_cast<uint32_t>(chain.upOutSize), 1 };
    if (forceDownToHost) {
        chain.down.prepare(onnxSpec, hostSR, "down-test", hostBlock);
    } else {
        chain.down.prepare(onnxSpec, hostSR, "down-test");
    }
    chain.hostBuffer.setSize(1, hostBlock);
    chain.hostBuffer.clear();
    return chain;
}

inline int prepareUpOnly(double hostSR, int hostBlock, ResamplingProcessor& up) {
    juce::dsp::ProcessSpec monoSpec{ hostSR, static_cast<uint32_t>(hostBlock), 1 };
    return up.prepare(monoSpec, kOnnxRate, "up-only");
}

inline int prepareDownOnly(double hostSR, int upBlock, int hostBlock, ResamplingProcessor& down,
                           bool forceDownToHost = false) {
    juce::dsp::ProcessSpec onnxSpec{ kOnnxRate, static_cast<uint32_t>(upBlock), 1 };
    if (forceDownToHost) {
        return down.prepare(onnxSpec, hostSR, "down-only", hostBlock);
    }
    return down.prepare(onnxSpec, hostSR, "down-only");
}

inline void runSilencePreRoll(RoundTripChain& chain, int nBlocks) {
    chain.hostBuffer.clear();
    for (int n = 0; n < nBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        (void) chain.down.processBlock(upOut);
    }
}

inline void runSilencePreRoll(ResamplingProcessor& resampler, juce::AudioBuffer<float>& buf, int nBlocks) {
    buf.clear();
    for (int n = 0; n < nBlocks; ++n) {
        (void) resampler.processBlock(buf);
    }
}

inline void assertWarmupAllowsPartialOutput(const ResamplingProcessor& resampler) {
    const long gen = resampler.getLastOutputFramesGenerated();
    const int outSize = resampler.getOutputBufferSize();
    EXPECT_GT(outSize, 0);
    EXPECT_LE(gen, static_cast<long>(outSize));
}

inline void assertSteadyStateFullOutput(const ResamplingProcessor& resampler) {
    EXPECT_EQ(resampler.getLastOutputFramesGenerated(), static_cast<long>(resampler.getOutputBufferSize()))
        << "steady state: output_frames_gen must fill output buffer";
    EXPECT_EQ(resampler.getLastInputFramesUsed(), static_cast<long>(resampler.getInputBufferSize()))
        << "steady state: all input frames consumed";
}

struct BlockAccounting {
    long totalIn = 0;
    long totalOut = 0;
};

inline BlockAccounting accumulateRoundTripPerBlock(RoundTripChain& chain, int nBlocks) {
    BlockAccounting acc;
    chain.hostBuffer.clear();
    for (int n = 0; n < nBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        acc.totalIn += chain.up.getLastInputFramesUsed();
        acc.totalOut += chain.up.getLastOutputFramesGenerated();
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        acc.totalIn += chain.down.getLastInputFramesUsed();
        acc.totalOut += chain.down.getLastOutputFramesGenerated();
        juce::ignoreUnused(downOut);
    }
    return acc;
}

inline int findImpulsePeak(const std::vector<float>& collected, int searchStart, int searchEnd) {
    int peakPos = -1;
    float peakVal = 0.0f;
    searchStart = std::max(0, searchStart);
    searchEnd = std::min(static_cast<int>(collected.size()), searchEnd);
    for (int i = searchStart; i < searchEnd; ++i) {
        const float v = std::abs(collected[static_cast<size_t>(i)]);
        if (v > peakVal) {
            peakVal = v;
            peakPos = i;
        }
    }
    return peakPos;
}

class PassthroughProcessor : public IProcessor {
public:
    void processBlock(juce::AudioBuffer<float>& buffer) override { juce::ignoreUnused(buffer); }
};

inline void runPassthroughChain(RoundTripChain& chain, IProcessor& passthrough, int nBlocks) {
    chain.hostBuffer.clear();
    for (int n = 0; n < nBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        passthrough.processBlock(upOut);
        (void) chain.down.processBlock(upOut);
    }
}

} // namespace resampling_test
