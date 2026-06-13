//
// Created by schee on 24/05/2023.
//

#include "FooterComponent.h"
#include "BuildInfo.h"

FooterComponent::FooterComponent(AudioPluginAudioProcessor &p, juce::AudioProcessorValueTreeState &parameters) : processor(p), parameters(parameters)
{
    updateSpecs();
    startTimerHz(1);
    setLookAndFeel(&customFontLookAndFeel);
    // font = CustomFontLookAndFeel::getCustomFont();

    // cpuLabel.setFont(font);
    statusLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(statusLabel);

    // tooltipLabel.setFont(font);
    tooltipLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(tooltipLabel);
}

FooterComponent::~FooterComponent()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void FooterComponent::resized()
{
    auto r = getLocalBounds();

    r.removeFromRight(8);
    r.removeFromLeft(8);
    r.removeFromBottom(5);

    r.removeFromLeft(35);
    auto tooltipSection = r.removeFromLeft(500);
    tooltipLabel.setBounds(tooltipSection);

    statusLabel.setBounds(r);
    statusLabel.setJustificationType(juce::Justification::right);
}

void FooterComponent::paint(juce::Graphics &)
{
}

void FooterComponent::updateSpecs()
{
    latencySamples = processor.getLatencySamples();
    sampleRate = static_cast<int>(processor.getSampleRate());
    latencySeconds = (float)latencySamples / float(sampleRate);

    processorUse = processor.getCpuLoad();

    const auto statusText = juce::String("Latency: ") + juce::String((int)(latencySeconds * 1000)) + " ms | CPU: "
                          + juce::String((int)processorUse) + "% | " + BuildInfo::commitHash + " \u2022 "
                          + BuildInfo::commitDate;
    statusLabel.setText(statusText, juce::dontSendNotification);
}

void FooterComponent::timerCallback()
{
    updateSpecs();
}

void FooterComponent::setTooltipText(juce::String newText)
{
    tooltipLabel.setText(newText, juce::dontSendNotification);
}
