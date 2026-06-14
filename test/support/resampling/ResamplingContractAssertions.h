#pragma once

/// @file ResamplingContractAssertions.h
/// @brief gtest contract assertions for resampling behavioral tests.
///
/// Assert-phase helpers: call `measure*` from ResamplingMeasurements.h internally
/// or assert directly on processor I/O. Tolerance constants are tuned via
/// CalibrationProbe DISABLED tests.
///
/// @namespace resampling_test

#include <cmath>
#include <gtest/gtest.h>
#include <random>
#include "ResamplingMeasurements.h"

namespace resampling_test {

/// CI round-trip swept-sine RMS ceiling (passthrough middle).
constexpr float kRoundTripRmsTolerance = 0.15f;
/// Production 48 kHz RMS ceiling (CalibrationProbe worst ~0.26 + margin).
constexpr float kProductionRoundTripRmsTolerance = 0.3f;
/// Max secondary impulse lobe as fraction of main peak.
constexpr float kImpulseSecondaryPeakRatio = 0.15f;

/// Steady-state: host block in == host block out (round-trip, passthrough path).
inline void assertBlockSizePreserved(RoundTripChain& chain, int hostBlock, int nBlocks = kSteadyBlocks) {
    chain.hostBuffer.clear();
    for (int n = 0; n < nBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        EXPECT_EQ(downOut.getNumSamples(), hostBlock)
            << "block=" << hostBlock << " steady block " << n;
    }
}

/// Steady-state: host block in == host block out (production chain).
inline void assertProductionBlockSizePreserved(ProductionChain& chain, int hostBlock,
                                               int nBlocks = kSteadyBlocks) {
    chain.resamplers.hostBuffer.clear();
    juce::AudioBuffer<float> onnxBuf;
    for (int n = 0; n < nBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.resamplers.up.processBlock(chain.resamplers.hostBuffer);
        onnxBuf.makeCopyOf(upOut);
        chain.onnx.processBlock(onnxBuf);
        juce::AudioBuffer<float>& downOut = chain.resamplers.down.processBlock(onnxBuf);
        EXPECT_EQ(downOut.getNumSamples(), hostBlock)
            << "block=" << hostBlock << " steady block " << n;
    }
}

/// Per-processor steady state: full input consumed, full output generated.
inline void assertProcessorSteadyStateFullIo(ResamplingProcessor& proc, juce::AudioBuffer<float>& buf,
                                             int expectedIn, int expectedOut, int nBlocks = kSteadyBlocks) {
    for (int n = 0; n < nBlocks; ++n) {
        buf.clear();
        juce::AudioBuffer<float>& out = proc.processBlock(buf);
        EXPECT_EQ(proc.getLastInputFramesUsed(), expectedIn)
            << "steady block " << n;
        EXPECT_EQ(proc.getLastOutputFramesGenerated(), expectedOut)
            << "steady block " << n;
        EXPECT_EQ(out.getNumSamples(), expectedOut);
    }
}

/// Silence in → near-zero out after pre-roll (no DC leak).
inline void assertRoundTripSilenceOut(RoundTripChain& chain, int hostBlock,
                                      int nBlocks = kSteadyBlocks, float maxAbs = 1.0e-5f) {
    chain.hostBuffer.clear();
    for (int n = 0; n < nBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        for (int i = 0; i < downOut.getNumSamples(); ++i) {
            EXPECT_LT(std::abs(downOut.getSample(0, i)), maxAbs);
        }
    }
}

/// Random finite input → finite output (NaN/Inf guard).
inline void assertRoundTripFiniteOutput(RoundTripChain& chain, int nBlocks = 8) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

    for (int n = 0; n < nBlocks; ++n) {
        for (int i = 0; i < chain.hostBuffer.getNumSamples(); ++i) {
            chain.hostBuffer.setSample(0, i, dist(rng));
        }
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        for (int i = 0; i < downOut.getNumSamples(); ++i) {
            const float s = downOut.getSample(0, i);
            EXPECT_FALSE(std::isnan(s));
            EXPECT_FALSE(std::isinf(s));
        }
    }
}

