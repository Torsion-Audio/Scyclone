// Dry/wet dirac alignment — user-audible time alignment contract.
// Why: PluginProcessor sets wet latency from reported total; dry and wet must peak together.

#include <gtest/gtest.h>
#include "ResamplingHelpers.h"

using namespace resampling_test;

// Signal: dirac at block center (dry) + full chain (wet) → DryWetMixer; mixed peak at diracPos + totalLatency.
TEST_P(DryWetHostConfigTest, DryWet_DiracPeakAlignedAtTotalLatency)
{
    const auto &cfg = GetParam();
    skipIfInfeasible(cfg);
    assertDryWetDiracAligned(cfg.hostSR, static_cast<uint32_t>(cfg.hostBlock));
}

INSTANTIATE_TEST_SUITE_P(DryWetAlignment, DryWetHostConfigTest, hostConfigValues(dryWetHostConfigs()), hostConfigName);
