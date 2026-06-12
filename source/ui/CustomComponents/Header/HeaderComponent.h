//
// Created by valentin.ackva on 24.02.2023.
//

#pragma once

#include "JuceHeader.h"
#include "../../../utils/colors.h"
#include "../../LookAndFeel/SliderLookAndFeel.h"
#include "../../LookAndFeel/CustomFontLookAndFeel.h"
#include "../../../PluginParameters.h"
#include "../../../PluginProcessor.h"

class HeaderComponent : public juce::Component{
public:
    HeaderComponent(AudioPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& parameters);
    ~HeaderComponent();

    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::DrawableButton detailButton;
    juce::DrawableButton scycloneButton;

    std::function<void(bool newState)> onParameterControlViewChange;
    std::function<void(bool newState)> onScyloneButtonClick;

    juce::Component** getTooltipPointers();

private:
    AudioPluginAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& apvts;

    void setupLabel(juce::Label& label, const juce::String& text, float fontSize, const juce::String& color, juce::Justification justification);
    void setupSlider(juce::Slider& slider, const juce::ParameterID& paramID, juce::AudioProcessorValueTreeState& parameters, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment);
    void setupDetailButton();
    void setupScycloneButton();
    void setupComponentArray();

    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    CustomLinearVolumeSliderLookAndFeel customLinearVolumeSliderLookAndFeel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;

    std::unique_ptr<juce::Drawable> detailsButtonOn = juce::Drawable::createFromImageData (BinaryData::HamburgerButtonOn_svg,
                                                                                           BinaryData::HamburgerButtonOn_svgSize);
    std::unique_ptr<juce::Drawable> detailsButtonOff = juce::Drawable::createFromImageData (BinaryData::HamburgerButtonOff_svg,
                                                                                            BinaryData::HamburgerButtonOff_svgSize);

    std::unique_ptr<juce::Drawable> scycloneLogo = juce::Drawable::createFromImageData(BinaryData::TorsionAudio_CombinationMark_svg,
                                                                                       BinaryData::TorsionAudio_CombinationMark_svgSize);
    std::unique_ptr<juce::Drawable> scycloneLogoOver = juce::Drawable::createFromImageData(BinaryData::TorsionAudio_CombinationMark_Hover_svg,
                                                                                           BinaryData::TorsionAudio_CombinationMark_Hover_svgSize);

    std::unique_ptr<juce::Drawable> scycloneTypo = juce::Drawable::createFromImageData(BinaryData::Scyclone_Typo_svg,
                                                                                       BinaryData::Scyclone_Typo_svgSize);

    std::unique_ptr<juce::Drawable> neuralTransferTypo = juce::Drawable::createFromImageData(BinaryData::NeuralTransfer_Typo_svg,
                                                                                             BinaryData::NeuralTransfer_Typo_svgSize);
    struct {
        juce::Label vaeSynth;
        juce::Label neutralTransfer;
        juce::Label inputGainLabel;
        juce::Label outputGainLabel;
    } labels;


    juce::Rectangle<float> scycloneLogoSection;
    juce::Rectangle<float> scycloneTypoSection;
    juce::Rectangle<float> neuralTransferTypoSection;

    juce::Component* componentArray[4]; // TooltipCounts::header — order in TooltipManager::initializeTooltipMap
};
