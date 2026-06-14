#pragma once

/// @file ResamplingTopology.h
/// @brief Resampler chain topology, ONNX-path constants, and host-config feasibility.
///
/// Arrange-phase helpers: chain structs, `prepare*`, and reported-latency math.
/// Does not run block loops (see ResamplingRunner.h) or assert contracts.
///
/// @namespace scyclone::test::resampling

#include <cmath>
#include <gtest/gtest.h>
#include <vector>
#include <JuceHeader.h>

#include "PassthroughProcessor.h"
#include "SimulatedOnnxProcessor.h"
#include "dsp/IProcessor.h"
#include "dsp/resampler/ResamplingProcessor.h"
#include "dsp/utils/utils.h"

namespace scyclone::test::resampling
{

    using scyclone::test::PassthroughProcessor;

    constexpr double kOnnxRate = 48000.0;
    /// Minimum ONNX block size enforced by production graph feasibility checks.
    constexpr int kMinOnnxBlock = 32;
    /// Default silence blocks before steady-state assertions.
    constexpr int kPreRollBlocks = 4;
    /// Steady-state blocks for block-size and RMS contract windows.
    constexpr int kSteadyBlocks = 8;
    /// Long-run conservation test block count.
    constexpr int kLongRunBlocks = 100;

    /// Parameter for gtest host-rate / block-size matrices.
    struct HostConfig
    {
        double hostSR;
        int hostBlock;
    };

    /// ONNX up-path output block size at a host rate/block (ceil sizing).
    inline int upOutputBlockSize(double hostSR, int hostBlock)
    {
        return static_cast<int>(std::ceil(kOnnxRate / hostSR * static_cast<double>(hostBlock)));
    }

    /// Expected production up-path ratio case for data-driven structural tests.
    struct ProductionRatioCase
    {
        double hostSR;
        int hostBlock;
        int expectedUpOut;
        double upSrcRatio;
    };

    inline ProductionRatioCase makeProductionRatioCase(double hostSR, int hostBlock)
    {
        const int expectedUpOut = upOutputBlockSize(hostSR, hostBlock);
        return {hostSR, hostBlock, expectedUpOut,
                static_cast<double>(expectedUpOut) / static_cast<double>(hostBlock)};
    }

    /// Uncoupled down output size (ceil footgun — documented in IsolationTest).
    inline int uncoupledDownOutputSize(double hostSR, int upBlock)
    {
        return static_cast<int>(std::ceil(hostSR / kOnnxRate * static_cast<double>(upBlock)));
    }

    /// True when up-path ONNX block meets @ref kMinOnnxBlock.
    inline bool isFeasibleHostConfig(double hostSR, int hostBlock)
    {
        return upOutputBlockSize(hostSR, hostBlock) >= kMinOnnxBlock;
    }

    /// Skips the current test via `GTEST_SKIP` when config is infeasible.
    inline void skipIfInfeasible(const HostConfig &cfg)
    {
        if (!isFeasibleHostConfig(cfg.hostSR, cfg.hostBlock))
        {
            GTEST_SKIP() << "up output < " << kMinOnnxBlock << " for hostSR=" << cfg.hostSR
                         << " block=" << cfg.hostBlock;
        }
    }

    /// up → down chain with shared host buffer (passthrough or custom middle via runner).
    struct RoundTripChain
    {
        ResamplingProcessor up;
        ResamplingProcessor down;
        juce::AudioBuffer<float> hostBuffer;
        int upOutSize = 0;
    };

    /// Full production topology: up → SimulatedOnnx → down.
    struct ProductionChain
    {
        RoundTripChain resamplers;
        SimulatedOnnxProcessor onnx;
    };

    /// In-place prepare for an existing @p chain (host buffer allocated and cleared).
    inline void initRoundTripChain(RoundTripChain &chain, double hostSR, int hostBlock,
                                   bool forceDownToHost = true)
    {
        juce::dsp::ProcessSpec monoSpec{hostSR, static_cast<uint32_t>(hostBlock), 1};
        chain.upOutSize = chain.up.prepare(monoSpec, kOnnxRate, "up-test");
        juce::dsp::ProcessSpec onnxSpec{kOnnxRate, static_cast<uint32_t>(chain.upOutSize), 1};
        if (forceDownToHost)
        {
            chain.down.prepare(onnxSpec, hostSR, "down-test", hostBlock);
        }
        else
        {
            chain.down.prepare(onnxSpec, hostSR, "down-test");
        }
        chain.hostBuffer.setSize(1, hostBlock);
        chain.hostBuffer.clear();
    }

    /// Returns a prepared round-trip chain (value convenience for tests).
    inline RoundTripChain prepareRoundTripChain(double hostSR, int hostBlock, bool forceDownToHost = true)
    {
        RoundTripChain chain;
        initRoundTripChain(chain, hostSR, hostBlock, forceDownToHost);
        return chain;
    }

    inline void initProductionChain(ProductionChain &chain, double hostSR, int hostBlock)
    {
        initRoundTripChain(chain.resamplers, hostSR, hostBlock);
        juce::dsp::ProcessSpec onnxSpec{
            kOnnxRate, static_cast<uint32_t>(chain.resamplers.upOutSize), 1};
        chain.onnx.prepare(onnxSpec);
    }

    inline ProductionChain prepareProductionChain(double hostSR, int hostBlock)
    {
        ProductionChain chain;
        initProductionChain(chain, hostSR, hostBlock);
        return chain;
    }

    /// Prepare up-only processor for structural / SNR tests.
    inline int prepareUpOnly(double hostSR, int hostBlock, ResamplingProcessor &up)
    {
        juce::dsp::ProcessSpec monoSpec{hostSR, static_cast<uint32_t>(hostBlock), 1};
        return up.prepare(monoSpec, kOnnxRate, "up-only");
    }

    /// Prepare down-only processor; @p forceDownToHost enables production coupled sizing.
    inline int prepareDownOnly(double hostSR, int upBlock, int hostBlock, ResamplingProcessor &down,
                               bool forceDownToHost = false)
    {
        juce::dsp::ProcessSpec onnxSpec{kOnnxRate, static_cast<uint32_t>(upBlock), 1};
        if (forceDownToHost)
        {
            return down.prepare(onnxSpec, hostSR, "down-only", hostBlock);
        }
        return down.prepare(onnxSpec, hostSR, "down-only");
    }

    /// Pre-roll block count from bulk reported latency (latency blocks + @ref kPreRollBlocks).
    inline int productionPreRollBlocks(int totalLatencyHost, int hostBlock)
    {
        return (totalLatencyHost + hostBlock - 1) / hostBlock + kPreRollBlocks;
    }

    /// Expected impulse peak index: sum of reported latencies converted to host rate.
    inline int expectedMiddleChainPeakSamples(const RoundTripChain &chain, const IProcessor &middle,
                                              double hostSR)
    {
        return utils::computeTotalLatencyInSamples(
            chain.up.getLatencyInSamples(),
            middle.getLatencyInSamples(),
            chain.down.getLatencyInSamples(),
            kOnnxRate,
            hostSR);
    }

    inline int expectedRoundTripPeakSamples(const RoundTripChain &chain, double hostSR)
    {
        return expectedMiddleChainPeakSamples(chain, PassthroughProcessor{}, hostSR);
    }

    inline int productionChainTotalLatency(const ProductionChain &chain, double hostSR)
    {
        return expectedMiddleChainPeakSamples(chain.resamplers, chain.onnx, hostSR);
    }

} // namespace scyclone::test::resampling
