//
// Created by valentin.ackva on 24.02.2023.
//

#ifndef VAESYNTH_CUSTOMLABELLOOKANDFEEL_H
#define VAESYNTH_CUSTOMLABELLOOKANDFEEL_H

#include "JuceHeader.h"

class CustomLabelLookAndFeel : public juce::LookAndFeel_V2 {
public:
    CustomLabelLookAndFeel() {

    }

    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        g.setColour(label.findColour (juce::Label::backgroundColourId));
        g.fillRoundedRectangle(g.getClipBounds().toFloat(), 6.5f);

        if (! label.isBeingEdited())
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f;
            const juce::Font font (getLabelFont (label));

            g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
            g.setFont (font);

            auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

            g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                              juce::jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                              label.getMinimumHorizontalScale());

            g.setColour (label.findColour (juce::Label::outlineColourId).withMultipliedAlpha (alpha));
        }
        else if (label.isEnabled())
        {
            g.setColour (label.findColour (juce::Label::outlineColourId));
        }

        g.drawRect (label.getLocalBounds());
    }

};

#endif //VAESYNTH_CUSTOMLABELLOOKANDFEEL_H
