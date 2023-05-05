//
// Created by valentin.ackva on 16.03.2023.
//

#include "ProcessorGain.h"

void ProcessorGain::processInputBlock(juce::AudioBuffer<float> &buffer) {
    buffer.applyGainRamp(0, buffer.getNumSamples(), inputGain.previousGain, inputGain.currentGain);
    inputGain.previousGain = inputGain.currentGain;
}

void ProcessorGain::processOutputBlock(juce::AudioBuffer<float> &buffer) {
    buffer.applyGainRamp(0, buffer.getNumSamples(), outputGain.previousGain, outputGain.currentGain);
    outputGain.previousGain = outputGain.currentGain;
}

void ProcessorGain::parameterChanged(const juce::String &parameterID, float newValue) {
    if (parameterID == PluginParameters::INPUT_GAIN_ID.getParamID()) {
        inputGain.currentGain = juce::Decibels::decibelsToGain(newValue);
    } else if (parameterID == PluginParameters::OUTPUT_GAIN_ID.getParamID()) {
        outputGain.currentGain = juce::Decibels::decibelsToGain(newValue);
    }
}
