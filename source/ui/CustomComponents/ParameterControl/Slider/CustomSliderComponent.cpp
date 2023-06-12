//
// Created by valentin.ackva on 31.01.2023.
//

#include "CustomSliderComponent.h"

CustomSliderComponent::CustomSliderComponent(juce::String sliderName, CustomSliderType type) : sliderType(type)
{
    setName("Custom Slider Component");

    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(CustomFontLookAndFeel::getCustomFont());
    titleLabel.setText(sliderName, juce::dontSendNotification);
    titleLabel.setColour(juce::Label::ColourIds::textColourId, textColour);
    addAndMakeVisible(titleLabel);

    if (type == normal)
        slider.setLookAndFeel(&customSliderLookAndFeel);
    else if (type == crossfade)
        slider.setLookAndFeel(&customCrossfadeSliderLookAndFeel);

    slider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    slider.hideTextBox(true);
    slider.setTextBoxIsEditable(false);
    slider.onValueChange = [this] { this->sliderChanged();  };
    slider.openManualValueBox = [this] {    this->openManualValueBox(); };
    addAndMakeVisible(slider);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::ColourIds::textColourId, valueColour);
    valueLabel.setInterceptsMouseClicks(false, false);
    valueLabel.onEditorHide = [this] {
        valueLabel.setInterceptsMouseClicks(false, false);
    };
    valueLabel.onTextChange = [this] { manuelValueBoxChanged(); };
    addAndMakeVisible(valueLabel);
    valueLabel.addMouseListener(this, false);

    componentArray[0] = slider.getChildComponent(0);
    componentArray[1] = &titleLabel;
    componentArray[2] = &valueLabel;
}

CustomSliderComponent::~CustomSliderComponent() {
    setLookAndFeel(nullptr);
}

void CustomSliderComponent::addSliderAttachment(juce::AudioProcessorValueTreeState &state, const juce::String &parameterID) {
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, parameterID, slider);
    juce::ignoreUnused(attachment);
    parameter = std::make_unique<juce::RangedAudioParameter*>(state.getParameter(parameterID));

    state.getParameter(parameterID)->getNormalisableRange();

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
    titleLabel.setFont(titleLabel.getFont().withHeight(fontSize));

    const auto areaTitleLabel = area.removeFromTop(headerHeight);

    auto sliderArea = area;
    sliderArea.removeFromLeft(10);
    sliderArea.removeFromRight(10);
    const auto areaSlider = sliderArea;
    const auto areaValueLabel = sliderArea.removeFromBottom(headerHeight);

    titleLabel.setBounds(areaTitleLabel);
    valueLabel.setBounds(areaValueLabel);
    slider.setBounds(areaSlider);
}

void CustomSliderComponent::setCustomColour(CustomSliderColourID colourId, juce::Colour colour) {
    if (sliderType == normal) {
        customSliderLookAndFeel.setCustomColour(colourId, colour);
        slider.setLookAndFeel(&customSliderLookAndFeel);
    }
    else{
        customCrossfadeSliderLookAndFeel.setCustomColour(colourId, colour);
        slider.setLookAndFeel(&customCrossfadeSliderLookAndFeel);
    }
}

void CustomSliderComponent::setDoubleClickReturnValue(double valueToSetOnDoubleClick) {
    slider.setDoubleClickReturnValue(true, valueToSetOnDoubleClick);
}

