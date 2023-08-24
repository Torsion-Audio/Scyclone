//
// Created by valentin.ackva on 24.02.2023.
//

#ifndef VAESYNTH_HEADERCOMPONENT_H
#define VAESYNTH_HEADERCOMPONENT_H

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

    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;

    std::unique_ptr<juce::Drawable> detailsButtonOn = juce::Drawable::createFromImageData (BinaryData::detailButtonOn_svg,
                                                                                     BinaryData::detailButtonOn_svgSize);
    std::unique_ptr<juce::Drawable> detailsButtonOff = juce::Drawable::createFromImageData (BinaryData::detailButtonOff_svg,
                                                                                      BinaryData::detailButtonOff_svgSize);
    std::unique_ptr<juce::Drawable> scycloneLogo = juce::Drawable::createFromImageData(BinaryData::social_preview_svg,
                                                                                       BinaryData::social_preview_svgSize);
    std::unique_ptr<juce::Drawable> scycloneTypo = juce::Drawable::createFromImageData(BinaryData::Scyclone_svg,
                                                                                       BinaryData::Scyclone_svgSize);
    std::unique_ptr<juce::Drawable> neuralTransferTypo = juce::Drawable::createFromImageData(BinaryData::NeuralTransfer_svg,
                                                                                             BinaryData::NeuralTransfer_svgSize);

    CustomLinearVolumeSliderLookAndFeel customLinearVolumeSliderLookAndFeel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;

    juce::Rectangle<float> scycloneLogoSection;
    juce::Rectangle<float> scycloneTypoSection;
    juce::Rectangle<float> neuralTransferTypoSection;


    struct {
        juce::Label vaeSynth;
        juce::Label neutralTransfer;
        juce::Label inputGainLabel;
        juce::Label outputGainLabel;
    } labels;

    juce::Component* componentArray[4];
};

#endif //VAESYNTH_HEADERCOMPONENT_H
