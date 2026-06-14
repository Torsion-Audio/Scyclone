#pragma once

// Resampler chain builders and single-processor streaming helpers.
// Include after ResamplingSignalUtils.h and processor mocks.

#include <vector>
#include <JuceHeader.h>
#include "ResamplingSignalUtils.h"
#include "PassthroughProcessor.h"
#include "SimulatedOnnxProcessor.h"
#include "dsp/resampler/ResamplingProcessor.h"
#include "dsp/IProcessor.h"
#include "dsp/utils/utils.h"

namespace resampling_test {

struct RoundTripChain {
    ResamplingProcessor up;
    ResamplingProcessor down;
    juce::AudioBuffer<float> hostBuffer;
    int upOutSize = 0;
};

struct ProductionChain {
    RoundTripChain resamplers;
    SimulatedOnnxProcessor onnx;
};

inline void initRoundTripChain(RoundTripChain& chain, double hostSR, int hostBlock,
                               bool forceDownToHost = true) {
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
}

inline RoundTripChain prepareRoundTripChain(double hostSR, int hostBlock, bool forceDownToHost = true) {
    RoundTripChain chain;
    initRoundTripChain(chain, hostSR, hostBlock, forceDownToHost);
    return chain;
}

inline void initProductionChain(ProductionChain& chain, double hostSR, int hostBlock) {
    initRoundTripChain(chain.resamplers, hostSR, hostBlock);
    juce::dsp::ProcessSpec onnxSpec{
        kOnnxRate, static_cast<uint32_t>(chain.resamplers.upOutSize), 1 };
    chain.onnx.prepare(onnxSpec);
}

inline ProductionChain prepareProductionChain(double hostSR, int hostBlock) {
    ProductionChain chain;
    initProductionChain(chain, hostSR, hostBlock);
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

inline int productionPreRollBlocks(int totalLatencyHost, int hostBlock) {
    return (totalLatencyHost + hostBlock - 1) / hostBlock + kPreRollBlocks;
}

inline void processProductionBlock(ProductionChain& chain, juce::AudioBuffer<float>& onnxBuf,
                                   std::vector<float>& outputSignal) {
    juce::AudioBuffer<float>& upOut = chain.resamplers.up.processBlock(chain.resamplers.hostBuffer);
    onnxBuf.setSize(1, upOut.getNumSamples(), false, false, true);
    onnxBuf.copyFrom(0, 0, upOut, 0, 0, upOut.getNumSamples());
    chain.onnx.processBlock(onnxBuf);
    juce::AudioBuffer<float>& downOut = chain.resamplers.down.processBlock(onnxBuf);
    for (int i = 0; i < downOut.getNumSamples(); ++i) {
        outputSignal.push_back(downOut.getSample(0, i));
    }
}

inline void processProductionBlockDiscardOutput(ProductionChain& chain, juce::AudioBuffer<float>& onnxBuf) {
    juce::AudioBuffer<float>& upOut = chain.resamplers.up.processBlock(chain.resamplers.hostBuffer);
    onnxBuf.setSize(1, upOut.getNumSamples(), false, false, true);
    onnxBuf.copyFrom(0, 0, upOut, 0, 0, upOut.getNumSamples());
    chain.onnx.processBlock(onnxBuf);
    (void) chain.resamplers.down.processBlock(onnxBuf);
}

inline void runRoundTripSineWarmup(RoundTripChain& chain, double hostSR, int hostBlock,
                                   IProcessor& middle, int nBlocks, int blockIndexStart) {
    for (int block = 0; block < nBlocks; ++block) {
        const int blockIndex = blockIndexStart + block;
        for (int i = 0; i < hostBlock; ++i) {
            chain.hostBuffer.setSample(0, i, generateSweptSine(hostSR, hostBlock, blockIndex, i));
        }
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        middle.processBlock(upOut);
        (void) chain.down.processBlock(upOut);
    }
}

inline void runProductionSineWarmup(ProductionChain& chain, double hostSR, int hostBlock,
                                    int nBlocks, int blockIndexStart) {
    juce::AudioBuffer<float> onnxBuf;
    for (int block = 0; block < nBlocks; ++block) {
        const int blockIndex = blockIndexStart + block;
        for (int i = 0; i < hostBlock; ++i) {
            chain.resamplers.hostBuffer.setSample(
                0, i, generateSweptSine(hostSR, hostBlock, blockIndex, i));
        }
        processProductionBlockDiscardOutput(chain, onnxBuf);
    }
}

inline void runProductionSilencePreRoll(ProductionChain& chain, int nBlocks) {
    chain.resamplers.hostBuffer.clear();
    juce::AudioBuffer<float> onnxBuf;
    for (int n = 0; n < nBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.resamplers.up.processBlock(chain.resamplers.hostBuffer);
        if (upOut.getNumChannels() < 1 || upOut.getNumSamples() < 1) {
            return;
        }
        onnxBuf.setSize(1, upOut.getNumSamples(), false, false, true);
        onnxBuf.copyFrom(0, 0, upOut, 0, 0, upOut.getNumSamples());
        chain.onnx.processBlock(onnxBuf);
        (void) chain.resamplers.down.processBlock(onnxBuf);
    }
}

inline int expectedMiddleChainPeakSamples(const RoundTripChain& chain, const IProcessor& middle,
                                          double hostSR) {
    return utils::computeTotalLatencyInSamples(
        chain.up.getLatencyInSamples(),
        middle.getLatencyInSamples(),
        chain.down.getLatencyInSamples(),
        kOnnxRate,
        hostSR);
}

inline int expectedRoundTripPeakSamples(const RoundTripChain& chain, double hostSR) {
    return expectedMiddleChainPeakSamples(chain, PassthroughProcessor{}, hostSR);
}

inline void runMiddleChainSilencePreRoll(RoundTripChain& chain, IProcessor& middle, int nBlocks) {
    chain.hostBuffer.clear();
    juce::AudioBuffer<float> middleBuf;
    for (int n = 0; n < nBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        middleBuf.makeCopyOf(upOut);
        middle.processBlock(middleBuf);
        (void) chain.down.processBlock(middleBuf);
    }
}

inline std::vector<float> collectMiddleChainOutput(RoundTripChain& chain, IProcessor& middle,
                                                   int totalSamples) {
    std::vector<float> collected;
    collected.reserve(static_cast<size_t>(totalSamples));
    juce::AudioBuffer<float> middleBuf;
    while (static_cast<int>(collected.size()) < totalSamples) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        middleBuf.makeCopyOf(upOut);
        middle.processBlock(middleBuf);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(middleBuf);
        for (int i = 0; i < downOut.getNumSamples(); ++i) {
            collected.push_back(downOut.getSample(0, i));
        }
        chain.hostBuffer.clear();
    }
    return collected;
}

inline int productionChainTotalLatency(const ProductionChain& chain, double hostSR) {
    return expectedMiddleChainPeakSamples(chain.resamplers, chain.onnx, hostSR);
}

inline std::vector<float> collectRoundTripOutput(RoundTripChain& chain, int totalSamples) {
    PassthroughProcessor passthrough;
    return collectMiddleChainOutput(chain, passthrough, totalSamples);
}

inline std::vector<float> collectProductionOutput(ProductionChain& chain, int totalSamples) {
    return collectMiddleChainOutput(chain.resamplers, chain.onnx, totalSamples);
}

// --- Single-processor structural helpers (libsamplerate streaming_test patterns) ---

inline void runProcessorSilencePreRoll(ResamplingProcessor& proc, juce::AudioBuffer<float>& buf,
                                       int nBlocks) {
    buf.clear();
    for (int n = 0; n < nBlocks; ++n) {
        (void) proc.processBlock(buf);
    }
}

inline void streamBufferThroughProcessor(ResamplingProcessor& proc, juce::AudioBuffer<float>& buf,
                                         const float* input, int inputLen, std::vector<float>& output) {
    const int blockSize = buf.getNumSamples();
    for (int pos = 0; pos < inputLen; pos += blockSize) {
        buf.clear();
        const int copyLen = std::min(blockSize, inputLen - pos);
        for (int i = 0; i < copyLen; ++i) {
            buf.setSample(0, i, input[pos + i]);
        }
        juce::AudioBuffer<float>& out = proc.processBlock(buf);
        for (int i = 0; i < out.getNumSamples(); ++i) {
            output.push_back(out.getSample(0, i));
        }
    }
}

inline std::vector<float> collectUpOutput(ResamplingProcessor& up, juce::AudioBuffer<float>& hostBuf,
                                          const float* input, int inputLen, int preRollBlocks) {
    runProcessorSilencePreRoll(up, hostBuf, preRollBlocks);
    std::vector<float> output;
    output.reserve(static_cast<size_t>(inputLen * 2));
    streamBufferThroughProcessor(up, hostBuf, input, inputLen, output);
    return output;
}

inline std::vector<float> collectDownOutput(ResamplingProcessor& down, juce::AudioBuffer<float>& onnxBuf,
                                            const float* input, int inputLen, int preRollBlocks) {
    runProcessorSilencePreRoll(down, onnxBuf, preRollBlocks);
    std::vector<float> output;
    output.reserve(static_cast<size_t>(inputLen));
    streamBufferThroughProcessor(down, onnxBuf, input, inputLen, output);
    return output;
}

} // namespace resampling_test
