#pragma once

#include "PluginProcessor.h"
#include "utils/colors.h"
#include "PluginParameters.h"
#include "ui/CustomComponents/TransientViewer/TransientViewer.h"
#include "ui/CustomComponents/OpenGLBackground/OpenGLBackground.h"
#include "ui/CustomComponents/ParameterControl/ParameterControl.h"
#include "ui/CustomComponents/AdvancedParameterControl/AdvancedParameterControl.h"
#include "ui/CustomComponents/Header/HeaderComponent.h"
#include "ui/CustomComponents/Footer/FooterComponent.h"
#include "ui/LookAndFeel/CustomFontLookAndFeel.h"
#include "ui/CustomComponents/Texture/TextureComponent.h"

//==============================================================================
class AudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&, juce::AudioProcessorValueTreeState& parameters);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;

private:
    void openFileChooser(int id);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;


    juce::AudioProcessorValueTreeState& apvts;
    TransientViewer transientViewer;
    //OpenGLBackground openGLBackground;
    std::unique_ptr<OpenGLBackground> openGLBackground;
    ParameterControl parameterControl;
    AdvancedParameterControl advancedParameterControl;
    HeaderComponent headerComponent;
    TextureComponent textureComponent;
    FooterComponent footerComponent;

    CustomFontLookAndFeel customFontLookAndFeel;

    std::unique_ptr<juce::FileChooser> fc;

    juce::Component** xyPadComponents;
    juce::Component** parameterControlComponents;
    juce::Component** advancedParameterControlComponents;
    juce::Component** headerComponents;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
