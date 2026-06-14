#pragma once

/// @file TestInfrastructure.h
/// @brief Compatibility shim for legacy `resampling_test` namespace.
///
/// New code should include targeted headers (`JuceFixture.h`, `ResamplingTopology.h`,
/// etc.) and use `scyclone::test` / `scyclone::test::resampling` directly.
/// This header re-exports symbols so existing `.cpp` files can keep
/// `using namespace resampling_test`.

#include "JuceFixture.h"
#include "LongRun.h"
#include "ResamplingTopology.h"

namespace resampling_test
{
    using namespace scyclone::test;
    using namespace scyclone::test::resampling;
} // namespace resampling_test
