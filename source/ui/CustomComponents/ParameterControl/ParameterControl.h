
//
// Created by valentin.ackva on 31.01.2023.
//

#ifndef VAESYNTH_PARAMETERCONTROL_H
#define VAESYNTH_PARAMETERCONTROL_H

#include "JuceHeader.h"
#include "Slider/CustomSliderComponent.h"
#include "../../Core/BaseComponent.h"
#include "../../../PluginParameters.h"
#include "../../LookAndFeel/CustomLabelLookAndFeel.h"
class ParameterControl : public BaseComponent {
public:
    explicit ParameterControl(juce::AudioProcessorValueTreeState& parameters);
    void defineLayout() override;
    void paint (juce::Graphics&) override;
    void parameterChanged(const juce::String &parameterID, float newValue);
    void handleNetworkEnablementChange();

    juce::Component** getTooltipPointers();

private:
    CustomSliderComponent fadeSlider {"Fade", crossfade};
    CustomSliderComponent dynamicSlider {"Dynamic"};
    CustomSliderComponent mixSlider {"Mix"};

    juce::AudioProcessorValueTreeState& parameters;

    juce::Component* componentArray[9];
};


#endif //VAESYNTH_PARAMETERCONTROL_H
