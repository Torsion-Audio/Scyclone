//
// Created by valentin.ackva on 22.03.2023.
//

#ifndef VAESYNTH_WARNINGWINDOW_H
#define VAESYNTH_WARNINGWINDOW_H

#include "JuceHeader.h"
#include "../../utils/colors.h"

enum WarningType {
    SampleRateWarning,
    SystemTooSlow
};

class WarningWindow {
public:
    WarningWindow() {

    }
    void showWarningWindow(WarningType type) {

        juce::String errorMessage;
        juce::String title;

        switch (type) {
            case SampleRateWarning:
                title = "Warning: unsupported sample rate";
                errorMessage = "This plugin is still in alpha. At the moment only a sample rate of 48kHz is supported.";
                break;
            case SystemTooSlow:
                title = "Warning: system load to high";
                errorMessage = "It seems that this system is not fast enough to process the audio data. Try to only use one network.";
                break;
        }

        juce::AlertWindow window {title, errorMessage, juce::MessageBoxIconType::NoIcon};
        setLookAndFeel(window);

        auto options = juce::MessageBoxOptions()
                .withTitle(title)
                .withMessage(errorMessage)
                .withIconType(juce::MessageBoxIconType::NoIcon)
                .withButton("OK") ;
        window.showAsync(options, nullptr);

    }

private:
    void setLookAndFeel(juce::AlertWindow& window) {
        auto& lookAndFeel = window.getLookAndFeel();

        lookAndFeel.setColour(juce::AlertWindow::ColourIds::outlineColourId, juce::Colour::fromString(ColorPallete::BG));
        lookAndFeel.setColour(juce::AlertWindow::ColourIds::backgroundColourId, juce::Colour::fromString(ColorPallete::BG));
        lookAndFeel.setColour(juce::AlertWindow::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
        lookAndFeel.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colour::fromString(ColorPallete::TEXT2));
        lookAndFeel.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colour::fromString(ColorPallete::TEXT2));
        lookAndFeel.setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colour::fromString(ColorPallete::BG));
        lookAndFeel.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colour::fromString(ColorPallete::BG));

        window.setLookAndFeel(&lookAndFeel);
    }
};

#endif //VAESYNTH_WARNINGWINDOW_H
