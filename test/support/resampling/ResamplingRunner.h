#pragma once

/// @file ResamplingRunner.h
/// @brief Unified resampler chain block runners and output collectors.
///
/// Act-phase helpers built on `detail::runChainBlocks`. Public aliases preserve
/// the original API (`runSilencePreRoll`, `collectMiddleChainOutput`, etc.).
/// Single-processor helpers delegate to `scyclone::test::BlockStreaming.h`.
///
/// @namespace scyclone::test::resampling

#include <functional>
#include <vector>
#include <JuceHeader.h>

#include "BlockStreaming.h"
#include "ResamplingTopology.h"
#include "SignalGenerators.h"

namespace scyclone::test::resampling
{

    using scyclone::test::PassthroughProcessor;

    namespace detail
    {

        inline void processMiddleChainBlock(RoundTripChain &chain, IProcessor &middle,
                                            std::vector<float> *output = nullptr)
        {
            juce::AudioBuffer<float> middleBuf;
            juce::AudioBuffer<float> &upOut = chain.up.processBlock(chain.hostBuffer);
            middleBuf.makeCopyOf(upOut);
            middle.processBlock(middleBuf);
            juce::AudioBuffer<float> &downOut = chain.down.processBlock(middleBuf);
            if (output != nullptr)
            {
                scyclone::test::appendBufferSamples(downOut, *output);
            }
        }

        inline bool processProductionBlock(ProductionChain &chain, juce::AudioBuffer<float> &onnxBuf,
                                           std::vector<float> *output = nullptr)
        {
            juce::AudioBuffer<float> &upOut = chain.resamplers.up.processBlock(chain.resamplers.hostBuffer);
            if (upOut.getNumChannels() < 1 || upOut.getNumSamples() < 1)
            {
                return false;
            }
            onnxBuf.setSize(1, upOut.getNumSamples(), false, false, true);
            onnxBuf.copyFrom(0, 0, upOut, 0, 0, upOut.getNumSamples());
            chain.onnx.processBlock(onnxBuf);
            juce::AudioBuffer<float> &downOut = chain.resamplers.down.processBlock(onnxBuf);
            if (output != nullptr)
            {
                scyclone::test::appendBufferSamples(downOut, *output);
            }
            return true;
        }

        enum class ChainRunKind
        {
            RoundTrip,
            MiddleChain,
            Production
        };

        struct ChainRunContext
        {
            ChainRunKind kind = ChainRunKind::RoundTrip;
            RoundTripChain *roundTrip = nullptr;
            ProductionChain *production = nullptr;
            IProcessor *middle = nullptr;
            juce::AudioBuffer<float> *onnxBuf = nullptr;
            double hostSR = 0.0;
            int hostBlock = 0;
            int blockIndexStart = 0;
            std::vector<float> *output = nullptr;
            bool clearHostAfterBlock = false;
            std::function<void(int blockIndex)> fillHost;
        };

        /// Internal unified block loop for all chain pre-roll / warmup / collect paths.
        inline void runChainBlocks(ChainRunContext &ctx, int nBlocks)
        {
            for (int block = 0; block < nBlocks; ++block)
            {
                const int blockIndex = ctx.blockIndexStart + block;
                if (ctx.fillHost)
                {
                    ctx.fillHost(blockIndex);
                }

                switch (ctx.kind)
                {
                case ChainRunKind::RoundTrip:
                    processMiddleChainBlock(*ctx.roundTrip, *ctx.middle, ctx.output);
                    break;
                case ChainRunKind::MiddleChain:
                    processMiddleChainBlock(*ctx.roundTrip, *ctx.middle, ctx.output);
                    break;
                case ChainRunKind::Production:
                    if (!processProductionBlock(*ctx.production, *ctx.onnxBuf, ctx.output))
                    {
                        return;
                    }
                    break;
                }

                if (ctx.clearHostAfterBlock)
                {
                    if (ctx.kind == ChainRunKind::Production)
                    {
                        ctx.production->resamplers.hostBuffer.clear();
                    }
                    else
                    {
                        ctx.roundTrip->hostBuffer.clear();
                    }
                }
            }
        }

    } // namespace detail

    /// Silence pre-roll: up → passthrough → down for @p nBlocks.
    inline void runSilencePreRoll(RoundTripChain &chain, int nBlocks)
    {
        chain.hostBuffer.clear();
        PassthroughProcessor passthrough;
        detail::ChainRunContext ctx;
        ctx.kind = detail::ChainRunKind::RoundTrip;
        ctx.roundTrip = &chain;
        ctx.middle = &passthrough;
        detail::runChainBlocks(ctx, nBlocks);
    }

    /// Production-chain silence pre-roll through full ONNX stub path.
    inline void runProductionSilencePreRoll(ProductionChain &chain, int nBlocks)
    {
        chain.resamplers.hostBuffer.clear();
        juce::AudioBuffer<float> onnxBuf;
        detail::ChainRunContext ctx;
        ctx.kind = detail::ChainRunKind::Production;
        ctx.production = &chain;
        ctx.onnxBuf = &onnxBuf;
        detail::runChainBlocks(ctx, nBlocks);
    }

