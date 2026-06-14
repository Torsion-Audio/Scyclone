// Chain contracts — round-trip (passthrough) and production (ONNX stub) over default CI configs.
// See docs/resampling_architecture.md

#include <gtest/gtest.h>
#include "HostConfigCatalog.h"
#include "LongRun.h"
#include "PassthroughProcessor.h"
#include "ResamplingChainDriver.h"
#include "ResamplingContractAssertions.h"
#include "ResamplingFixtures.h"
#include "ScycloneHostPresets.h"
#include "TestTiming.h"

using namespace torsion::test;
using namespace scyclone::test::resampling;

// Signal: silence → up → passthrough → down; every steady block returns host N samples.
TEST_P(RoundTripChainContractTest, OutputBlockEqualsHostBlock)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    assertBlockSizePreserved(chain, cfg.hostBlock);
}

// Signal: swept sine through round-trip chain; compare output at reported latency to input RMS.
TEST_P(RoundTripChainContractTest, SweptSine_RmsWithinTolerance)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    PassthroughProcessor passthrough;
    assertRoundTripSignalFidelity(
        chain, cfg.hostSR, cfg.hostBlock, passthrough, kRoundTripRmsTolerance);
}

// Signal: impulse at sample 0 → round-trip chain; peak must land near reported total latency.
TEST_P(RoundTripChainContractTest, ImpulsePeak_MatchesReportedLatency)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    assertImpulsePeakNearLatency(chain, cfg.hostSR, cfg.hostBlock);
}

// Signal: silence → production chain; every steady block returns host N samples.
TEST_P(ProductionChainContractTest, OutputBlockEqualsHostBlock)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareProductionChain(cfg.hostSR, cfg.hostBlock);
    const int lat = productionChainTotalLatency(chain, cfg.hostSR);
    runProductionSilencePreRoll(chain, latencyPreRollBlocks(lat, cfg.hostBlock));
    assertProductionBlockSizePreserved(chain, cfg.hostBlock);
}

// Signal: swept sine through production chain (48 kHz); RMS at bulk reported latency.
TEST_P(Production48kSignalContractTest, SweptSine_RmsWithinTolerance)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareProductionChain(cfg.hostSR, cfg.hostBlock);
    assertProductionRoundTripSignalFidelity(
        chain, cfg.hostSR, cfg.hostBlock, kProductionRoundTripRmsTolerance);
}

// Signal: impulse at sample 0 → production chain (48 kHz); peak near total latency.
TEST_P(Production48kSignalContractTest, ImpulsePeak_MatchesReportedLatency)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareProductionChain(cfg.hostSR, cfg.hostBlock);
    assertProductionImpulsePeakNearLatency(chain, cfg.hostSR, cfg.hostBlock);
}

// Signal: round-trip block-size check on extended host matrix (one steady block only).
TEST_P(ExtendedHostMatrixTest, RoundTrip_BlockSize_FirstSteadyBlock)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    assertBlockSizePreserved(chain, cfg.hostBlock, 1);
}

// Signal: long-run silence blocks; host in/out sample totals should match within ratio tolerance.
TEST_P(RoundTripCiTest, LongRun_SampleCountConservation)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    const auto counts = runRoundTripLongRunSampleCount(chain, kLongRunBlocks);
    const double effectiveRatio = static_cast<double>(counts.outputTotal) / static_cast<double>(counts.inputTotal);
    const int tol = longRunTolerance(effectiveRatio);
    EXPECT_LE(std::abs(counts.outputTotal - counts.inputTotal), tol)
        << "host in/out should match within tolerance; hostSR=" << cfg.hostSR;

    assertRoundTripFiniteOutput(chain);
}

// Signal: silence after pre-roll → near-zero round-trip output (DC leak guard).
TEST_P(RoundTripCiTest, SilenceIn_SilenceOut_AfterPreRoll)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    assertRoundTripSilenceOut(chain, cfg.hostBlock);
}

INSTANTIATE_TEST_SUITE_P(RoundTripChainContract, RoundTripChainContractTest,
                         hostConfigValues(defaultCiHostConfigs()), hostConfigName);
INSTANTIATE_TEST_SUITE_P(ProductionChainContract, ProductionChainContractTest,
                         hostConfigValues(defaultCiHostConfigs()), hostConfigName);
INSTANTIATE_TEST_SUITE_P(Production48kSignalChainContract, Production48kSignalContractTest,
                         hostConfigValues(productionSignalContractConfigs()), hostConfigName);
INSTANTIATE_TEST_SUITE_P(RoundTripCi, RoundTripCiTest,
                         hostConfigValues(defaultCiHostConfigs()), hostConfigName);
INSTANTIATE_TEST_SUITE_P(ExtendedHostMatrix, ExtendedHostMatrixTest,
                         hostConfigValues(extendedHostMatrixConfigs()), hostConfigName);
