//
// Created by valentin.ackva on 16.03.2023.
//

#ifndef VAESYNTH_PROCESSORGAIN_H
#define VAESYNTH_PROCESSORGAIN_H

#include "JuceHeader.h"
#include "../../PluginParameters.h"

struct GainLevel{
    float currentGain = 1.f;
    float previousGain = 1.f;
};

class ProcessorGain {
public:
    void processInputBlock(juce::AudioBuffer<float>& buffer);
    void processOutputBlock(juce::AudioBuffer<float>& buffer);
    void parameterChanged(const juce::String &parameterID, float newValue);

private:
    GainLevel inputGain, outputGain;
};


#endif //VAESYNTH_PROCESSORGAIN_H