/// Swept-sine RMS round-trip fidelity vs @p rmsTolerance.
inline void assertRoundTripSignalFidelity(RoundTripChain& chain, double hostSR, int hostBlock,
                                          IProcessor& middle, float rmsTolerance) {
    const float rmsError = measureRoundTripRmsError(chain, hostSR, hostBlock, middle);
    EXPECT_LT(rmsError, rmsTolerance)
        << "hostSR=" << hostSR << " block=" << hostBlock;
}

/// Production-chain swept-sine RMS fidelity vs @p rmsTolerance.
inline void assertProductionRoundTripSignalFidelity(ProductionChain& chain, double hostSR, int hostBlock,
                                                    float rmsTolerance) {
    const float rmsError = measureProductionRmsError(chain, hostSR, hostBlock);
    EXPECT_LT(rmsError, rmsTolerance)
        << "hostSR=" << hostSR << " block=" << hostBlock;
}

/// Impulse peak index within @p tolerance of `expectedPeak`.
inline void assertImpulsePeakWithinTolerance(const ImpulseResponse& response,
                                             int tolerance = kImpulsePeakToleranceSamples) {
    ASSERT_GE(response.narrowPeakPos, 0)
        << "no impulse peak found near expectedPeak=" << response.expectedPeak
        << " widePeakPos=" << response.widePeakPos << " wideDelta="
        << (response.widePeakPos - response.expectedPeak);
    EXPECT_GE(response.narrowPeakPos, response.expectedPeak - tolerance)
        << "expectedPeak=" << response.expectedPeak << " narrowPeakPos=" << response.narrowPeakPos
        << " widePeakPos=" << response.widePeakPos;
    EXPECT_LE(response.narrowPeakPos, response.expectedPeak + tolerance)
        << "expectedPeak=" << response.expectedPeak << " narrowPeakPos=" << response.narrowPeakPos
        << " widePeakPos=" << response.widePeakPos;
}

/// No duplicate impulse lobes above @p secondaryRatio × main peak.
inline void assertImpulseSideLobesBelowThreshold(const ImpulseResponse& response,
                                                 float secondaryRatio = kImpulseSecondaryPeakRatio) {
    ASSERT_GE(response.narrowPeakPos, 0);
    ASSERT_GT(response.narrowPeakVal, 1.0e-6f)
        << "narrowPeakPos=" << response.narrowPeakPos
        << " widePeakPos=" << response.widePeakPos
        << " widePeakVal=" << response.widePeakVal;
    EXPECT_LE(response.maxOutsideMainWindow, secondaryRatio * response.narrowPeakVal)
        << "duplicate/truncated impulse lobe; narrowPeakPos=" << response.narrowPeakPos
        << " maxOutside=" << response.maxOutsideMainWindow
        << " narrowPeakVal=" << response.narrowPeakVal;
}

/// Combined peak position + side-lobe contract.
inline void assertImpulseResponse(const ImpulseResponse& response,
                                  int peakTolerance = kImpulsePeakToleranceSamples,
                                  float secondaryRatio = kImpulseSecondaryPeakRatio) {
    assertImpulsePeakWithinTolerance(response, peakTolerance);
    assertImpulseSideLobesBelowThreshold(response, secondaryRatio);
}

/// First partial SRC block must zero samples [framesGen, outSize).
inline void assertWarmupPartialBlockTailZeroed(ResamplingProcessor& proc, juce::AudioBuffer<float>& buf) {
    buf.clear();
    const int outSize = proc.getOutputBufferSize();
    for (int block = 0; block < 64; ++block) {
        juce::AudioBuffer<float>& out = proc.processBlock(buf);
        const long framesGen = proc.getLastOutputFramesGenerated();
        if (framesGen < outSize) {
            for (int i = static_cast<int>(framesGen); i < outSize; ++i) {
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

/// Round-trip impulse peak at reported total latency.
inline void assertImpulsePeakNearLatency(RoundTripChain& chain, double hostSR, int hostBlock) {
    const ImpulseResponse response = measureRoundTripImpulse(chain, hostSR, hostBlock);
    assertImpulseResponse(response);
}

/// Production-chain impulse peak at reported total latency.
inline void assertProductionImpulsePeakNearLatency(ProductionChain& chain, double hostSR, int hostBlock) {
    const ImpulseResponse response = measureProductionImpulse(chain, hostSR, hostBlock);
    assertImpulseResponse(response);
}

} // namespace resampling_test
