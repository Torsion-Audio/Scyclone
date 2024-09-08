//
// Created by valentin.ackva on 22.02.2023.
//

#pragma once

#include <JuceHeader.h>

class AudioVisualiser {
public:
    AudioVisualiser();

    void initializeVisualiserComponent(juce::AudioVisualiserComponent& visualiser);

    void prepare(const juce::dsp::ProcessSpec& spec);

    bool validateBufferForNaN(const juce::AudioBuffer<float>& buffer);

    void updateFromAudioBuffer(juce::AudioBuffer<float>& buffer1, juce::AudioBuffer<float>& buffer2);

    juce::AudioVisualiserComponent& getAudioVisualiser(int id);

private:
    juce::AudioVisualiserComponent audioVisualiserComponent1; // Visualiser component 1
    juce::AudioVisualiserComponent audioVisualiserComponent2; // Visualiser component 2
};
