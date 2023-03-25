
//
// Created by valentin.ackva on 31.01.2023.
//

#ifndef VAESYNTH_PARAMETERCONTROL_H
#define VAESYNTH_PARAMETERCONTROL_H

#include "JuceHeader.h"
#include "Slider/CustomSliderComponent.h"
#include "../../../PluginParameters.h"
#include "../../LookAndFeel/CustomLabelLookAndFeel.h"
class ParameterControl : public juce::Component {
public:
    explicit ParameterControl(juce::AudioProcessorValueTreeState& parameters);
    void resized() override;
    void paint (juce::Graphics&) override;
    void parameterChanged(const juce::String &parameterID, float newValue);
    void handleNetworkEnablementChange();

private:
    CustomSliderComponent fadeSlider {"Fade", crossfade};
    CustomSliderComponent dynamicSlider {"Dynamic"};
    CustomSliderComponent mixSlider {"Mix"};

    juce::AudioProcessorValueTreeState& parameters;
};


#endif //VAESYNTH_PARAMETERCONTROL_H
