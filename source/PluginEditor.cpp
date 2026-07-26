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

    xyPadComponents = openGLBackground->getXYPad()->getTooltipPointers();
    parameterControlComponents = parameterControl.getTooltipPointers();
    advancedParameterControlComponents = advancedParameterControl.getTooltipPointers();
    headerComponents = headerComponent.getTooltipPointers();

    setInterceptsMouseClicks(true, true);
    addMouseListener(this, true);

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

    componentAnimator = std::make_unique<juce::ComponentAnimator>();
    initializeTooltipMap();
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

// Tooltips
void AudioPluginAudioProcessorEditor::mouseEnter(const juce::MouseEvent &event)
{
    auto component = event.originalComponent;
    auto it = tooltipMap.find(component);
    if (it != tooltipMap.end())
    {
        footerComponent.setTooltipText(it->second);
    }
}

void AudioPluginAudioProcessorEditor::mouseExit(const juce::MouseEvent &)
{
    footerComponent.setTooltipText("");
}

void AudioPluginAudioProcessorEditor::initializeTooltipMap()
{
    tooltipMap.clear();

    // Check if all required components are loaded
    if (!xyPadComponents)
    {
        throw std::runtime_error("xyPadComponents not loaded");
    }
    if (!parameterControlComponents)
    {
        throw std::runtime_error("parameterControlComponents not loaded");
    }
    if (!headerComponents)
    {
        throw std::runtime_error("headerComponents not loaded");
    }
    if (!advancedParameterControlComponents)
    {
        throw std::runtime_error("advancedParameterControlComponents not loaded");
    }
    if (!openGLBackground)
    {
        throw std::runtime_error("openGLBackground not loaded");
    }

    // Manually add each component to the map with its corresponding tooltip
    tooltipMap[xyPadComponents[0]] = "RAVE Network 1";
    tooltipMap[xyPadComponents[1]] = "Load custom RAVE Network 1";
    tooltipMap[xyPadComponents[2]] = "Grain Delay On/Off RAVE Network 1";
    tooltipMap[xyPadComponents[3]] = "On/Off RAVE Network 1";
    tooltipMap[xyPadComponents[4]] = "RAVE Network 2";
    tooltipMap[xyPadComponents[5]] = "Load custom RAVE Network 2";
    tooltipMap[xyPadComponents[6]] = "Grain Delay On/Off RAVE Network 2";
    tooltipMap[xyPadComponents[7]] = "On/Off RAVE Network 2";

    tooltipMap[parameterControlComponents[0]] = "Fade between both networks";
    tooltipMap[parameterControlComponents[1]] = "Fade between both networks";
    tooltipMap[parameterControlComponents[2]] = "Fade between both networks";
    tooltipMap[parameterControlComponents[3]] = "Dry/Wet Output Compressor";
    tooltipMap[parameterControlComponents[4]] = "Dry/Wet Output Compressor";
    tooltipMap[parameterControlComponents[5]] = "Dry/Wet Output Compressor";
    tooltipMap[parameterControlComponents[6]] = "Dry/Wet Input Output signal";
    tooltipMap[parameterControlComponents[7]] = "Dry/Wet Input Output signal";
    tooltipMap[parameterControlComponents[8]] = "Dry/Wet Input Output signal";

    tooltipMap[headerComponents[0]] = "Trim input gain";
    tooltipMap[headerComponents[1]] = "Trim output gain";
    tooltipMap[headerComponents[2]] = "Power User View";
    tooltipMap[headerComponents[3]] = juce::String("Scyclone v.") + ProjectInfo::versionString + juce::String(" | Click for more information.");

    tooltipMap[advancedParameterControlComponents[0]] = "RAVE Network 1 Transient Shaper Attack Time";
    tooltipMap[advancedParameterControlComponents[1]] = "RAVE Network 2 Transient Shaper Attack Time";
    tooltipMap[advancedParameterControlComponents[2]] = "Crossfade between RAVE networks";
    tooltipMap[advancedParameterControlComponents[3]] = "Output Compressor Threshold";
    tooltipMap[advancedParameterControlComponents[4]] = "Output Compressor Ratio";
    tooltipMap[advancedParameterControlComponents[5]] = "Output Compressor Makeup";
    tooltipMap[advancedParameterControlComponents[6]] = "Output Compressor Dry/Wet";
    tooltipMap[advancedParameterControlComponents[7]] = "Master Dry/Wet";
    tooltipMap[advancedParameterControlComponents[8]] = "RAVE Network 1 Grain Interval";
    tooltipMap[advancedParameterControlComponents[9]] = "RAVE Network 1 Grain Size";
    tooltipMap[advancedParameterControlComponents[10]] = "RAVE Network 1 Grain Pitch";
    tooltipMap[advancedParameterControlComponents[11]] = "RAVE Network 1 Grain Delay Dry/Wet";
    tooltipMap[advancedParameterControlComponents[12]] = "RAVE Network 2 Grain Interval";
    tooltipMap[advancedParameterControlComponents[13]] = "RAVE Network 2 Grain Size";
    tooltipMap[advancedParameterControlComponents[14]] = "RAVE Network 2 Grain Pitch";
    tooltipMap[advancedParameterControlComponents[15]] = "RAVE Network 2 Grain Delay Dry/Wet";

    auto *labels = openGLBackground->getLabels();
    if (labels)
    {
        tooltipMap[&labels->sharp] = "Low Cut Filter Frequency";
        tooltipMap[&labels->attack] = "Transient Shaper: Attack";
        tooltipMap[&labels->smooth] = "High Cut Filter Frequency";
        tooltipMap[&labels->sustain] = "Transient Shaper: Sustain";
    }
}
