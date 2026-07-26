//
// Created by schee on 24/05/2023.
//

#ifndef SCYCLONE_FOOTERCOMPONENT_H
#define SCYCLONE_FOOTERCOMPONENT_H

#include "JuceHeader.h"
#include "../../../utils/colors.h"
#include "../../Core/BaseComponent.h"
#include "../../../PluginParameters.h"
#include "../../../PluginProcessor.h"
#include "../../../ui/LookAndFeel/CustomFontLookAndFeel.h"

class FooterComponent : public BaseComponent, juce::Timer
{
public:
    FooterComponent(AudioPluginAudioProcessor &p, juce::AudioProcessorValueTreeState &parameters);
    ~FooterComponent();

    void defineLayout() override;
    void paint(juce::Graphics &g) override;
    void timerCallback() override;

    void updateSpecs();

    void setTooltipText(juce::String newText);

private:
    AudioPluginAudioProcessor &processor;
    juce::AudioProcessorValueTreeState &parameters;

    juce::Label tooltipLabel;
    juce::Label statusLabel;

    int latencySamples = 0;
    int sampleRate = 0;
    float latencySeconds = 0.f;
    float processorUse = 0.f;

    CustomFontLookAndFeel customFontLookAndFeel;
    juce::Font font;
};

#endif // SCYCLONE_FOOTERCOMPONENT_H
