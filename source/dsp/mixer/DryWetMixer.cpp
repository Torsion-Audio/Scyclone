//
// Created by valentin.ackva on 26.10.2022.
//

#include "DryWetMixer.h"
#include "../../PluginParameters.h"

DryWetMixer::DryWetMixer() {
    mixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    mixer.setWetMixProportion(dryWetProportion);
}

void DryWetMixer::prepare(const juce::dsp::ProcessSpec &spec) {
    mixer.prepare(spec);
    mixer.reset();
}

void DryWetMixer::setDryWetProportion(float dryWet) {
    dryWetProportion = dryWet;
    mixer.setWetMixProportion(dryWetProportion);
}

void DryWetMixer::setDryWetProportionPercentage(float dryWet) {
    setDryWetProportion(dryWet/100);
}

void DryWetMixer::setDrySamples(juce::AudioBuffer<float> &dryBuffer) {
    juce::dsp::AudioBlock<float> input_block(dryBuffer);
    mixer.pushDrySamples(input_block);
}

void DryWetMixer::setWetSamples(juce::AudioBuffer<float> &wetBuffer) {
    mixer.mixWetSamples(wetBuffer);
}

void DryWetMixer::setMixingCurve(juce::dsp::DryWetMixingRule rule) {
    mixer.setMixingRule(rule);
}

void DryWetMixer::setWetLatency(int numberOfSamples) {
    mixer.setWetLatency((float) numberOfSamples);
}

void DryWetMixer::parameterChanged(const juce::String &parameterID, float newValue) {
    juce::ignoreUnused(parameterID);
    setDryWetProportion(newValue);
}

