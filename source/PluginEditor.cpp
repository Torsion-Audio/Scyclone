#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginParameters.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& parameters)
    : AudioProcessorEditor (&p), apvts(parameters), processorRef (p), fileChooserManager(p), transientViewer(p)/*, openGLBackground(parameters, p)*/, advancedParameterControl(parameters), parameterControl(parameters),
      footerComponent(p, parameters), headerComponent(p, parameters)
{
    juce::ignoreUnused (processorRef);

    openGLBackground = std::make_unique<OpenGLBackground>(parameters, p);

    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        parameters.addParameterListener(parameterID, this);
    }

    juce::LookAndFeel::setDefaultLookAndFeel (&customFontLookAndFeel);

    addAndMakeVisible(headerComponent);
    addAndMakeVisible(*openGLBackground);
    addAndMakeVisible(advancedParameterControl);
    addAndMakeVisible(parameterControl);
    addAndMakeVisible(transientViewer);
    addAndMakeVisible(textureComponent);
    addAndMakeVisible(footerComponent);

    componentAnimator = std::make_unique<juce::ComponentAnimator>();

    headerComponent.onParameterControlViewChange = [this](bool newState)
    {
        if (newState)
        {
            componentAnimator->fadeIn(&advancedParameterControl, fadeTime);
            componentAnimator->fadeOut(&parameterControl, fadeTime);
            componentAnimator->fadeOut(&transientViewer, fadeTime);
        }
        else
        {
            componentAnimator->fadeOut(&advancedParameterControl, fadeTime);
            componentAnimator->fadeIn(&parameterControl, fadeTime);
            componentAnimator->fadeIn(&transientViewer, fadeTime);
        }
    };

    bool state = processorRef.advancedParameterControlVisible.getValue();

    headerComponent.detailButton.setToggleState(state, juce::sendNotification);
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
        openGLBackground->externalModelLoaded(modelID, modelName);
    };

    setResizable(false, false);
    // dirty work around to make the blobs appear correctly from the beginning
    auto fadeParam = parameters.getParameter(PluginParameters::FADE_ID.getParamID());
    auto fadeStatus = fadeParam->getValue();
    fadeParam->setValueNotifyingHost(0.5f*fadeStatus);
    fadeParam->setValueNotifyingHost(fadeStatus);

    setInterceptsMouseClicks(true, true);

    headerComponent.onScyloneButtonClick = [this](bool newState)
            {
                if (newState) {
                    componentAnimator->fadeOut(&transientViewer, fadeTime);
                    if (headerComponent.detailButton.getToggleState()) {
                        componentAnimator->fadeOut(&advancedParameterControl, fadeTime);
                    }
                    else {
                        componentAnimator->fadeOut(&parameterControl, fadeTime);
                    }
                }
                else {
                    if (headerComponent.detailButton.getToggleState()) {
                        componentAnimator->fadeIn(&advancedParameterControl, fadeTime);
                    }
                    else {
                        componentAnimator->fadeIn(&parameterControl, fadeTime);
                        componentAnimator->fadeIn(&transientViewer, fadeTime);
                    }
                }

                openGLBackground->showSignalFlowChart(newState);
                resized();

                headerComponent.detailButton.setEnabled(!newState);
            };

    tooltipManager = std::make_unique<TooltipManager>(
        *this,
        [this](const juce::String& text) { footerComponent.setTooltipText(text); });

    tooltipManager->initializeTooltipMap(
        openGLBackground->getXYPad()->getTooltipPointers(),
        parameterControl.getTooltipPointers(),
        headerComponent.getTooltipPointers(),
        advancedParameterControl.getTooltipPointers(),
        openGLBackground.get());
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

    if (openGLBackground->isSignalFlowChartVisible()) {
        auto window = getLocalBounds();
        window.removeFromTop(headerSection.getHeight() + 20);
        window.removeFromBottom(footerComponent.getHeight() + 20);
        openGLBackground->setBounds(window);
    }
    else
        openGLBackground->setBounds(padSection);

    transientViewer.setBounds(710, 450, 163, 163);
    advancedParameterControl.setBounds(765, 60, 600, 600);

    auto areaParameter = getLocalBounds().removeFromRight(static_cast<int>((float)getWidth()*0.4f));
    areaParameter.removeFromTop(40);
    parameterControl.setBounds(areaParameter);

    headerComponent.setBounds(getLocalBounds());

    textureComponent.setBounds(getLocalBounds());

    footerComponent.setBounds(getLocalBounds().removeFromBottom(35));

    processorRef.onNetwork1NameChange(processorRef.network1Name.toString());
    processorRef.onNetwork2NameChange(processorRef.network2Name.toString());

}

void AudioPluginAudioProcessorEditor::parameterChanged(const juce::String &parameterID, float newValue) {
    parameterControl.parameterChanged(parameterID, newValue);
    if (parameterID == PluginParameters::SELECT_NETWORK1_ID.getParamID() && newValue == 1.f) {
        fileChooserManager.openFileChooserForNetwork(1);
    } else if (parameterID == PluginParameters::SELECT_NETWORK2_ID.getParamID() && newValue == 1.f) {
        fileChooserManager.openFileChooserForNetwork(2);
    }
}
