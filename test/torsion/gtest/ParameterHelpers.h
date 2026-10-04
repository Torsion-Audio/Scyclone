#pragma once

/// @file ParameterHelpers.h
/// @brief Drive a processor's parameters by ID the way a host would.
///
/// @namespace torsion::test

#include <gtest/gtest.h>

#include "JuceHeader.h"

namespace torsion::test
{

    /// Sets the parameter with @p paramID to @p normalisedValue through setValueNotifyingHost,
    /// so the processor's listeners fire exactly as for host automation.
    inline void setParameterById(juce::AudioProcessor &processor,
                                 const juce::String &paramID,
                                 float normalisedValue)
    {
        for (auto *parameter : processor.getParameters())
        {
            if (auto *withID = dynamic_cast<juce::AudioProcessorParameterWithID *>(parameter);
                withID != nullptr && withID->paramID == paramID)
            {
                withID->setValueNotifyingHost(normalisedValue);
                return;
            }
        }
        ADD_FAILURE() << "no parameter with ID " << paramID;
    }

    inline void setBoolParameterById(juce::AudioProcessor &processor, const juce::String &paramID, bool on)
    {
        setParameterById(processor, paramID, on ? 1.0f : 0.0f);
    }

} // namespace torsion::test
