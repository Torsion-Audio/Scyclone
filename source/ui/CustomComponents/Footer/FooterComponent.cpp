//
// Created by schee on 24/05/2023.
//

#include "FooterComponent.h"

FooterComponent::FooterComponent(AudioPluginAudioProcessor &p, juce::AudioProcessorValueTreeState &parameters) : processor(p), parameters(parameters) {
    updateSpecs();
    startTimerHz(1);
    setLookAndFeel(&customFontLookAndFeel);
    //font = CustomFontLookAndFeel::getCustomFont();

    //cpuLabel.setFont(font);
    cpuLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(cpuLabel);

    //latencyLabel.setFont(font);
    latencyLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(latencyLabel);

    //tooltipLabel.setFont(font);
    tooltipLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(tooltipLabel);
}

FooterComponent::~FooterComponent() {
    stopTimer();
    setLookAndFeel(nullptr);
}


void FooterComponent::resized() {
    auto r = getLocalBounds();

    r.removeFromRight(8);
    r.removeFromLeft(8);
    r.removeFromBottom(5);


    auto cpuSection = r.removeFromRight(130);
    cpuSection.removeFromRight(37);
    cpuLabel.setBounds(cpuSection);
    cpuLabel.setJustificationType(juce::Justification::right);

    auto latencySection = r.removeFromRight(130);
    latencyLabel.setBounds(latencySection);
    latencyLabel.setJustificationType(juce::Justification::right);

    r.removeFromLeft(35);
    auto tooltipSection = r.removeFromLeft(500);
    tooltipLabel.setBounds(tooltipSection);
}

void FooterComponent::paint(juce::Graphics &g) {
}

void FooterComponent::updateSpecs(){
    latencySamples = processor.getLatencySamples();
    sampleRate = processor.getSampleRate();
    latencySeconds = (float)latencySamples / float(sampleRate);

    //processorUse = processor.getCpuLoad();
    processorUse = systemSpecs.getCPULoad();

    std::string cpuString = "CPU: " + std::to_string((int)processorUse ) + " %";
    cpuLabel.setText(cpuString, juce::dontSendNotification);
    std::string latencyString = "Latency: " + std::to_string(int(latencySeconds * 1000)) + " ms";
    latencyLabel.setText(latencyString, juce::dontSendNotification);
}

void FooterComponent::timerCallback() {
    updateSpecs();
}

void FooterComponent::setTooltipText(juce::String newText) {
    tooltipLabel.setText(newText, juce::dontSendNotification);
}

