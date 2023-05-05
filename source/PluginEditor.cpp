#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginParameters.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& parameters)
    : AudioProcessorEditor (&p), apvts(parameters), processorRef (p), transientViewer(p), openGLBackground(parameters, p), advancedParameterControl(parameters), parameterControl(parameters)
{
    juce::ignoreUnused (processorRef);

    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        parameters.addParameterListener(parameterID, this);
    }

    juce::LookAndFeel::setDefaultLookAndFeel (&customFontLookAndFeel);

    headerComponent = std::make_unique<HeaderComponent>(p, parameters);

    headerComponent->onParameterControlViewChange = [this](bool newState)
    {
        if (newState)
        {
            advancedParameterControl.setVisible(true);
            parameterControl.setVisible(false);
            transientViewer.setVisible(false);
        }
        else
        {
            advancedParameterControl.setVisible(false);
            parameterControl.setVisible(true);
            transientViewer.setVisible(true);
        }
    };

    addAndMakeVisible(headerComponent.get());
    addAndMakeVisible(openGLBackground);
    addAndMakeVisible(advancedParameterControl);
//    advancedParameterControl.setVisible(false);
    addAndMakeVisible(parameterControl);
    addAndMakeVisible(transientViewer);
    addAndMakeVisible(textureComponent);
    bool state = processorRef.advancedParameterControlVisible.getValue();

    headerComponent->detailButton.setToggleState(state, juce::sendNotification);
    if (!state)
        advancedParameterControl.setVisible(false);
    else
    {
        parameterControl.setVisible(false);
        transientViewer.setVisible(false);
    }

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1400, 700);

    processorRef.setExternalModelName = [this] (int modelID, juce::String& modelName) {
        openGLBackground.externalModelLoaded(modelID, modelName);
    };

    //setResizable(false, false);
    // dirty work around to make the blobs appear correctly from the beginning
    auto fadeParam = parameters.getParameter(PluginParameters::FADE_ID.getParamID());
    auto fadeStatus = fadeParam->getValue();
    fadeParam->setValueNotifyingHost(0.5f*fadeStatus);
    fadeParam->setValueNotifyingHost(fadeStatus);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        apvts.removeParameterListener(parameterID, this);
    }
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colour::fromString(ColorPallete::BG));
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop(20);
    auto headerSection = r.removeFromTop(32);
    auto padSection = r.removeFromLeft(700);
    auto miniMapSection = r.removeFromLeft(176);
    r.removeFromLeft(45);
    auto sliderSection = r;

    juce::ignoreUnused(headerSection, miniMapSection, sliderSection);

    openGLBackground.setBounds(padSection);
    transientViewer.setBounds(710, 450, 163, 163);
    advancedParameterControl.setBounds(765, 60, 600, 600);


    auto areaParameter = getLocalBounds().removeFromRight(static_cast<int>((float)getWidth()*0.4f));
    areaParameter.removeFromTop(40);
    parameterControl.setBounds(areaParameter);
    headerComponent->setBounds(getLocalBounds());
    textureComponent.setBounds(getLocalBounds());

    processorRef.onNetwork1NameChange(processorRef.network1Name.toString());
    processorRef.onNetwork2NameChange(processorRef.network2Name.toString());
}

void AudioPluginAudioProcessorEditor::parameterChanged(const juce::String &parameterID, float newValue) {
    parameterControl.parameterChanged(parameterID, newValue);
    if (parameterID == PluginParameters::SELECT_NETWORK1_ID.getParamID() && newValue == 1.f) {
        openFileChooser(1);
    } else if (parameterID == PluginParameters::SELECT_NETWORK2_ID.getParamID() && newValue == 1.f) {
        openFileChooser(2);
    }
}

void AudioPluginAudioProcessorEditor::openFileChooser(int networkID) {
    fc = std::make_unique<juce::FileChooser> ("Choose a file to open...", juce::File::getSpecialLocation(juce::File::SpecialLocationType::userHomeDirectory),
                                              "*.ort", true);

    fc->launchAsync (juce::FileBrowserComponent::openMode
                     | juce::FileBrowserComponent::canSelectFiles,
                     [this, networkID] (const juce::FileChooser& chooser)
                     {
                         juce::File chosen;
                         auto results = chooser.getURLResults();

                         for (const auto& result : results) {
                             if (result.isLocalFile()) {
                                 chosen = result.getLocalFile();
                             }
                             else
                             {
                                 return;
                             }

                         }

                         if (chosen.getSize() != 0) {
                             processorRef.loadExternalModel(chosen.getFullPathName(), networkID);
                         } else {
                             auto param = (networkID == 1) ? PluginParameters::SELECT_NETWORK1_ID.getParamID() : PluginParameters::SELECT_NETWORK2_ID.getParamID();
                             apvts.getParameter(param)->setValueNotifyingHost(0.f);
                         }

                     });
}
