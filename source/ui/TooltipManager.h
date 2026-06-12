#pragma once

#include <functional>
#include <map>

#include <juce_gui_basics/juce_gui_basics.h>

class OpenGLBackground;

// Maps UI components to footer tooltip text via recursive mouse listening on the editor.
// Known limitation: moving between tooltip targets via a non-tooltip child may briefly clear the footer.
class TooltipManager : private juce::MouseListener,
                       private juce::Timer
{
public:
    using SetTooltipTextFn = std::function<void(const juce::String&)>;

    TooltipManager(juce::Component& parentComponent, SetTooltipTextFn setTooltipText);
    ~TooltipManager() override;

    void initializeTooltipMap(juce::Component** xyPadComponents,
                              juce::Component** parameterControlComponents,
                              juce::Component** headerComponents,
                              juce::Component** advancedParameterControlComponents,
                              OpenGLBackground* openGLBackground);

private:
    void addTooltip(juce::Component* component, const juce::String& tooltipText);
    void addTooltipAt(juce::Component** array, int index, int arraySize, const juce::String& tooltipText);
    bool isMouseOverTooltipTarget() const;

    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void timerCallback() override;

    juce::Component& parentComponent;
    SetTooltipTextFn setTooltipText;
    std::map<juce::Component*, juce::String> tooltipMap;

    static constexpr int clearTooltipDelayMs = 50;
};
