// Tooltip registry: component arrays must match TooltipCounts sizes and index order in initializeTooltipMap.
// OpenGL label pointers are captured at init; call initializeTooltipMap again if labels are recreated.
// To add a control: update componentArray in the widget and add one entry here.

#include "TooltipManager.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include "ui/CustomComponents/OpenGLBackground/OpenGLBackground.h"
#include "ui/TooltipCounts.h"

namespace
{
    void logTooltipInitFailure(const char* message)
    {
        jassertfalse;
        juce::Logger::writeToLog("TooltipManager: " + juce::String(message));
    }
}

TooltipManager::TooltipManager(juce::Component& parent, SetTooltipTextFn setTooltipTextFn)
    : parentComponent(parent), setTooltipText(std::move(setTooltipTextFn))
{
    parentComponent.addMouseListener(this, true);
}

TooltipManager::~TooltipManager()
{
    stopTimer();
    parentComponent.removeMouseListener(this);
}

void TooltipManager::addTooltip(juce::Component* component, const juce::String& tooltipText)
{
    if (component != nullptr)
        tooltipMap[component] = tooltipText;
}

void TooltipManager::addTooltipAt(juce::Component** array, int index, int arraySize, const juce::String& tooltipText)
{
    jassert(array != nullptr && index >= 0 && index < arraySize);

    if (array == nullptr || index < 0 || index >= arraySize)
    {
        DBG("TooltipManager: skipping tooltip at index " << index);
        return;
    }

    addTooltip(array[index], tooltipText);
}

