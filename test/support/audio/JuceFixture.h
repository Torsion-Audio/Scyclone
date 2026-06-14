#pragma once

/// @file JuceFixture.h
/// @brief Google Test fixture that initialises the JUCE GUI subsystem.
///
/// Inherit from `JuceAudioTest` in any test that touches JUCE audio buffers,
/// `ProcessSpec`, or plugin code. Equivalent to a minimal `juce::UnitTest`
/// environment hook for gtest.
///
/// @namespace scyclone::test

#include <gtest/gtest.h>
#include <JuceHeader.h>

namespace scyclone::test
{

    /// Base fixture: owns `ScopedJuceInitialiser_GUI` for the test lifetime.
    class JuceAudioTest : public ::testing::Test
    {
    protected:
        juce::ScopedJuceInitialiser_GUI juceInit;
    };

} // namespace scyclone::test
