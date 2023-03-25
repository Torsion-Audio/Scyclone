//
// Created by schee on 22/03/2023.
//
#include <JuceHeader.h>
#include "../../../modules/RnboExport/rnbo/RNBO.h"
#include "../../PluginParameters.h"

#ifndef GITMODULES_GRAINDELAY_H
#define GITMODULES_GRAINDELAY_H


class GrainDelay {
public:
    GrainDelay(const int no);
    ~GrainDelay();

    void prepare(const juce::dsp::ProcessSpec &spec);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void parameterChanged(const juce::String &parameterID, float newValue);

    void setParameterValue(std::atomic<float>* parameterToConnect, int rnboParameterIdx);
    void setMuted(bool newState);

private:
    int sampleRate = 48000;
    RNBO::CoreObject rnboObject;
    std::unordered_map<juce::String, RNBO::ParameterIndex> apvtsParamIdToRnboParamIndex;
    bool isMuted = true;
    int number;
};


#endif //GITMODULES_GRAINDELAY_H
