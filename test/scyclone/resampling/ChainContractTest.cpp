// Chain contracts — round-trip (passthrough) and production (ONNX stub) over default CI configs.
// See docs/resampling_architecture.md

#include <gtest/gtest.h>
#include "HostConfigCatalog.h"
#include "LongRun.h"
#include "PassthroughProcessor.h"
#include "ResamplingContractAssertions.h"
#include "ResamplingFixtures.h"
#include "ScycloneHostPresets.h"
#include "TestTiming.h"

using namespace torsion::test;
using namespace scyclone::test::resampling;

// Signal: silence → up → middle → down; every steady block returns host N samples.
TEST_P(ChainContractTest, OutputBlockEqualsHostBlock) {
    const auto& testCase = GetParam();
    skipIfInfeasible(testCase.cfg);

    if (testCase.chain == ChainKind::RoundTrip) {
        auto chain = prepareRoundTripChain(testCase.cfg.hostSR, testCase.cfg.hostBlock);
        runSilencePreRoll(chain, kPreRollBlocks);
        assertBlockSizePreserved(chain, testCase.cfg.hostBlock);
        return;
    }

    auto chain = prepareProductionChain(testCase.cfg.hostSR, testCase.cfg.hostBlock);
    const int lat = productionChainTotalLatency(chain, testCase.cfg.hostSR);
    runProductionSilencePreRoll(chain, latencyPreRollBlocks(lat, testCase.cfg.hostBlock));
    assertProductionBlockSizePreserved(chain, testCase.cfg.hostBlock);
}

// Signal: swept sine through chain; compare output at reported latency to input RMS.
TEST_P(ChainContractTest, SweptSine_RmsWithinTolerance) {
    const auto& testCase = GetParam();
    skipIfInfeasible(testCase.cfg);

    if (testCase.chain == ChainKind::Production && testCase.cfg.hostSR != kOnnxRate) {
        GTEST_SKIP() << "cross-rate production swept-sine group delay != bulk reported latency";
    }

    if (testCase.chain == ChainKind::RoundTrip) {
        auto chain = prepareRoundTripChain(testCase.cfg.hostSR, testCase.cfg.hostBlock);
        PassthroughProcessor passthrough;
        assertRoundTripSignalFidelity(
            chain, testCase.cfg.hostSR, testCase.cfg.hostBlock, passthrough, kRoundTripRmsTolerance);
        return;
    }

    auto chain = prepareProductionChain(testCase.cfg.hostSR, testCase.cfg.hostBlock);
    assertProductionRoundTripSignalFidelity(
        chain, testCase.cfg.hostSR, testCase.cfg.hostBlock, kProductionRoundTripRmsTolerance);
}

// Signal: impulse at sample 0 → chain; peak must land near reported total latency.
TEST_P(ChainContractTest, ImpulsePeak_MatchesReportedLatency) {
    const auto& testCase = GetParam();
    skipIfInfeasible(testCase.cfg);

    if (testCase.chain == ChainKind::Production && testCase.cfg.hostSR != kOnnxRate) {
        GTEST_SKIP() << "cross-rate production impulse group delay != productionChainTotalLatency";
    }

    if (testCase.chain == ChainKind::RoundTrip) {
        auto chain = prepareRoundTripChain(testCase.cfg.hostSR, testCase.cfg.hostBlock);
        assertImpulsePeakNearLatency(chain, testCase.cfg.hostSR, testCase.cfg.hostBlock);
        return;
    }

    auto chain = prepareProductionChain(testCase.cfg.hostSR, testCase.cfg.hostBlock);
    assertProductionImpulsePeakNearLatency(chain, testCase.cfg.hostSR, testCase.cfg.hostBlock);
}

// Signal: round-trip block-size check on extended host matrix (one steady block only).
TEST_P(ExtendedHostMatrixTest, RoundTrip_BlockSize_FirstSteadyBlock) {
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    assertBlockSizePreserved(chain, cfg.hostBlock, 1);
}

// Signal: 100 blocks silence round-trip; host sample count in ≈ out within SRC terminate tolerance.
TEST_P(RoundTripCiTest, LongRun_SampleCountConservation) {
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    const auto counts = runLongBlockLoop(kLongRunBlocks, [&](int, LongRunCounts &c)
    {
        chain.hostBuffer.clear();
        c.inputTotal += chain.hostBuffer.getNumSamples();
        juce::AudioBuffer<float> &upOut = chain.up.processBlock(chain.hostBuffer);
        juce::AudioBuffer<float> &downOut = chain.down.processBlock(upOut);
        c.outputTotal += downOut.getNumSamples();
    });
    const double effectiveRatio = static_cast<double>(counts.outputTotal) / static_cast<double>(counts.inputTotal);
    const int tol = longRunTolerance(effectiveRatio);
    EXPECT_LE(std::abs(counts.outputTotal - counts.inputTotal), tol)
        << "host in/out should match within tolerance; hostSR=" << cfg.hostSR;

    assertRoundTripFiniteOutput(chain);
}

// Signal: silence in after pre-roll → up → down → near-zero out (no DC leak).
TEST_P(RoundTripCiTest, SilenceIn_SilenceOut_AfterPreRoll) {
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    assertRoundTripSilenceOut(chain, cfg.hostBlock);
}

INSTANTIATE_TEST_SUITE_P(ChainContract, ChainContractTest,
                         chainContractValuesFor(defaultCiHostConfigs()), chainContractCaseName);
INSTANTIATE_TEST_SUITE_P(RoundTripCi, RoundTripCiTest,
                         hostConfigValues(defaultCiHostConfigs()), hostConfigName);
INSTANTIATE_TEST_SUITE_P(ExtendedHostMatrix, ExtendedHostMatrixTest,
                         hostConfigValues(extendedHostMatrixConfigs()), hostConfigName);