    /// Middle-chain silence pre-roll with injectable @p middle processor.
    inline void runMiddleChainSilencePreRoll(RoundTripChain &chain, IProcessor &middle, int nBlocks)
    {
        chain.hostBuffer.clear();
        detail::ChainRunContext ctx;
        ctx.kind = detail::ChainRunKind::MiddleChain;
        ctx.roundTrip = &chain;
        ctx.middle = &middle;
        detail::runChainBlocks(ctx, nBlocks);
    }

    /// One production block: appends down output to @p outputSignal.
    inline void processProductionBlock(ProductionChain &chain, juce::AudioBuffer<float> &onnxBuf,
                                       std::vector<float> &outputSignal)
    {
        detail::processProductionBlock(chain, onnxBuf, &outputSignal);
    }

    /// One production block without collecting output (warmup).
    inline void processProductionBlockDiscardOutput(ProductionChain &chain, juce::AudioBuffer<float> &onnxBuf)
    {
        detail::processProductionBlock(chain, onnxBuf, nullptr);
    }

    /// Swept-sine warmup through round-trip chain with custom @p middle.
    inline void runRoundTripSineWarmup(RoundTripChain &chain, double hostSR, int hostBlock,
                                       IProcessor &middle, int nBlocks, int blockIndexStart)
    {
        detail::ChainRunContext ctx;
        ctx.kind = detail::ChainRunKind::MiddleChain;
        ctx.roundTrip = &chain;
        ctx.middle = &middle;
        ctx.hostSR = hostSR;
        ctx.hostBlock = hostBlock;
        ctx.blockIndexStart = blockIndexStart;
        ctx.fillHost = [&](int blockIndex)
        {
            for (int i = 0; i < hostBlock; ++i)
            {
                chain.hostBuffer.setSample(
                    0, i, scyclone::test::generateSweptSine(hostSR, hostBlock, blockIndex, i));
            }
        };
        detail::runChainBlocks(ctx, nBlocks);
    }

    /// Swept-sine warmup through production chain.
    inline void runProductionSineWarmup(ProductionChain &chain, double hostSR, int hostBlock,
                                        int nBlocks, int blockIndexStart)
    {
        juce::AudioBuffer<float> onnxBuf;
        detail::ChainRunContext ctx;
        ctx.kind = detail::ChainRunKind::Production;
        ctx.production = &chain;
        ctx.onnxBuf = &onnxBuf;
        ctx.hostSR = hostSR;
        ctx.hostBlock = hostBlock;
        ctx.blockIndexStart = blockIndexStart;
        ctx.fillHost = [&](int blockIndex)
        {
            for (int i = 0; i < hostBlock; ++i)
            {
                chain.resamplers.hostBuffer.setSample(
                    0, i, scyclone::test::generateSweptSine(hostSR, hostBlock, blockIndex, i));
            }
        };
        detail::runChainBlocks(ctx, nBlocks);
    }

    /// Streams blocks until @p totalSamples down output samples are collected.
    /// Clears host buffer after each block (impulse / steady capture pattern).
    inline std::vector<float> collectMiddleChainOutput(RoundTripChain &chain, IProcessor &middle,
                                                       int totalSamples)
    {
        std::vector<float> collected;
        collected.reserve(static_cast<size_t>(totalSamples));
        detail::ChainRunContext ctx;
        ctx.kind = detail::ChainRunKind::MiddleChain;
        ctx.roundTrip = &chain;
        ctx.middle = &middle;
        ctx.output = &collected;
        ctx.clearHostAfterBlock = true;
        while (static_cast<int>(collected.size()) < totalSamples)
        {
            detail::runChainBlocks(ctx, 1);
        }
        return collected;
    }

    inline std::vector<float> collectRoundTripOutput(RoundTripChain &chain, int totalSamples)
    {
        PassthroughProcessor passthrough;
        return collectMiddleChainOutput(chain, passthrough, totalSamples);
    }

    inline std::vector<float> collectProductionOutput(ProductionChain &chain, int totalSamples)
    {
        return collectMiddleChainOutput(chain.resamplers, chain.onnx, totalSamples);
    }

    /// Single-processor silence pre-roll (structural tests).
    inline void runProcessorSilencePreRoll(ResamplingProcessor &proc, juce::AudioBuffer<float> &buf,
                                           int nBlocks)
    {
        scyclone::test::runSilencePreRollBlocks(proc, buf, nBlocks);
    }

    /// Up-only streaming collection for SNR tests.
    inline std::vector<float> collectUpOutput(ResamplingProcessor &up, juce::AudioBuffer<float> &hostBuf,
                                              const float *input, int inputLen, int preRollBlocks)
    {
        return scyclone::test::collectProcessorOutput(up, hostBuf, input, inputLen, preRollBlocks,
                                                      static_cast<size_t>(inputLen * 2));
    }

    /// Down-only streaming collection for SNR tests.
    inline std::vector<float> collectDownOutput(ResamplingProcessor &down, juce::AudioBuffer<float> &onnxBuf,
                                                const float *input, int inputLen, int preRollBlocks)
    {
        return scyclone::test::collectProcessorOutput(down, onnxBuf, input, inputLen, preRollBlocks,
                                                      static_cast<size_t>(inputLen));
    }

} // namespace scyclone::test::resampling
