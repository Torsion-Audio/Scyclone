//
// Created by valentin.ackva on 26.10.2022.
//

#ifndef AUDIO_PLUGIN_EXAMPLE_DRYWETMIXER_H
#define AUDIO_PLUGIN_EXAMPLE_DRYWETMIXER_H

#include <JuceHeader.h>

class DryWetMixer {
public:
    DryWetMixer();
    ~DryWetMixer() = default;
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setDryWetProportion(float dryWet);
    void setDryWetProportionPercentage (float dryWet);
    void setMixingCurve(juce::dsp::DryWetMixingRule rule);
    void setDrySamples(juce::AudioBuffer<float>& dryBuffer);
    void setWetSamples(juce::AudioBuffer<float>& wetBuffer);
    void setWetLatency(int numberOfSamples);
    void parameterChanged(const juce::String &parameterID, float newValue);


private:
    juce::dsp::DryWetMixer<float> mixer {48000};
    float dryWetProportion = 0.0f;
};


#endif //AUDIO_PLUGIN_EXAMPLE_DRYWETMIXER_H
