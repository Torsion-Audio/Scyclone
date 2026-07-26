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
    statusLabel.setJustificationType(juce::Justification::right);

    // tooltipLabel.setFont(font);
    tooltipLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
}

FooterComponent::~FooterComponent()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void FooterComponent::defineLayout()
{
    layout.add(tooltipLabel, 43, 0, 500, 30, FontType::regular, 14.f);
    layout.add(statusLabel, 543, 0, 849, 30, FontType::regular, 14.f);
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
