//
// Created by valentin.ackva on 31.01.2023.
//

#include "ParameterControl.h"

ParameterControl::ParameterControl(juce::AudioProcessorValueTreeState &parameters) : parameters(parameters) {
    addAndMakeVisible(fadeSlider);
    fadeSlider.setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colour {0xff004E92});
    fadeSlider.setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colour {0xffEB1E79} );
    fadeSlider.addSliderAttachment(parameters, PluginParameters::FADE_ID.getParamID());
    fadeSlider.setDoubleClickReturnValue(0.0);

    addAndMakeVisible(dynamicSlider);
    dynamicSlider.addSliderAttachment(parameters, PluginParameters::COMP_DRY_WET_ID.getParamID());
    dynamicSlider.setDoubleClickReturnValue(0.0);

    addAndMakeVisible(mixSlider);
    mixSlider.addSliderAttachment(parameters, PluginParameters::DRY_WET_ID.getParamID());
    //mixSlider.setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colour {0xffF6EFE8});
    //mixSlider.setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colour {0xff00467F} );
    mixSlider.setDoubleClickReturnValue(10.0);

    parameterChanged(PluginParameters::ON_OFF_NETWORK1_ID.getParamID(), parameters.getParameterAsValue(PluginParameters::ON_OFF_NETWORK1_ID.getParamID()).getValue());
    parameterChanged(PluginParameters::ON_OFF_NETWORK2_ID.getParamID(), parameters.getParameterAsValue(PluginParameters::ON_OFF_NETWORK2_ID.getParamID()).getValue());
}

void ParameterControl::resized() {
    //~700x700
    fadeSlider.setBounds(105, 50, 120, 500);
    dynamicSlider.setBounds(255, 95, 120, 500);
    mixSlider.setBounds(405, 40, 120, 500);
}

void ParameterControl::paint(juce::Graphics &) {
    //handleNetworkEnablementChange(); Greys fade slider out when one network is disabled. This functionality is currently disabled and might be enabled and brought to other sliders as well in the future...
    //fadeSlider.repaint();
}

void ParameterControl::parameterChanged(const juce::String &parameterID, float newValue) {
    juce::ignoreUnused(parameterID, newValue);
    /*
     *  Greys fade slider out when one network is disabled. This functionality is currently disabled and might be enabled and brought to other sliders as well in the future...
     *
    if (parameterID == PluginParameters::ON_OFF_NETWORK1_ID || parameterID == PluginParameters::ON_OFF_NETWORK2_ID) {
        handleNetworkEnablementChange();
    }
     */
}

void ParameterControl::handleNetworkEnablementChange() {
    bool network1Enablement = parameters.getParameterAsValue(PluginParameters::ON_OFF_NETWORK1_ID.getParamID()).getValue();
    bool network2Enablement = parameters.getParameterAsValue(PluginParameters::ON_OFF_NETWORK2_ID.getParamID()).getValue();

    if (network1Enablement && network2Enablement){
        fadeSlider.setEnabled(true);
        fadeSlider.setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colour {0xff004E92});
        fadeSlider.setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colour {0xffEB1E79} );
    }
    else {
        fadeSlider.setEnabled(false);
        fadeSlider.setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colours::grey);
        fadeSlider.setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colours::grey);
    }
}
