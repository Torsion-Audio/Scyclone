#pragma once

/// @file ResamplingContractAssertions.h
/// @brief gtest contract assertions for Scyclone resampling behavioral tests.
///
/// Assert-phase helpers: call `measure*` from ResamplingMeasurements.h internally
/// or assert directly on processor I/O. Steady chain loops delegate to
/// ResamplingChainDriver.h; pre-roll/warmup/impulse collectors use ResamplingRunner.h.
/// Tolerance constants are tuned via CalibrationProbe DISABLED tests. Impulse peak/side-lobe
/// contracts live in `torsion::test::ImpulseAssertions.h`.
///
/// @namespace scyclone::test::resampling

#include <cmath>
#include <gtest/gtest.h>
#include <random>

#include "ImpulseAssertions.h"
#include "ResamplingChainDriver.h"
#include "ResamplingMeasurements.h"

namespace scyclone::test::resampling
{

    using torsion::test::assertImpulseResponse;
    using torsion::test::kLongRunBlocks;
    using torsion::test::kPreRollBlocks;
    using torsion::test::kSteadyBlocks;

    /// CI round-trip swept-sine RMS ceiling (passthrough middle).
    constexpr float kRoundTripRmsTolerance = 0.15f;
    /// Production 48 kHz RMS ceiling (CalibrationProbe worst ~0.26 + margin).
    constexpr float kProductionRoundTripRmsTolerance = 0.3f;

    /// Steady-state: host block in == host block out (round-trip, passthrough path).
    inline void assertBlockSizePreserved(RoundTripChain &chain, int hostBlock, int nBlocks = kSteadyBlocks)
    {
        const auto sizes = steadyRoundTripBlockSizes(chain, nBlocks);
        for (int n = 0; n < nBlocks; ++n)
        {
            EXPECT_EQ(sizes[static_cast<size_t>(n)], hostBlock)
                << "block=" << hostBlock << " steady block " << n;
        }
    }

    /// Steady-state: host block in == host block out (production chain).
    inline void assertProductionBlockSizePreserved(ProductionChain &chain, int hostBlock,
                                                   int nBlocks = kSteadyBlocks)
    {
        const auto sizes = steadyProductionBlockSizes(chain, nBlocks);
        for (int n = 0; n < nBlocks; ++n)
        {
            EXPECT_EQ(sizes[static_cast<size_t>(n)], hostBlock)
                << "block=" << hostBlock << " steady block " << n;
        }
    }

    /// Per-processor steady I/O: last frames used/generated match expected in/out sizes.
    inline void assertProcessorSteadyStateFullIo(ResamplingProcessor &proc, juce::AudioBuffer<float> &buf,
                                                 int expectedIn, int expectedOut, int nBlocks = kSteadyBlocks)
    {
        for (int n = 0; n < nBlocks; ++n)
        {
            buf.clear();
            juce::AudioBuffer<float> &out = proc.processBlock(buf);
            EXPECT_EQ(proc.getLastInputFramesUsed(), expectedIn)
                << "steady block " << n;
            EXPECT_EQ(proc.getLastOutputFramesGenerated(), expectedOut)
                << "steady block " << n;
            EXPECT_EQ(out.getNumSamples(), expectedOut);
        }
    }

    /// Silence in → silence out after pre-roll (DC leak guard).
    inline void assertRoundTripSilenceOut(RoundTripChain &chain, int hostBlock,
                                          int nBlocks = kSteadyBlocks, float maxAbs = 1.0e-5f)
    {
        const auto blocks = steadyRoundTripSilenceOutputs(chain, nBlocks);
        for (const auto &blockOut : blocks)
        {
            for (const float sample : blockOut)
            {
                EXPECT_LT(std::abs(sample), maxAbs);
            }
        }
        juce::ignoreUnused(hostBlock);
    }

    /// Random host input → finite down output (NaN/Inf guard).
    inline void assertRoundTripFiniteOutput(RoundTripChain &chain, int nBlocks = 8)
    {
        std::mt19937 rng(42);
        const auto blocks = steadyRoundTripRandomOutputs(chain, nBlocks, rng);
        for (const auto &blockOut : blocks)
        {
            for (const float s : blockOut)
            {
                EXPECT_FALSE(std::isnan(s));
                EXPECT_FALSE(std::isinf(s));
            }
        }
    }

    inline void assertRoundTripSignalFidelity(RoundTripChain &chain, double hostSR, int hostBlock,
                                              IProcessor &middle, float rmsTolerance)
    {
        const float rmsError = measureRoundTripRmsError(chain, hostSR, hostBlock, middle);
        EXPECT_LT(rmsError, rmsTolerance)
            << "hostSR=" << hostSR << " block=" << hostBlock;
    }

    inline void assertProductionRoundTripSignalFidelity(ProductionChain &chain, double hostSR, int hostBlock,
                                                        float rmsTolerance)
    {
        const float rmsError = measureProductionRmsError(chain, hostSR, hostBlock);
        EXPECT_LT(rmsError, rmsTolerance)
            << "hostSR=" << hostSR << " block=" << hostBlock;
    }

    inline void assertWarmupPartialBlockTailZeroed(ResamplingProcessor &proc, juce::AudioBuffer<float> &buf)
    {
        buf.clear();
        const int outSize = proc.getOutputBufferSize();
        for (int block = 0; block < 64; ++block)
        {
            juce::AudioBuffer<float> &out = proc.processBlock(buf);
            const long framesGen = proc.getLastOutputFramesGenerated();
            if (framesGen < outSize)
            {
                for (int i = static_cast<int>(framesGen); i < outSize; ++i)
                {
                    EXPECT_FLOAT_EQ(out.getSample(0, i), 0.0f)
                        << "stale partial-block tail at block=" << block << " sample=" << i
                        << " framesGen=" << framesGen << " outSize=" << outSize;
                }
                return;
            }
            buf.clear();
        }
        GTEST_SKIP() << "no partial output block within 64 blocks (outSize=" << outSize << ")";
    }

    inline void assertImpulsePeakNearLatency(RoundTripChain &chain, double hostSR, int hostBlock)
    {
        const ImpulseResponse response = measureRoundTripImpulse(chain, hostSR, hostBlock);
        assertImpulseResponse(response);
    }

    inline void assertProductionImpulsePeakNearLatency(ProductionChain &chain, double hostSR, int hostBlock)
    {
        const ImpulseResponse response = measureProductionImpulse(chain, hostSR, hostBlock);
        assertImpulseResponse(response);
    }

} // namespace scyclone::test::resampling
