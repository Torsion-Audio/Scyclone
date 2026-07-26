//
// Created by Christian Scheer on 02/03/2023.
//
#include "AdvancedParameterControl.h"
AdvancedParameterControl::AdvancedParameterControl(juce::AudioProcessorValueTreeState &parameters)
{

    transientSplitter1.addSliderAttachment(parameters, PluginParameters::TRAN_ATTACK_TIME_NETWORK1_ID.getParamID());
    transientSplitter2.addSliderAttachment(parameters, PluginParameters::TRAN_ATTACK_TIME_NETWORK2_ID.getParamID());
    raveFade.addSliderAttachment(parameters, PluginParameters::FADE_ID.getParamID());
    compThreshold.addSliderAttachment(parameters, PluginParameters::COMP_THRESHOLD_ID.getParamID());
    compRatio.addSliderAttachment(parameters, PluginParameters::COMP_RATIO_ID.getParamID());
    compMakeup.addSliderAttachment(parameters, PluginParameters::COMP_MAKEUPGAIN_ID.getParamID());
    compMix.addSliderAttachment(parameters, PluginParameters::COMP_DRY_WET_ID.getParamID());
    masterMixParam.addSliderAttachment(parameters, PluginParameters::DRY_WET_ID.getParamID());

    grainDelay1Param1.addSliderAttachment(parameters, PluginParameters::GRAIN_NETWORK1_INTERVAL_ID.getParamID());
    grainDelay1Param2.addSliderAttachment(parameters, PluginParameters::GRAIN_NETWORK1_SIZE_ID.getParamID());
    grainDelay1Param3.addSliderAttachment(parameters, PluginParameters::GRAIN_NETWORK1_PITCH_ID.getParamID());
    grainDelay1Param4.addSliderAttachment(parameters, PluginParameters::GRAIN_NETWORK1_MIX_ID.getParamID());

    grainDelay2Param1.addSliderAttachment(parameters, PluginParameters::GRAIN_NETWORK2_INTERVAL_ID.getParamID());
    grainDelay2Param2.addSliderAttachment(parameters, PluginParameters::GRAIN_NETWORK2_SIZE_ID.getParamID());
    grainDelay2Param3.addSliderAttachment(parameters, PluginParameters::GRAIN_NETWORK2_PITCH_ID.getParamID());
    grainDelay2Param4.addSliderAttachment(parameters, PluginParameters::GRAIN_NETWORK2_MIX_ID.getParamID());

    for (int i = 0; i<numberOfSliders; i++)
    {
        if (i == 0) {
            //transient 1
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colour{0xffF02FC2});
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colour{0xff6094EA});
        } else if(i == 1) {
            //transient 2
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colour{0xffC82471});
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colour{0xffD09192});
        } else if(i == 2) {
            //fade
            // use default
        } else if(i < 7) {
            //comp
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colour{0xff1E5B53});
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colour{0xffCCFFAA});
        } else if(i == 7) {
            //master mix
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colour{0xff18A5A7});
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colour{0xffBFFFC7});
        } else if(i < 12) {
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colour{0xff6E0380});
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colour{0xff49FFFF});
        } else {
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourTopId, juce::Colour{0xff74276C});
            sliders[i]->setCustomColour(CustomSliderColourID::gradientColourBottomId, juce::Colour{0xffFD8263});
        }
    }

    componentArray[0] = *transientSplitter1.getSliderComponents();
    componentArray[1] = *transientSplitter2.getSliderComponents();
    componentArray[2] = *raveFade.getSliderComponents();
    componentArray[3] = *compThreshold.getSliderComponents();
    componentArray[4] = *compRatio.getSliderComponents();
    componentArray[5] = *compMakeup.getSliderComponents();
    componentArray[6] = *compMix.getSliderComponents();
    componentArray[7] = *masterMixParam.getSliderComponents();
    componentArray[8] = *grainDelay1Param1.getSliderComponents();
    componentArray[9] = *grainDelay1Param2.getSliderComponents();
    componentArray[10] = *grainDelay1Param3.getSliderComponents();
    componentArray[11] = *grainDelay1Param4.getSliderComponents();
    componentArray[12] = *grainDelay2Param1.getSliderComponents();
    componentArray[13] = *grainDelay2Param2.getSliderComponents();
    componentArray[14] = *grainDelay2Param3.getSliderComponents();
    componentArray[15] = *grainDelay2Param4.getSliderComponents();
}

AdvancedParameterControl::~AdvancedParameterControl()
{
    setLookAndFeel(nullptr);
}

void AdvancedParameterControl::defineLayout()
{
    for (int i = 0; i < numberOfSliders; i++)
    {
        const int row = i / slidersPerRow;
        const int column = i % slidersPerRow;

        const float x = (float) (column * (sliderWidth + sliderDistance));
        const float y = topRowY + (float) row * ((float) sliderHeight + rowGap);

        layout.add(*sliders[i], x, y, (float) sliderWidth, (float) sliderHeight);
    }
}

void AdvancedParameterControl::paint(juce::Graphics & g)
{
    const float scale = CustomFontLookAndFeel::getScale();

    g.setColour(juce::Colour {0xff181819});
    g.fillRect(topLine1 * scale);
    g.fillRect(topLine2 * scale);
    g.fillRect(topLine3 * scale);
    g.fillRect(topLine4 * scale);
    g.fillRect(bottomLine1 * scale);
    g.fillRect(bottomLine2 * scale);

    g.setFont(getFont(FontType::bold, CustomFontLookAndFeel::scaled(12.f)));
    g.setColour(juce::Colour {0xff757677});
    g.drawText("Transient Control", topText1 * scale, juce::Justification::centred);
    g.drawText("Blend 1 / 2", topText2 * scale, juce::Justification::centred);
    g.drawText("Post Compressor", topText3 * scale, juce::Justification::centred);
    g.drawText("Global", topText4 * scale, juce::Justification::centred);
    g.drawText("Grain FX 1", bottomText1 * scale, juce::Justification::centred);
    g.drawText("Grain FX 2", bottomText2 * scale, juce::Justification::centred);
}
