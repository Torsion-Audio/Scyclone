//
// Created by valentin.ackva on 22.02.2023.
//

#ifndef VAESYNTH_AUDIOVISUALISER_H
#define VAESYNTH_AUDIOVISUALISER_H

#include "JuceHeader.h"

class AudioVisualiser {
public:
    AudioVisualiser();

    void prepare(const juce::dsp::ProcessSpec &spec);
    void processSample(juce::AudioBuffer<float> &buffer1, juce::AudioBuffer<float> &buffer2);

    juce::AudioVisualiserComponent& getAudioVisualiser(int id);

private:
    juce::AudioVisualiserComponent audioVisualiserComponent1;
    juce::AudioVisualiserComponent audioVisualiserComponent2;
};

#endif //VAESYNTH_AUDIOVISUALISER_H
