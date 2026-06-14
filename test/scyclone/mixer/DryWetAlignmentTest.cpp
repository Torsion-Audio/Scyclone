// Dry/wet dirac alignment — user-audible time alignment contract.
// Why: PluginProcessor sets wet latency from reported total; dry and wet must peak together.

#include <gtest/gtest.h>
#include "DryWetContract.h"
#include "HostConfigCatalog.h"
#include "ResamplingTopology.h"
#include "ScycloneHostPresets.h"

using namespace scyclone::test::mixer;
using namespace scyclone::test::resampling;

TEST_P(DryWetHostConfigTest, DryWet_DiracPeakAlignedAtTotalLatency)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    assertDryWetDiracAligned(cfg.hostSR, static_cast<uint32_t>(cfg.hostBlock));
}

// Wet path only: reported total latency must match measured group delay (no mixer compensation).
TEST_P(DryWet48kSignalConfigTest, WetPath_DiracPeakMatchesReportedLatency)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    assertProductionWetPeakMatchesReportedLatency(cfg.hostSR, static_cast<uint32_t>(cfg.hostBlock));
}

INSTANTIATE_TEST_SUITE_P(DryWetAlignment, DryWetHostConfigTest,
                         torsion::test::hostConfigValues(dryWetHostConfigs()),
                         torsion::test::hostConfigName);

INSTANTIATE_TEST_SUITE_P(DryWetWetPath, DryWet48kSignalConfigTest,
                         torsion::test::hostConfigValues(productionSignalContractConfigs()),
                         torsion::test::hostConfigName);
