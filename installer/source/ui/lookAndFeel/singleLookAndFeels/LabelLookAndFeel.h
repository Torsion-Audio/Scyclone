//
// Created by valentin.ackva on 05.05.2023.
//

#ifndef GUI_APP_EXAMPLE_LABELLOOKANDFEEL_H
#define GUI_APP_EXAMPLE_LABELLOOKANDFEEL_H

#include "JuceHeader.h"

class CustomLabelLookAndFeel : public juce::LookAndFeel_V2 {
public:
    CustomLabelLookAndFeel() {

    }

    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        auto area = g.getClipBounds().toFloat().reduced(10);

        g.setColour(juce::Colour{0xff1C1C1D});
        g.drawRoundedRectangle(area, 10.f, 3.f);

        if (! label.isBeingEdited())
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f;
            const juce::Font font (getLabelFont (label));

            g.setColour (juce::Colour{0xff565656});
            g.setFont (font);

            auto textArea = g.getClipBounds().reduced(20);

            g.drawFittedText (label.getText(), textArea, juce::Justification::left,
                              juce::jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                              label.getMinimumHorizontalScale());

            g.setColour (label.findColour (juce::Label::outlineColourId).withMultipliedAlpha (alpha));
        }
    }

};

#endif //GUI_APP_EXAMPLE_LABELLOOKANDFEEL_H
