//
// Created by valentin.ackva on 12.02.2023.
//

#include "SliderLookAndFeel.h"

/****************************************************
 *              Linear Slider
****************************************************/
SliderLookAndFeel::SliderLookAndFeel() {
    auto transparentColour = juce::Colours::transparentWhite;
    setColour (juce::Slider::ColourIds::backgroundColourId, transparentColour);
    setColour (juce::Slider::ColourIds::textBoxOutlineColourId, transparentColour);
    setColour (juce::Slider::ColourIds::rotarySliderOutlineColourId, transparentColour);
    setColour (juce::Slider::ColourIds::textBoxTextColourId, transparentColour);
}

void SliderLookAndFeel::setCustomColour(CustomSliderColourID customSliderColourId, juce::Colour colourToUse) {
    switch (customSliderColourId) {
        case ellipseColourId: ellipseColour = colourToUse; break;
        case gradientColourTopId: gradientColourTop = colourToUse; break;
        case gradientColourBottomId: gradientColourBottom = colourToUse; break;
        case sliderFillId: sliderFillColour = colourToUse; break;
    }
}

void SliderLookAndFeel::drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                                         float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle,
                                         juce::Slider &slider) {

    juce::ignoreUnused(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, slider);

    auto areaSlider = juce::Rectangle<float> {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)};

    // Fill Background
    const float xRectBg = areaSlider.getX() + 0.5f;
    const float yRectBg = areaSlider.getY();
    const float wRectBg = areaSlider.getWidth() - 1.0f;
    const float hRectBg = areaSlider.getHeight();

    auto rectBackground = juce::Rectangle<float> (xRectBg, yRectBg, wRectBg, hRectBg);

    juce::ColourGradient fillColour {gradientColourTop,
                                     juce::Point<float>(xRectBg, yRectBg),
                                     gradientColourBottom,
                                     juce::Point<float>(xRectBg + wRectBg, yRectBg + hRectBg),
                                     true
    };
    g.setGradientFill(fillColour);
    g.fillRoundedRectangle(rectBackground, cornerRadius);


    // Fill Slider
    const float radius = 2 * cornerRadius;
    const float xRectFill = areaSlider.getX() + 0.5f;
    const float yRectFill = sliderPos;
    const float wRectFill = areaSlider.getWidth() - 1.0f;
    const float hRectFill = areaSlider.getY() + ((float) areaSlider.getHeight() - sliderPos);


    if (hRectBg - hRectFill < cornerRadius) {
        auto rectFill = juce::Rectangle<float> (xRectFill, yRectFill, wRectFill, hRectFill);
        g.setColour (sliderFillColour);
        g.setOpacity(0.3f);
        g.fillRoundedRectangle(rectFill, cornerRadius);
    }
    else if (hRectBg - hRectFill > hRectBg - cornerRadius) {
        auto rectFill = juce::Rectangle<float> (xRectFill + (cornerRadius / 4), yRectFill, wRectFill - (cornerRadius / 2), hRectFill);
        g.setColour (sliderFillColour);
        g.setOpacity(0.3f);
        g.fillRoundedRectangle(rectFill, cornerRadius);
    } else {
        juce::Path p;
        p.startNewSubPath (xRectFill, yRectFill);
        p.lineTo (xRectFill + wRectFill, yRectFill);
        p.lineTo (xRectFill + wRectFill, yRectFill + hRectFill - radius);
        p.addArc (xRectFill + wRectFill - radius, yRectFill + hRectFill - radius, radius, radius, juce::MathConstants<float>::halfPi, juce::MathConstants<float>::pi);
        p.lineTo (xRectFill + radius, yRectFill + hRectFill);
        p.addArc (xRectFill, yRectFill + hRectFill - radius, radius, radius, juce::MathConstants<float>::pi, juce::MathConstants<float>::pi * 1.5f);
        p.lineTo (xRectFill, yRectFill);
        p.closeSubPath();
        g.setColour (sliderFillColour);
        g.setOpacity(0.3f);
        g.fillPath (p);
    }

    // Draw Ellipse
    juce::Rectangle<float> areaEllipse(xRectBg, yRectBg + sliderPos - (wRectBg / 2.f),wRectBg,wRectBg);
    g.setColour(ellipseColour);
    g.drawEllipse(areaEllipse.reduced( wRectBg * 0.4f), wRectBg * 0.03f);
}



/****************************************************
 *              Crossfade Slider
****************************************************/
CrossfadeSliderLookAndFeel::CrossfadeSliderLookAndFeel()
{
    auto transparentColour = juce::Colours::transparentWhite;
    setColour (juce::Slider::ColourIds::backgroundColourId, transparentColour);
    setColour (juce::Slider::ColourIds::textBoxOutlineColourId, transparentColour);
    setColour (juce::Slider::ColourIds::rotarySliderOutlineColourId, transparentColour);
    setColour (juce::Slider::ColourIds::textBoxTextColourId, transparentColour);
}

void CrossfadeSliderLookAndFeel::setCustomColour(CustomSliderColourID customSliderColourId, juce::Colour colourToUse)
{
    switch (customSliderColourId) {
        case ellipseColourId: ellipseColour = colourToUse; break;
        case gradientColourTopId: gradientColourTop = colourToUse; break;
        case gradientColourBottomId: gradientColourBottom = colourToUse; break;
        case sliderFillId: sliderFillColour = colourToUse; break;
    }
}