void TooltipManager::initializeTooltipMap(juce::Component** xyPadComponents,
                                          juce::Component** parameterControlComponents,
                                          juce::Component** headerComponents,
                                          juce::Component** advancedParameterControlComponents,
                                          OpenGLBackground* openGLBackground)
{
    tooltipMap.clear();

    if (xyPadComponents == nullptr)
    {
        logTooltipInitFailure("xyPadComponents not loaded");
        return;
    }
    if (parameterControlComponents == nullptr)
    {
        logTooltipInitFailure("parameterControlComponents not loaded");
        return;
    }
    if (headerComponents == nullptr)
    {
        logTooltipInitFailure("headerComponents not loaded");
        return;
    }
    if (advancedParameterControlComponents == nullptr)
    {
        logTooltipInitFailure("advancedParameterControlComponents not loaded");
        return;
    }
    if (openGLBackground == nullptr)
    {
        logTooltipInitFailure("openGLBackground not loaded");
        return;
    }

    addTooltipAt(xyPadComponents, 0, TooltipCounts::xyPad, "RAVE Network 1");
    addTooltipAt(xyPadComponents, 1, TooltipCounts::xyPad, "Load custom RAVE Network 1");
    addTooltipAt(xyPadComponents, 2, TooltipCounts::xyPad, "Grain Delay On/Off RAVE Network 1");
    addTooltipAt(xyPadComponents, 3, TooltipCounts::xyPad, "On/Off RAVE Network 1");
    addTooltipAt(xyPadComponents, 4, TooltipCounts::xyPad, "RAVE Network 2");
    addTooltipAt(xyPadComponents, 5, TooltipCounts::xyPad, "Load custom RAVE Network 2");
    addTooltipAt(xyPadComponents, 6, TooltipCounts::xyPad, "Grain Delay On/Off RAVE Network 2");
    addTooltipAt(xyPadComponents, 7, TooltipCounts::xyPad, "On/Off RAVE Network 2");

    for (int i = 0; i < 3; ++i)
        addTooltipAt(parameterControlComponents, i, TooltipCounts::parameterControl, "Fade between both networks");
    for (int i = 3; i < 6; ++i)
        addTooltipAt(parameterControlComponents, i, TooltipCounts::parameterControl, "Dry/Wet Output Compressor");
    for (int i = 6; i < 9; ++i)
        addTooltipAt(parameterControlComponents, i, TooltipCounts::parameterControl, "Dry/Wet Input Output signal");

    addTooltipAt(headerComponents, 0, TooltipCounts::header, "Trim input gain");
    addTooltipAt(headerComponents, 1, TooltipCounts::header, "Trim output gain");
    addTooltipAt(headerComponents, 2, TooltipCounts::header, "Power User View");
    addTooltipAt(headerComponents, 3, TooltipCounts::header,
                 juce::String("Scyclone v.") + ProjectInfo::versionString + juce::String(" | Click for more information."));

    addTooltipAt(advancedParameterControlComponents, 0, TooltipCounts::advancedParameterControl, "RAVE Network 1 Transient Shaper Attack Time");
    addTooltipAt(advancedParameterControlComponents, 1, TooltipCounts::advancedParameterControl, "RAVE Network 2 Transient Shaper Attack Time");
    addTooltipAt(advancedParameterControlComponents, 2, TooltipCounts::advancedParameterControl, "Crossfade between RAVE networks");
    addTooltipAt(advancedParameterControlComponents, 3, TooltipCounts::advancedParameterControl, "Output Compressor Threshold");
    addTooltipAt(advancedParameterControlComponents, 4, TooltipCounts::advancedParameterControl, "Output Compressor Ratio");
    addTooltipAt(advancedParameterControlComponents, 5, TooltipCounts::advancedParameterControl, "Output Compressor Makeup");
    addTooltipAt(advancedParameterControlComponents, 6, TooltipCounts::advancedParameterControl, "Output Compressor Dry/Wet");
    addTooltipAt(advancedParameterControlComponents, 7, TooltipCounts::advancedParameterControl, "Master Dry/Wet");
    addTooltipAt(advancedParameterControlComponents, 8, TooltipCounts::advancedParameterControl, "RAVE Network 1 Grain Interval");
    addTooltipAt(advancedParameterControlComponents, 9, TooltipCounts::advancedParameterControl, "RAVE Network 1 Grain Size");
    addTooltipAt(advancedParameterControlComponents, 10, TooltipCounts::advancedParameterControl, "RAVE Network 1 Grain Pitch");
    addTooltipAt(advancedParameterControlComponents, 11, TooltipCounts::advancedParameterControl, "RAVE Network 1 Grain Delay Dry/Wet");
    addTooltipAt(advancedParameterControlComponents, 12, TooltipCounts::advancedParameterControl, "RAVE Network 2 Grain Interval");
    addTooltipAt(advancedParameterControlComponents, 13, TooltipCounts::advancedParameterControl, "RAVE Network 2 Grain Size");
    addTooltipAt(advancedParameterControlComponents, 14, TooltipCounts::advancedParameterControl, "RAVE Network 2 Grain Pitch");
    addTooltipAt(advancedParameterControlComponents, 15, TooltipCounts::advancedParameterControl, "RAVE Network 2 Grain Delay Dry/Wet");

    auto* labels = openGLBackground->getLabels();
    if (labels != nullptr)
    {
        addTooltip(&labels->sharp, "Low Cut Filter Frequency");
        addTooltip(&labels->attack, "Transient Shaper: Attack");
        addTooltip(&labels->smooth, "High Cut Filter Frequency");
        addTooltip(&labels->sustain, "Transient Shaper: Sustain");
    }
}

bool TooltipManager::isMouseOverTooltipTarget() const
{
    auto* componentUnderMouse = juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

    for (auto* component = componentUnderMouse; component != nullptr; component = component->getParentComponent())
    {
        if (tooltipMap.contains(component))
            return true;
    }

    return false;
}

void TooltipManager::mouseEnter(const juce::MouseEvent& event)
{
    stopTimer();

    auto* component = event.originalComponent;
    if (tooltipMap.contains(component))
        setTooltipText(tooltipMap[component]);
    else
        setTooltipText({}); // intentional: clear when entering non-tooltip UI and cancel pending deferred clear
}

void TooltipManager::mouseExit(const juce::MouseEvent& event)
{
    if (tooltipMap.contains(event.originalComponent))
        startTimer(clearTooltipDelayMs);
}

void TooltipManager::timerCallback()
{
    stopTimer();

    if (isMouseOverTooltipTarget())
        return;

    setTooltipText({});
}
