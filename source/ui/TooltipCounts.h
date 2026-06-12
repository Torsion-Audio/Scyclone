#pragma once

// Expected sizes of componentArray in widgets that expose getTooltipPointers().
// TooltipManager registers tooltips by index 
// these constants are used for bounds checks in addTooltipAt(). 
// If you add/remove a tooltip target, update 
// - the widget's componentArray, 
// - this constant, 
// - and the matching entries in TooltipManager.cpp.
namespace TooltipCounts
{
    inline constexpr int xyPad = 8;                    // XYPad::componentArray
    inline constexpr int parameterControl = 9;         // ParameterControl::componentArray
    inline constexpr int header = 4;                   // HeaderComponent::componentArray
    inline constexpr int advancedParameterControl = 16; // AdvancedParameterControl::componentArray
}
