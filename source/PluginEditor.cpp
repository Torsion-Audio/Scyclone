#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginParameters.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& parameters)
    : AudioProcessorEditor (&p), apvts(parameters), processorRef (p), transientViewer(p), openGLBackground(parameters, p), advancedParameterControl(parameters), parameterControl(parameters),
      footerComponent(p, parameters)
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
    addAndMakeVisible(parameterControl);
    addAndMakeVisible(transientViewer);
    addAndMakeVisible(textureComponent);
    addAndMakeVisible(footerComponent);
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

    xyPadComponents = openGLBackground.getXYPad()->getTooltipPointers();
    parameterControlComponents = parameterControl.getTooltipPointers();
    advancedParameterControlComponents = advancedParameterControl.getTooltipPointers();
    headerComponents = headerComponent->getTooltipPointers();

    setInterceptsMouseClicks(true, true);
    addMouseListener(this, true);
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

    auto logoSection = juce::Rectangle<int>{getWidth()/2 - 20, 20, 40, 20};

    auto headerSection = r.removeFromTop(32);
    auto padSection = r.removeFromLeft(700);
    padSection.removeFromBottom(35);
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
    footerComponent.setBounds(getLocalBounds().removeFromBottom(35));

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

void AudioPluginAudioProcessorEditor::mouseEnter(const juce::MouseEvent &event) {
    auto component = event.originalComponent;
    std::cout << component->getName() << "\n";
    if (component == xyPadComponents[0]) {
        footerComponent.setTooltipText("RAVE Network 1");
    } else if (component == xyPadComponents[1]) {
        footerComponent.setTooltipText("Select RAVE Network 1");
    } else if (component == xyPadComponents[2]) {
        footerComponent.setTooltipText("Grain Delay On/Off RAVE Network 1");
    } else if (component == xyPadComponents[3]) {
        footerComponent.setTooltipText("On/Off RAVE Network 1");
    } else if (component == xyPadComponents[4]) {
        footerComponent.setTooltipText("RAVE Network 2");
    } else if (component == xyPadComponents[5]) {
        footerComponent.setTooltipText("Select RAVE Network 2");
    } else if (component == xyPadComponents[6]) {
        footerComponent.setTooltipText("Grain Delay On/Off RAVE Network 2");
    } else if (component == xyPadComponents[7]){
        footerComponent.setTooltipText("On/Off RAVE Network 2");
    }

    else if (component == parameterControlComponents[0] || component == parameterControlComponents[1] || component == parameterControlComponents[2]) {
        footerComponent.setTooltipText("Fade between both networks");
    } else if (component == parameterControlComponents[3] || component == parameterControlComponents[4] || component == parameterControlComponents[5]) {
        footerComponent.setTooltipText("Dry/Wet Output Compressor");
    } else if (component == parameterControlComponents[6] || component == parameterControlComponents[7] || component == parameterControlComponents[8]) {
        footerComponent.setTooltipText("Dry/Wet Input Output signal");
    }

    else if (component == headerComponents[0]) {
        footerComponent.setTooltipText("Trim input gain");
    } else if (component == headerComponents[1]) {
        footerComponent.setTooltipText("Trim output gain");
    } else if (component == headerComponents[2]) {
        footerComponent.setTooltipText("Power User View");
    }

    else if (component == advancedParameterControlComponents[0])
        footerComponent.setTooltipText("RAVE Network 1 Transient Shaper Attack Time");
    else if (component == advancedParameterControlComponents[1])
            footerComponent.setTooltipText("RAVE Network 2 Transient Shaper Attack Time");
    else if (component == advancedParameterControlComponents[2])
        footerComponent.setTooltipText("Crossfade between RAVE networks");
    else if (component == advancedParameterControlComponents[3])
        footerComponent.setTooltipText("Output Compressor Threshold");
    else if (component == advancedParameterControlComponents[4])
        footerComponent.setTooltipText("Output Compressor Ratio");
    else if (component == advancedParameterControlComponents[5])
        footerComponent.setTooltipText("Output Compressor Makeup");
    else if (component == advancedParameterControlComponents[6])
        footerComponent.setTooltipText("Output Compressor Dry/Wet");
    else if (component == advancedParameterControlComponents[7])
        footerComponent.setTooltipText("Master Dry/Wet");
    else if (component == advancedParameterControlComponents[8])
        footerComponent.setTooltipText("RAVE Network 1 Grain Interval");
    else if (component == advancedParameterControlComponents[9])
        footerComponent.setTooltipText("RAVE Network 1 Grain Size");
    else if (component == advancedParameterControlComponents[10])
        footerComponent.setTooltipText("RAVE Network 1 Grain Pitch");
    else if (component == advancedParameterControlComponents[11])
        footerComponent.setTooltipText("RAVE Network 1 Grain Delay Dry/Wet");
    else if (component == advancedParameterControlComponents[12])
        footerComponent.setTooltipText("RAVE Network 2 Grain Interval");
    else if (component == advancedParameterControlComponents[13])
        footerComponent.setTooltipText("RAVE Network 2 Grain Size");
    else if (component == advancedParameterControlComponents[14])
        footerComponent.setTooltipText("RAVE Network 2 Grain Pitch");
    else if (component == advancedParameterControlComponents[15])
        footerComponent.setTooltipText("RAVE Network 2 Grain Delay Dry/Wet");

    else if (component->getComponentID() == openGLBackground.getLabels()->sharp.getComponentID())
        footerComponent.setTooltipText("Low Cut Filter Frequency");
    else if (component->getComponentID() == openGLBackground.getLabels()->attack.getComponentID())
        footerComponent.setTooltipText("Transient Shaper: Attack");
    else if (component->getComponentID() == openGLBackground.getLabels()->smooth.getComponentID())
        footerComponent.setTooltipText("High Cut Filter Frequency");
    else if (component->getComponentID() == openGLBackground.getLabels()->sustain.getComponentID())
        footerComponent.setTooltipText("Transient Shaper: Sustain");
}

void AudioPluginAudioProcessorEditor::mouseExit(const juce::MouseEvent &event) {
    footerComponent.setTooltipText("");
}