//
// Created by schee on 22/03/2023.
//
#include <JuceHeader.h>
#include <RNBO.h>
#include "../IProcessor.h"

#ifndef GITMODULES_GRAINDELAY_H
#define GITMODULES_GRAINDELAY_H

class GrainDelay : public IProcessor {
public:
    GrainDelay(const int no);
    ~GrainDelay() override;

    void prepare(const juce::dsp::ProcessSpec &spec) override;
    void processBlock(juce::AudioBuffer<float>& buffer) override;
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