void CrossfadeSliderLookAndFeel::drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                                             float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle,
                                             juce::Slider &slider)
{
    juce::ignoreUnused(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, slider);

    auto areaSlider = juce::Rectangle<float> {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)};

    // Fill Background
    const float xRectBg = areaSlider.getX() + 0.5f;
    const float yRectBg = areaSlider.getY();
    const float wRectBg = areaSlider.getWidth() - 1.0f;
    const float hRectBg = areaSlider.getHeight();

    auto rectBackground = juce::Rectangle<float> (xRectBg, yRectBg, wRectBg, hRectBg);

    juce::ColourGradient fillColour {gradientColourTop,
                                     juce::Point<float>(xRectBg, yRectBg),
                                     gradientColourBottom,
                                     juce::Point<float>(xRectBg + wRectBg, yRectBg + hRectBg),
                                     true
    };
    g.setGradientFill(fillColour);
    g.fillRoundedRectangle(rectBackground, cornerRadius);


    // Fill Slider
    const float radius = cornerRadius;
    const float xRectFill = areaSlider.getX() + 0.5f;
    const float yRectFill = sliderPos;
    const float wRectFill = areaSlider.getWidth() - 1.0f;
    const float hRectFill = areaSlider.getY() + ((float) areaSlider.getHeight() - sliderPos);

    juce::ignoreUnused(hRectFill);

    juce::Path p;
    if (sliderPos >= (float)hRectBg/2)
    {
        /*
        if (hRectBg - hRectFill < radius) {
            //TODO slider is not inside background shape at top
            p.startNewSubPath(xRectFill, yRectFill);
            p.lineTo(xRectFill + wRectFill, yRectFill);
            p.lineTo(xRectFill + wRectFill, yRectFill + hRectFill - radius);
            p.addArc(xRectFill + wRectFill - radius, yRectFill + hRectFill - radius, radius, radius,
                     juce::MathConstants<float>::halfPi, juce::MathConstants<float>::pi);
            p.lineTo(xRectFill + radius, yRectFill + hRectFill);
            p.addArc(xRectFill, yRectFill + hRectFill - radius, radius, radius, juce::MathConstants<float>::pi,
                     juce::MathConstants<float>::pi * 1.5f);
            p.lineTo(xRectFill, yRectFill);
        } else if (hRectBg - hRectFill > hRectBg - radius) {
            //TODO slider is not inside background shape at bottom
            p.startNewSubPath(xRectFill, yRectFill);
            p.lineTo(xRectFill + wRectFill, yRectFill);
            p.lineTo(xRectFill + wRectFill, yRectFill + hRectFill - radius);
            p.addArc(xRectFill + wRectFill - radius, yRectFill + hRectFill - radius, radius, radius,
                     juce::MathConstants<float>::halfPi, juce::MathConstants<float>::pi);
            p.lineTo(xRectFill + radius, yRectFill + hRectFill);
            p.addArc(xRectFill, yRectFill + hRectFill - radius, radius, radius, juce::MathConstants<float>::pi,
                     juce::MathConstants<float>::pi * 1.5f);
            p.lineTo(xRectFill, yRectFill);
        } else {
         */
            p.startNewSubPath(xRectFill, (float) hRectBg / 2);
            p.lineTo(xRectFill + wRectFill, (float) hRectBg / 2);
            p.lineTo(xRectFill + wRectFill, yRectFill + radius);
            p.addArc(xRectFill + wRectFill - radius, yRectFill - radius, radius, radius,
                     juce::MathConstants<float>::pi / 2, juce::MathConstants<float>::pi);
            p.lineTo(xRectFill + radius, yRectFill);
            p.addArc(xRectFill, yRectFill - radius, radius, radius, juce::MathConstants<float>::pi,
                     3 * juce::MathConstants<float>::pi / 2);
            p.lineTo(xRectFill, (float) hRectBg / 2);
        //}
    }
    else
    {
        p.startNewSubPath(xRectFill, (float)hRectBg/2);
        p.lineTo(xRectFill, yRectFill - radius);
        p.addArc(xRectFill, yRectFill, radius, radius, 3*juce::MathConstants<float>::pi/2, 2*juce::MathConstants<float>::pi);
        p.lineTo(xRectFill + wRectBg - radius, yRectFill);
        p.addArc(xRectFill + wRectBg - radius, yRectFill, radius, radius, 0, juce::MathConstants<float>::pi/2);
        p.lineTo(xRectFill + wRectBg, (float)hRectBg/2);
    }
    p.closeSubPath();
    g.setColour (sliderFillColour);
    g.setOpacity(0.3f);
    g.fillPath (p);

    // Draw Ellipse
    juce::Rectangle<float> areaEllipse(xRectBg, yRectBg + sliderPos - (wRectBg / 2.f),wRectBg,wRectBg);
    g.setColour(ellipseColour);
    g.drawEllipse(areaEllipse.reduced( wRectBg * 0.4f), wRectBg * 0.03f);
}
