//
// Created by valentin.ackva on 16.03.2023.
//

#include "AudioVisualiser.h"

AudioVisualiser::AudioVisualiser() : audioVisualiserComponent1(1), audioVisualiserComponent2(1) {

    audioVisualiserComponent1.setRepaintRate(30);
    audioVisualiserComponent2.setRepaintRate(30);
    audioVisualiserComponent1.setSamplesPerBlock(256);
    audioVisualiserComponent2.setSamplesPerBlock(256);
    audioVisualiserComponent1.setBufferSize(512);
    audioVisualiserComponent2.setBufferSize(512);
}

void AudioVisualiser::prepare(const juce::dsp::ProcessSpec &spec) {
    juce::ignoreUnused(spec);
}

void AudioVisualiser::processSample(juce::AudioBuffer<float> &buffer1, juce::AudioBuffer<float> &buffer2) {
    for (int channel = 0; channel < buffer1.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer1.getNumSamples(); ++sample) {
                float sampleToCheck = buffer1.getSample(channel, sample);
                if (isnan(sampleToCheck)) {
                    return;
                }
            }
        }

        for (int channel = 0; channel < buffer2.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer2.getNumSamples(); ++sample) {
                float sampleToCheck = buffer2.getSample(channel, sample);
                if (isnan(sampleToCheck)) {
                    return;
                }
            }
        }

        audioVisualiserComponent1.pushBuffer(buffer1.getArrayOfReadPointers(), 1, buffer1.getNumSamples());
        audioVisualiserComponent2.pushBuffer(buffer2.getArrayOfReadPointers(), 1, buffer2.getNumSamples());
}

juce::AudioVisualiserComponent &AudioVisualiser::getAudioVisualiser(int id) {
    if (id == 1) {
        return audioVisualiserComponent1;
    } else {
        return audioVisualiserComponent2;
    }
}
