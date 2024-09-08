#include "AudioVisualiser.h"

AudioVisualiser::AudioVisualiser() : audioVisualiserComponent1(1), audioVisualiserComponent2(1) {
    initializeVisualiserComponent(audioVisualiserComponent1);
    initializeVisualiserComponent(audioVisualiserComponent2);
}

void AudioVisualiser::initializeVisualiserComponent(juce::AudioVisualiserComponent& visualiser) {
    visualiser.setRepaintRate(30);
    visualiser.setSamplesPerBlock(256);
    visualiser.setBufferSize(512);
}

void AudioVisualiser::prepare(const juce::dsp::ProcessSpec &spec) {
    juce::ignoreUnused(spec);
}

bool AudioVisualiser::validateBufferForNaN(const juce::AudioBuffer<float>& buffer) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (std::isnan(buffer.getSample(channel, sample))) {
                return false; // Invalid buffer due to NaN values
            }
        }
    }
    return true;
}

void AudioVisualiser::updateFromAudioBuffer(juce::AudioBuffer<float> &buffer1, juce::AudioBuffer<float> &buffer2) {
    if (!validateBufferForNaN(buffer1) || !validateBufferForNaN(buffer2)) {
        return;
    }

    // Push valid buffers to visualizer components
    audioVisualiserComponent1.pushBuffer(buffer1.getArrayOfReadPointers(), 1, buffer1.getNumSamples());
    audioVisualiserComponent2.pushBuffer(buffer2.getArrayOfReadPointers(), 1, buffer2.getNumSamples());
}

juce::AudioVisualiserComponent& AudioVisualiser::getAudioVisualiser(int id) {
    return (id == 1) ? audioVisualiserComponent1 : audioVisualiserComponent2;
}