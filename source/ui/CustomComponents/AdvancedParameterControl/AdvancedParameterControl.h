//
// Created by Christian Scheer on 02/03/2023.
//

#ifndef VAESYNTH_ADVANCEDSETTINGS_H
#define VAESYNTH_ADVANCEDSETTINGS_H

#include "JuceHeader.h"
#include "../ParameterControl/Slider/CustomSliderComponent.h"
#include "../../../utils/colors.h"
#include "../../../PluginParameters.h"


class AdvancedParameterControl : public juce::Component
{
public:
    explicit AdvancedParameterControl(juce::AudioProcessorValueTreeState& parameters);
    ~AdvancedParameterControl() override;

    void resized() override;
    void paint(juce::Graphics &g) override;

    juce::Component** getTooltipPointers() {    return componentArray; };

private:
    // Sliders
    CustomSliderComponent transientSplitter1{"Split 1"}; //ms
    CustomSliderComponent transientSplitter2{"Split 2"}; //ms
    CustomSliderComponent raveFade {"Fade"};
    CustomSliderComponent compThreshold{"Threshold"};
    CustomSliderComponent compRatio{"Ratio"};
    CustomSliderComponent compMakeup{"Makeup"};
    CustomSliderComponent compMix{"Dynamics"};
    CustomSliderComponent masterMixParam{"Mix"};

    CustomSliderComponent grainDelay1Param1{"Interval"};
    CustomSliderComponent grainDelay1Param2{"Size"};
    CustomSliderComponent grainDelay1Param3{"Pitch"};
    CustomSliderComponent grainDelay1Param4{"Mix"};

    CustomSliderComponent grainDelay2Param1{"Interval"};
    CustomSliderComponent grainDelay2Param2{"Size"};
    CustomSliderComponent grainDelay2Param3{"Pitch"};
    CustomSliderComponent grainDelay2Param4{"Mix"};

    CustomSliderComponent* sliders[16] = {&transientSplitter1,
                                         &transientSplitter2,
                                         &raveFade,
                                         &compThreshold,
                                         &compRatio,
                                         &compMakeup,
                                         &compMix,
                                         &masterMixParam,
                                         &grainDelay1Param1,
                                         &grainDelay1Param2,
                                         &grainDelay1Param3,
                                         &grainDelay1Param4,
                                         &grainDelay2Param1,
                                         &grainDelay2Param2,
                                         &grainDelay2Param3,
                                         &grainDelay2Param4};

    juce::Component* componentArray[16];
    int numberOfSliders = 16;

    // Geometry
    const int sliderWidth = 73;
    const int sliderHeight = 250;
    const int sliderDistance = 2;


    float yLineTop = 47.f;
    float yTextTop = 25.f;
    float yTextBottom = 325.f;
    float yLineBottom = 347.f;
    float lineHeight = 3.f;
    float textHeight = 20.f;

    //Line Transient Control
    juce::Rectangle<float> topLine1 {10.f, yLineTop, 128.f, lineHeight};
    juce::Rectangle<float> topText1 {10.f, yTextTop, 128.f, textHeight};
    //Line Blend
    juce::Rectangle<float> topLine2 {160.f, yLineTop, 53.f, lineHeight};
    juce::Rectangle<float> topText2 {160.f, yTextTop, 53.f, textHeight};
    //Line Post Processor
    juce::Rectangle<float> topLine3 {235.f, yLineTop, 278.f, lineHeight};
    juce::Rectangle<float> topText3 {235.f, yTextTop, 278.f, textHeight};
    //Global
    juce::Rectangle<float> topLine4 {535.f, yLineTop, 53.f, lineHeight};
    juce::Rectangle<float> topText4 {535.f, yTextTop, 53.f, textHeight};

    //Grain 1
    juce::Rectangle<float> bottomLine1 {10.f, yLineBottom, 278.f, lineHeight};
    juce::Rectangle<float> bottomText1 {10.f, yTextBottom, 278.f, textHeight};
    //Grain 2
    juce::Rectangle<float> bottomLine2 {310.f, yLineBottom, 278.f, lineHeight};
    juce::Rectangle<float> bottomText2 {310.f, yTextBottom, 278.f, textHeight};
};


#endif //VAESYNTH_ADVANCEDSETTINGS_H
