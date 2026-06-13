//
// Created by valentin.ackva on 22.03.2023.
//

#ifndef VAESYNTH_WARNINGWINDOW_H
#define VAESYNTH_WARNINGWINDOW_H

#include "JuceHeader.h"

enum WarningType
{
    SampleRateWarning,
    SystemTooSlow,
    UnsupportedFileType
};

class WarningWindow
{
public:
    void showWarningWindow(WarningType type, const juce::String& expectedFilePatterns = {})
    {
        juce::String errorMessage;
        juce::String title;

        switch (type)
        {
        case SampleRateWarning:
            title = "Warning: unsupported sample rate";
            errorMessage = "This plugin is still in alpha. At the moment only a sample rate of 48kHz is supported.";
            break;
        case SystemTooSlow:
            title = "Warning: system load too high";
            errorMessage = "It seems that this system is not fast enough to process the audio data. Try to only use one network.";
            break;
        case UnsupportedFileType:
            title = "Could not load file";
            errorMessage = makeUnsupportedFileMessage(expectedFilePatterns);
            break;
        }

        juce::AlertWindow::showAsync(
            juce::MessageBoxOptions()
                .withTitle(title)
                .withMessage(errorMessage)
                .withIconType(juce::MessageBoxIconType::NoIcon)
                .withButton("OK"),
            nullptr);
    }

private:
    static juce::String formatExpectedExtensions(const juce::String& filePatterns)
    {
        juce::StringArray extensions;
        extensions.addTokens(filePatterns, ";,", "*");

        juce::StringArray formatted;

        for (auto ext : extensions)
        {
            ext = ext.trim().trimCharactersAtStart("*.");

            if (ext.isNotEmpty())
                formatted.add("." + ext);
        }

        return formatted.joinIntoString(" or ");
    }

    static juce::String makeUnsupportedFileMessage(const juce::String& expectedFilePatterns)
    {
        const auto extensions = formatExpectedExtensions(expectedFilePatterns);

        if (extensions.isNotEmpty())
            return "Please choose a valid " + extensions + " file. Shortcuts and other file types cannot be loaded.";

        return "Please choose a valid file with a supported extension. Shortcuts and other file types cannot be loaded.";
    }
};

#endif // VAESYNTH_WARNINGWINDOW_H
