//
// Created by valentin.ackva on 31.01.2023.
//

#include "CustomSliderComponent.h"

CustomSliderComponent::CustomSliderComponent(juce::String sliderName, CustomSliderType type) : sliderType(type)
{

    titelLabel.setJustificationType(juce::Justification::centred);
    titelLabel.setFont(CustomFontLookAndFeel::getCustomFont());
    titelLabel.setText(sliderName, juce::dontSendNotification);
    titelLabel.setColour(juce::Label::ColourIds::textColourId, textColour);
    addAndMakeVisible(titelLabel);

    if (type == normal)
        slider.setLookAndFeel(&customSliderLookAndFeel);
    else if (type == crossfade)
        slider.setLookAndFeel(&customCrossfadeSliderLookAndFeel);

    slider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    slider.hideTextBox(true);
    slider.setTextBoxIsEditable(false);
    slider.onValueChange = [this] {this->sliderChanged();};
    slider.openManualValueBox = [this] {this->openManualValueBox();};
    addAndMakeVisible(slider);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::ColourIds::textColourId, valueColour);
    valueLabel.setInterceptsMouseClicks(false, false);
    valueLabel.onEditorHide = [this] {
        valueLabel.setInterceptsMouseClicks(false, false);
    };
    valueLabel.onTextChange = [this] { manuelValueBoxChanged(); };
    addAndMakeVisible(valueLabel);
}

CustomSliderComponent::~CustomSliderComponent() {
    slider.setLookAndFeel(nullptr);
}

void CustomSliderComponent::addSliderAttachment(juce::AudioProcessorValueTreeState &state, const juce::String &parameterID) {
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, parameterID, slider);
    parameter = std::make_unique<juce::RangedAudioParameter*>(state.getParameter(parameterID));
    updateValueLabel();
}

void CustomSliderComponent::sliderChanged() {
    updateValueLabel();
}

void CustomSliderComponent::openManualValueBox() {
    valueLabel.setInterceptsMouseClicks(true, true);
    //valueLabel.showEditor();
}

void CustomSliderComponent::manuelValueBoxChanged() {
    if (parameter != nullptr) {
        juce::RangedAudioParameter* param = *parameter;
        double newValue = param->getValueForText(valueLabel.getText(true));
        slider.setValue(newValue, juce::sendNotification);
    } else {
        double minValue = slider.getMinimum();
        double maxValue = slider.getMaximum();
        double newValue = static_cast<double>(valueLabel.getText(true).getFloatValue());

        newValue = (newValue > maxValue) ? maxValue : (newValue < minValue) ? minValue : newValue;
        slider.setValue(newValue, juce::sendNotification);
    }
}

void CustomSliderComponent::updateValueLabel() {
    if (parameter != nullptr) {
        juce::RangedAudioParameter* param = *parameter;

        juce::String valueText = param->getCurrentValueAsText() + " " + param->getLabel();
        valueLabel.setText(valueText, juce::dontSendNotification);
    }


}

void CustomSliderComponent::resized() {
    auto area = getLocalBounds();

    float fontSize = (getHeight() < 450) ? 13.5f : 16.5f;
    float fontSizeLabel = (getHeight() < 450) ? 13.5f : 16.5f;

    const int headerHeight = getHeight() / (int) fontSize + 10;

    valueLabel.setFont(valueLabel.getFont().withHeight(fontSizeLabel));
    titelLabel.setFont(titelLabel.getFont().withHeight(fontSize));

    const auto areaTitleLabel = area.removeFromTop(headerHeight);

    auto sliderArea = area;
    sliderArea.removeFromLeft(10);
    sliderArea.removeFromRight(10);
    const auto areaSlider = sliderArea;
    const auto areaValueLabel = sliderArea.removeFromBottom(headerHeight);

    titelLabel.setBounds(areaTitleLabel);
    slider.setBounds(areaSlider);
    valueLabel.setBounds(areaValueLabel);
}

void CustomSliderComponent::setCustomColour(CustomSliderColourID colourId, juce::Colour colour) {
    if (sliderType == normal) {
        customSliderLookAndFeel.setCustomColour(colourId, colour);
    }
    else{
        customCrossfadeSliderLookAndFeel.setCustomColour(colourId, colour);
    }
    slider.repaint();
}

void CustomSliderComponent::setDoubleClickReturnValue(double valueToSetOnDoubleClick) {
    slider.setDoubleClickReturnValue(true, valueToSetOnDoubleClick);
}

