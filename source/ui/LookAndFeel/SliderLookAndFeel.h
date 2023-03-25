//
// Created by valentin.ackva on 12.02.2023.
//

#ifndef VAESYNTH_SLIDERLOOKANDFEEL_H
#define VAESYNTH_SLIDERLOOKANDFEEL_H

#include "JuceHeader.h"

enum CustomSliderColourID {
    ellipseColourId,
    gradientColourTopId,
    gradientColourBottomId,
    sliderFillId
};

enum CustomSliderType {
    normal,
    crossfade
};

/****************************************************
 *              Linear Slider
*****************************************************/

class SliderLookAndFeel : public juce::LookAndFeel_V3 {
public:
    SliderLookAndFeel();
    void setCustomColour(CustomSliderColourID customSliderColourId, juce::Colour colourToUse);

private:
    void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos, float minSliderPos,
                          float maxSliderPos, const juce::Slider::SliderStyle, juce::Slider &slider) override;

private:
    juce::Colour ellipseColour {0xffC47EAD};
    juce::Colour gradientColourTop {0xff682D8C};
    juce::Colour gradientColourBottom {0xffEB1E79};
    juce::Colour sliderFillColour {juce::Colours::black};
    const float cornerRadius = 10;
};

/****************************************************
 *              Crossfade Slider
*****************************************************/

class CrossfadeSliderLookAndFeel : public juce::LookAndFeel_V3
{
public:
    CrossfadeSliderLookAndFeel();
    void setCustomColour(CustomSliderColourID customSliderColourId, juce::Colour colourToUse);
private:
    void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos, float minSliderPos,
                          float maxSliderPos, const juce::Slider::SliderStyle, juce::Slider &slider) override;

private:
    juce::Colour ellipseColour {0xffC47EAD};
    juce::Colour gradientColourTop {0xff682D8C};
    juce::Colour gradientColourBottom {0xffEB1E79};
    juce::Colour sliderFillColour {juce::Colours::black};
    const float cornerRadius = 10;
};


/****************************************************
 *              Linear Volume Slider
*****************************************************/

class CustomLinearVolumeSliderLookAndFeel : public juce::LookAndFeel_V4 {
public:
    CustomLinearVolumeSliderLookAndFeel() {
        setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }
private:
    void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos, float minSliderPos,
                          float maxSliderPos, const juce::Slider::SliderStyle, juce::Slider &slider) override {

        juce::ignoreUnused(x, y, width, height, sliderPos, minSliderPos, maxSliderPos, slider);

        g.setColour(backgroundColour);
        g.fillRoundedRectangle(g.getClipBounds().toFloat(), cornerRadius);
    }

private:
    juce::Colour backgroundColour {0xff212121};
    const float cornerRadius = 6.5f;
};
#endif //VAESYNTH_SLIDERLOOKANDFEEL_H
