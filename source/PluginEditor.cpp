#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginParameters.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p, juce::AudioProcessorValueTreeState &parameters)
    : AudioProcessorEditor(&p), apvts(parameters), processorRef(p), fileChooserManager(p), transientViewer(p) /*, openGLBackground(parameters, p)*/, advancedParameterControl(parameters), parameterControl(parameters),
      footerComponent(p, parameters), headerComponent(p, parameters)
{
    juce::ignoreUnused(processorRef);

    openGLBackground = std::make_unique<OpenGLBackground>(parameters, p);

    for (auto &parameterID : PluginParameters::getPluginParameterList())
    {
        parameters.addParameterListener(parameterID, this);
    }

    juce::LookAndFeel::setDefaultLookAndFeel(&customFontLookAndFeel);

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

    layout.setParent(this);
    defineLayout();
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
    setResizable(true, true);
    setResizeLimits(CustomFontLookAndFeel::originalWidth / 2,
                    CustomFontLookAndFeel::originalHeight / 2,
                    CustomFontLookAndFeel::originalWidth * 2,
                    CustomFontLookAndFeel::originalHeight * 2);
    getConstrainer()->setFixedAspectRatio(static_cast<double>(CustomFontLookAndFeel::originalWidth)
                                          / static_cast<double>(CustomFontLookAndFeel::originalHeight));

    auto storedScale = static_cast<float>(processorRef.windowScale.getValue());
    if (storedScale <= 0.0f)
        storedScale = 1.0f;
    storedScale = juce::jlimit(0.5f, 2.0f, storedScale);
    setSize(juce::roundToInt(CustomFontLookAndFeel::originalWidth * storedScale),
            juce::roundToInt(CustomFontLookAndFeel::originalHeight * storedScale));

    processorRef.setExternalModelName = [this](int modelID, juce::String &modelName)
    {
        openGLBackground->externalModelLoaded(modelID, modelName);
    };

    // dirty work around to make the blobs appear correctly from the beginning
    auto fadeParam = parameters.getParameter(PluginParameters::FADE_ID.getParamID());
    auto fadeStatus = fadeParam->getValue();
    fadeParam->setValueNotifyingHost(0.5f * fadeStatus);
    fadeParam->setValueNotifyingHost(fadeStatus);

    setInterceptsMouseClicks(true, true);

    headerComponent.onScyloneButtonClick = [this](bool newState)
    {
        if (newState)
        {
            componentAnimator->fadeOut(&transientViewer, fadeTime);
            if (headerComponent.detailButton.getToggleState())
            {
                componentAnimator->fadeOut(&advancedParameterControl, fadeTime);
            }
            else
            {
                componentAnimator->fadeOut(&parameterControl, fadeTime);
            }
        }
        else
        {
            if (headerComponent.detailButton.getToggleState())
            {
                componentAnimator->fadeIn(&advancedParameterControl, fadeTime);
            }
            else
            {
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
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    for (auto &parameterID : PluginParameters::getPluginParameterList())
    {
        apvts.removeParameterListener(parameterID, this);
    }
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colour::fromString(ColorPallete::BG));
}

void AudioPluginAudioProcessorEditor::defineLayout()
{
    layout.add(headerComponent, 0, 0, 1400, 700);
    layout.add(*openGLBackground, 0, 52, 700, 613);
    layout.add(advancedParameterControl, 765, 60, 600, 600);
    layout.add(parameterControl, 840, 40, 560, 660);
    layout.add(transientViewer, 710, 450, 163, 163);
    layout.add(textureComponent, 0, 0, 1400, 700);
    layout.add(footerComponent, 0, 665, 1400, 35);
}

void AudioPluginAudioProcessorEditor::resized()
{
    if (getWidth() <= 0 || getHeight() <= 0)
        return;

    const float scale = static_cast<float>(getWidth()) / static_cast<float>(CustomFontLookAndFeel::originalWidth);
    customFontLookAndFeel.setScale(scale);
    processorRef.windowScale = scale;

    layout.apply(scale);

    if (openGLBackground->isSignalFlowChartVisible())
        openGLBackground->setBounds((juce::Rectangle<float>(0, 52, 1400, 593) * scale).toNearestInt());

    processorRef.onNetwork1NameChange(processorRef.network1Name.toString());
    processorRef.onNetwork2NameChange(processorRef.network2Name.toString());
}

void AudioPluginAudioProcessorEditor::parameterChanged(const juce::String &parameterID, float newValue)
{
    parameterControl.parameterChanged(parameterID, newValue);
    if (parameterID == PluginParameters::SELECT_NETWORK1_ID.getParamID() && newValue == 1.f)
    {
        juce::Component::SafePointer<AudioPluginAudioProcessorEditor> safeThis(this);
        juce::MessageManager::callAsync([safeThis]()
                                        {
            if (safeThis != nullptr)
                safeThis->fileChooserManager.openFileChooserForNetwork(1, safeThis.getComponent()); });
    }
    else if (parameterID == PluginParameters::SELECT_NETWORK2_ID.getParamID() && newValue == 1.f)
    {
        juce::Component::SafePointer<AudioPluginAudioProcessorEditor> safeThis(this);
        juce::MessageManager::callAsync([safeThis]()
                                        {
            if (safeThis != nullptr)
                safeThis->fileChooserManager.openFileChooserForNetwork(2, safeThis.getComponent()); });
    }
}
