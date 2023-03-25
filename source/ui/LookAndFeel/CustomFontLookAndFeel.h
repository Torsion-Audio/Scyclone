//
// Created by valentin.ackva on 17.03.2023.
//

#ifndef VAESYNTH_CUSTOMFONTLOOKANDFEEL_H
#define VAESYNTH_CUSTOMFONTLOOKANDFEEL_H

#include "JuceHeader.h"

class CustomFontLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomFontLookAndFeel()
    {
        // without this custom Fonts won't work!!
        LookAndFeel::setDefaultLookAndFeel (this);

        // This can be used as one way of setting a default font
        // setDefaultSansSerifTypeface (getCustomFont().getTypeface());
    }

    static const juce::Font getCustomFont()
    {
        static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::InterRegular_ttf, BinaryData::InterRegular_ttfSize);
        return juce::Font (typeface);
    }

    static const juce::Font getCustomFontBold()
    {
        static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::InterBold_ttf, BinaryData::InterBold_ttfSize);
        return juce::Font (typeface);
    }

    static const juce::Font getCustomFontMedium()
    {
        static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::InterMedium_ttf, BinaryData::InterMedium_ttfSize);
        return juce::Font (typeface);
    }

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font& f) override
    {
        if (f.isBold()) {
            return getCustomFontBold().getTypefacePtr();
        } else {
            return getCustomFont().getTypefacePtr();
        }
    }
private:
};

#endif //VAESYNTH_CUSTOMFONTLOOKANDFEEL_H
