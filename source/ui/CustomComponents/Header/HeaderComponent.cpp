#include "HeaderComponent.h"

HeaderComponent::HeaderComponent(AudioPluginAudioProcessor &p, juce::AudioProcessorValueTreeState &parameters)
        : detailButton("detailButton", juce::DrawableButton::ButtonStyle::ImageFitted),
          scycloneButton("scycloneButton", juce::DrawableButton::ButtonStyle::ImageFitted),
          apvts(parameters),
          audioProcessor(p)
{
    setupLabel(labels.vaeSynth, "Scyclone",
               30.f,
               ColorPallete::WHITE,
               juce::Justification::centred);
    //addAndMakeVisible(labels.vaeSynth);

    setupLabel(labels.neutralTransfer,
               "Neural Transfer",
               19.f,
               ColorPallete::TEXT2,
               juce::Justification::centred);
    //addAndMakeVisible(labels.neutralTransfer);

    setupLabel(labels.inputGainLabel,
               "Input Gain",
               15.f, ColorPallete::
               TEXT2,
               juce::Justification::centredRight);

    setupSlider(inputGainSlider,
                PluginParameters::INPUT_GAIN_ID,
                parameters,
                inputGainAttachment);

    setupLabel(labels.outputGainLabel,
               "Output Gain", 15.f,
               ColorPallete::TEXT2,
               juce::Justification::centredRight);

    setupSlider(outputGainSlider,
                PluginParameters::OUTPUT_GAIN_ID,
                parameters,
                outputGainAttachment);

    setupDetailButton();

    setupScycloneButton();

    this->setInterceptsMouseClicks(true, true);

    setupComponentArray();
}

HeaderComponent::~HeaderComponent() {
    inputGainSlider.setLookAndFeel(nullptr);
    outputGainSlider.setLookAndFeel(nullptr);
}

void HeaderComponent::setupComponentArray() {
    componentArray[0] = inputGainSlider.getChildComponent(0);
    componentArray[1] = outputGainSlider.getChildComponent(0);
    componentArray[2] = &detailButton;
    componentArray[3] = &scycloneButton;
}

void HeaderComponent::setupLabel(juce::Label& label, const juce::String& text, float fontSize, const juce::String& color, juce::Justification justification) {
    label.setText(text, juce::dontSendNotification);
    label.setFont(getFont(FontType::bold, fontSize));
    label.setJustificationType(justification);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(color));
}

void HeaderComponent::setupSlider(juce::Slider& slider, const juce::ParameterID& paramID, juce::AudioProcessorValueTreeState& parameters, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment) {
    slider.setSliderStyle(juce::Slider::LinearBar);
    slider.setLookAndFeel(&customLinearVolumeSliderLookAndFeel);
    slider.setTextValueSuffix(" " + parameters.getParameter(paramID.getParamID())->getLabel());
    slider.setTextBoxIsEditable(false);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, paramID.getParamID(), slider);
}

void HeaderComponent::setupDetailButton() {
    detailButton.setClickingTogglesState(true);
    detailButton.setImages(detailsButtonOff.get(), detailsButtonOff.get(), detailsButtonOn.get(), detailsButtonOff.get(), detailsButtonOn.get(), detailsButtonOn.get(), detailsButtonOn.get(), detailsButtonOn.get());
    detailButton.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);

    detailButton.onClick = [this]() {
        inputGainSlider.repaint();
        outputGainSlider.repaint();
    };

    detailButton.onStateChange = [this]() {
        bool buttonDown = detailButton.getToggleState();
        onParameterControlViewChange(buttonDown);
        audioProcessor.advancedParameterControlVisible = buttonDown;
    };
}

void HeaderComponent::setupScycloneButton() {
    scycloneButton.setClickingTogglesState(true);

    scycloneButton.setImages(scycloneLogo.get(), scycloneLogoOver.get(), scycloneLogo.get(), scycloneLogo.get(), scycloneLogo.get(), scycloneLogo.get());
    scycloneButton.setHasFocusOutline(false);
    scycloneButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentWhite);
    scycloneButton.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentWhite);

    scycloneButton.onClick = [this]() {
        onScyloneButtonClick(scycloneButton.getToggleState());
    };
}

void HeaderComponent::defineLayout() {
    layout.add(labels.vaeSynth, 49, 21, 127, 30, FontType::bold, 30.f);
    layout.add(labels.neutralTransfer, 208, 29, 121, 19, FontType::bold, 19.f);
    layout.add(labels.inputGainLabel, 950, 21, 95, 24, FontType::bold, 15.f);
    layout.add(inputGainSlider, 1050, 21, 73, 24);
    layout.add(labels.outputGainLabel, 1125, 21, 95, 24, FontType::bold, 15.f);
    layout.add(outputGainSlider, 1225, 21, 73, 24);
    layout.add(detailButton, 1320, 24, 35, 19);
    layout.add(scycloneButton, 627.5f, 21, 145, 30);

    labels.vaeSynth.setVisible(false);
    labels.neutralTransfer.setVisible(false);
}

void HeaderComponent::scaleChanged(float scale) {
    scycloneTypoSection.setBounds(49.f * scale, 21.f * scale, 127.f * scale, 30.f * scale);
    neuralTransferTypoSection.setBounds(197.f * scale, 21.f * scale, 121.f * scale, 30.f * scale);
}

void HeaderComponent::paint(juce::Graphics &g) {
    scycloneTypo->drawWithin(g, scycloneTypoSection, juce::RectanglePlacement::yBottom, 100);
    neuralTransferTypo->drawWithin(g, neuralTransferTypoSection, juce::RectanglePlacement::yBottom, 100);
}

juce::Component** HeaderComponent::getTooltipPointers() {
    return componentArray;
}
