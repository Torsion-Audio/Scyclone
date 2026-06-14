#pragma once

/// @file HostConfigFixtures.h
/// @brief Parameterized gtest fixtures over HostConfig.
///
/// Base fixture for host SR/block matrices; domain tests inherit and add params.
///
/// @namespace torsion::test

#include <gtest/gtest.h>

#include "HostConfig.h"
#include "JuceFixture.h"

namespace torsion::test
{

    class HostConfigParamTest : public JuceAudioTest,
                                public ::testing::WithParamInterface<HostConfig>
    {
    };

} // namespace torsion::test
