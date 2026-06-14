#pragma once

/// @file ResamplingChainDriver.h
/// @brief Act-phase helpers for resampler chain contract tests (steady loops + sample capture).
///
/// Complements @ref ResamplingRunner.h:
/// - **Runner** — pre-roll, sine warmup, impulse / fixed-length collectors (`ChainRunContext`).
/// - **ChainDriver** — repeated steady blocks and aligned input/output capture for measure/assert.
///
/// All chain `processBlock` loops for contracts should go through Runner or ChainDriver,
/// not assert/measure headers. Round-trip paths use PassthroughProcessor + buffer copy
/// (equivalent to legacy direct up→down for zero-latency passthrough).
///
/// @namespace scyclone::test::resampling

#include <random>
#include <vector>
#include <JuceHeader.h>

#include "LongRun.h"
#include "ResamplingRunner.h"
#include "SignalGenerators.h"

namespace scyclone::test::resampling
{

    /// Host input + chain output captured block-by-block (RMS fidelity).
    struct InputOutputSignals
    {
        std::vector<float> input;
        std::vector<float> output;
    };

    /// Host-buffer snapshot + chain output per block (domain-neutral; mixer interprets dry/wet).
    struct HostAndChainOutput
    {
        std::vector<float> host;
        std::vector<float> chainOutput;
    };

    /// @return Down output size per steady silence block (round-trip, passthrough middle).
    inline std::vector<int> steadyRoundTripBlockSizes(RoundTripChain &chain, int nBlocks)
    {
        chain.hostBuffer.clear();
        PassthroughProcessor passthrough;
        std::vector<int> sizes;
        sizes.reserve(static_cast<size_t>(nBlocks));
        for (int n = 0; n < nBlocks; ++n)
        {
            sizes.push_back(detail::processMiddleChainBlock(chain, passthrough, nullptr));
        }
        return sizes;
    }

    /// @return Per-block down output after silence in (for DC-leak checks).
    inline std::vector<std::vector<float>> steadyRoundTripSilenceOutputs(RoundTripChain &chain, int nBlocks)
    {
        chain.hostBuffer.clear();
        PassthroughProcessor passthrough;
        std::vector<std::vector<float>> blocks;
        blocks.reserve(static_cast<size_t>(nBlocks));
        for (int n = 0; n < nBlocks; ++n)
        {
            std::vector<float> blockOut;
            detail::processMiddleChainBlock(chain, passthrough, &blockOut);
            blocks.push_back(std::move(blockOut));
        }
        return blocks;
    }

    /// @return Per-block down output after random host input (NaN/Inf guard).
    inline std::vector<std::vector<float>> steadyRoundTripRandomOutputs(RoundTripChain &chain, int nBlocks,
                                                                        std::mt19937 &rng)
    {
        std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
        PassthroughProcessor passthrough;
        std::vector<std::vector<float>> blocks;
        blocks.reserve(static_cast<size_t>(nBlocks));
        for (int n = 0; n < nBlocks; ++n)
        {
            for (int i = 0; i < chain.hostBuffer.getNumSamples(); ++i)
            {
                chain.hostBuffer.setSample(0, i, dist(rng));
            }
            std::vector<float> blockOut;
            detail::processMiddleChainBlock(chain, passthrough, &blockOut);
            blocks.push_back(std::move(blockOut));
        }
        return blocks;
    }

    /// @return Down output size per steady silence block (production chain).
    inline std::vector<int> steadyProductionBlockSizes(ProductionChain &chain, int nBlocks)
    {
        chain.resamplers.hostBuffer.clear();
        juce::AudioBuffer<float> onnxBuf;
        std::vector<int> sizes;
        sizes.reserve(static_cast<size_t>(nBlocks));
        for (int n = 0; n < nBlocks; ++n)
        {
            sizes.push_back(detail::processProductionBlock(chain, onnxBuf, nullptr));
        }
        return sizes;
    }

    /// Swept-sine host fill + middle chain; collects at least @p requiredOutputSamples on output.
    inline InputOutputSignals collectMiddleChainInputOutput(RoundTripChain &chain, IProcessor &middle,
                                                            double hostSR, int hostBlock,
                                                            int blockIndexStart, int requiredOutputSamples)
    {
        InputOutputSignals signals;
        signals.input.reserve(static_cast<size_t>(requiredOutputSamples));
        signals.output.reserve(static_cast<size_t>(requiredOutputSamples));

        int blockIndex = blockIndexStart;
        while (static_cast<int>(signals.output.size()) < requiredOutputSamples)
        {
            for (int i = 0; i < hostBlock; ++i)
            {
                chain.hostBuffer.setSample(
                    0, i, torsion::test::generateSweptSine(hostSR, hostBlock, blockIndex, i));
            }
            for (int i = 0; i < hostBlock; ++i)
            {
                signals.input.push_back(chain.hostBuffer.getSample(0, i));
            }
            detail::processMiddleChainBlock(chain, middle, &signals.output);
            ++blockIndex;
        }
        return signals;
    }

    /// Production-chain counterpart to @ref collectMiddleChainInputOutput.
    inline InputOutputSignals collectProductionInputOutput(ProductionChain &chain, double hostSR, int hostBlock,
                                                           int blockIndexStart, int requiredOutputSamples)
    {
        InputOutputSignals signals;
        signals.input.reserve(static_cast<size_t>(requiredOutputSamples));
        signals.output.reserve(static_cast<size_t>(requiredOutputSamples));
        juce::AudioBuffer<float> onnxBuf;

        int blockIndex = blockIndexStart;
        while (static_cast<int>(signals.output.size()) < requiredOutputSamples)
        {
            for (int i = 0; i < hostBlock; ++i)
            {
                chain.resamplers.hostBuffer.setSample(
                    0, i, torsion::test::generateSweptSine(hostSR, hostBlock, blockIndex, i));
            }
            for (int i = 0; i < hostBlock; ++i)
            {
                signals.input.push_back(chain.resamplers.hostBuffer.getSample(0, i));
            }
            detail::processProductionBlock(chain, onnxBuf, &signals.output);
            ++blockIndex;
        }
        return signals;
    }

    /// Snapshot host buffer then run production chain each block until @p collectSamples wet samples.
    inline HostAndChainOutput collectProductionHostAndOutput(ProductionChain &chain, int collectSamples)
    {
        HostAndChainOutput result;
        result.host.reserve(static_cast<size_t>(collectSamples));
        result.chainOutput.reserve(static_cast<size_t>(collectSamples));
        juce::AudioBuffer<float> onnxBuf;

        while (static_cast<int>(result.chainOutput.size()) < collectSamples)
        {
            for (int i = 0; i < chain.resamplers.hostBuffer.getNumSamples(); ++i)
            {
                result.host.push_back(chain.resamplers.hostBuffer.getSample(0, i));
            }
            detail::processProductionBlock(chain, onnxBuf, &result.chainOutput);
            chain.resamplers.hostBuffer.clear();
        }
        return result;
    }

    /// Long-run host in/out frame totals (passthrough middle, silence blocks).
    inline torsion::test::LongRunCounts runRoundTripLongRunSampleCount(RoundTripChain &chain, int nBlocks)
    {
        PassthroughProcessor passthrough;
        return torsion::test::runLongBlockLoop(nBlocks, [&](int, torsion::test::LongRunCounts &c)
        {
            chain.hostBuffer.clear();
            c.inputTotal += chain.hostBuffer.getNumSamples();
            c.outputTotal += detail::processMiddleChainBlock(chain, passthrough, nullptr);
        });
    }

} // namespace scyclone::test::resampling
