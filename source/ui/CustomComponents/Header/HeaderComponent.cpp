//
// Created by valentin.ackva on 16.03.2023.
//

#include "HeaderComponent.h"

HeaderComponent::HeaderComponent(AudioPluginAudioProcessor &p, juce::AudioProcessorValueTreeState &parameters) : detailButton("detailButton",
                                                                                                                              juce::DrawableButton::ButtonStyle::ImageFitted),
                                                                                                                 apvts(parameters),
                                                                                                                 audioProcessor(p){

    labels.vaeSynth.setText("Scyclone", juce::dontSendNotification);
    labels.vaeSynth.setFont(CustomFontLookAndFeel::getCustomFontBold().withHeight(30.f));
    labels.vaeSynth.setJustificationType(juce::Justification::centred);
    labels.vaeSynth.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::WHITE));
    //addAndMakeVisible(labels.vaeSynth);

    labels.neutralTransfer.setText("Neural Transfer", juce::dontSendNotification);
    labels.neutralTransfer.setFont(CustomFontLookAndFeel::getCustomFont().withHeight(19.f));
    labels.neutralTransfer.setJustificationType(juce::Justification::centred);
    labels.neutralTransfer.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    //addAndMakeVisible(labels.neutralTransfer);

    labels.inputGainLabel.setText("Input Gain", juce::dontSendNotification);
    labels.inputGainLabel.setFont(CustomFontLookAndFeel::getCustomFont().withHeight(15.f));
    labels.inputGainLabel.setJustificationType(juce::Justification::centredRight);
    labels.inputGainLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(labels.inputGainLabel);

    inputGainSlider.setSliderStyle(juce::Slider::LinearBar);
    inputGainSlider.setLookAndFeel(&customLinearVolumeSliderLookAndFeel);
    inputGainSlider.setTextValueSuffix(" " + parameters.getParameter(PluginParameters::INPUT_GAIN_ID)->getLabel());
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, PluginParameters::INPUT_GAIN_ID, inputGainSlider);
    inputGainSlider.setTextBoxIsEditable(false);
    addAndMakeVisible(inputGainSlider);

    labels.outputGainLabel.setText("Output Gain", juce::dontSendNotification);
    labels.outputGainLabel.setFont(CustomFontLookAndFeel::getCustomFont().withHeight(15.f));
    labels.outputGainLabel.setJustificationType(juce::Justification::centredRight);
    labels.outputGainLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(labels.outputGainLabel);

    outputGainSlider.setSliderStyle(juce::Slider::LinearBar);
    outputGainSlider.setLookAndFeel(&customLinearVolumeSliderLookAndFeel);
    outputGainSlider.setTextValueSuffix(" " + parameters.getParameter(PluginParameters::OUTPUT_GAIN_ID)->getLabel());
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, PluginParameters::OUTPUT_GAIN_ID, outputGainSlider);
    outputGainSlider.setTextBoxIsEditable(false);
    addAndMakeVisible(outputGainSlider);

    detailButton.setClickingTogglesState(true);
    detailButton.setImages(detailsButtonOff.get(),
                           detailsButtonOff.get(),
                           detailsButtonOn.get(),
                           detailsButtonOff.get(),
                           detailsButtonOn.get(),
                           detailsButtonOn.get(),
                           detailsButtonOn.get(),
                           detailsButtonOn.get());
    addAndMakeVisible(detailButton);
    detailButton.onClick = [this] () {
        //temporary bug fix - otherwise shape distortion
        outputGainSlider.repaint();
        inputGainSlider.repaint();
    };
    detailButton.onStateChange = [this](){
        bool buttonDown = detailButton.getToggleState();
        onParameterControlViewChange(buttonDown);
        audioProcessor.advancedParameterControlVisible = buttonDown;
        //audioProcessor.onUpdateUnautomatableParameters();
    };

    detailButton.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    this->setInterceptsMouseClicks(true, true);
}

HeaderComponent::~HeaderComponent() {
    inputGainSlider.setLookAndFeel(nullptr);
    outputGainSlider.setLookAndFeel(nullptr);
}

void HeaderComponent::resized() {
    labels.vaeSynth.setBounds(49, 21, 127, 30);
    labels.neutralTransfer.setBounds(208, 29, 121, 19);
    labels.inputGainLabel.setBounds(950, 21, 95, 24);
    inputGainSlider.setBounds(1050, 21, 73, 24);
    labels.outputGainLabel.setBounds(1125, 21, 95, 24);
    outputGainSlider.setBounds(1225, 21, 73, 24);
    detailButton.setBounds(getWidth() - 80, 24, 35, 19);
    scycloneTypoSection.setBounds(49.f, 21.f, 127.f, 30.f);
    neuralTransferTypoSection.setBounds(197.f, 21.f, 121.f, 30.f);
}

void HeaderComponent::paint(juce::Graphics &g) {
    //scycloneLogo1->drawAt(g, getWidth() - (int)((float)scycloneLogo1->getWidth())/2, 21, 100);
    scycloneTypo->drawWithin(g, scycloneTypoSection, juce::RectanglePlacement::yBottom, 100);
    neuralTransferTypo->drawWithin(g, neuralTransferTypoSection, juce::RectanglePlacement::yBottom, 100);
}
