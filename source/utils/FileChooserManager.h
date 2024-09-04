#pragma once
#include <JuceHeader.h>
#include "FileChooserManager.h"
#include "PluginProcessor.h"

class FileChooserManager
{
public:
    FileChooserManager(AudioPluginAudioProcessor& processor);

    // Generalized file chooser function, atm for single files only
    void openFileChooser(const juce::String& dialogTitle,
                         const juce::File& initialDirectory,
                         const juce::String& filePatterns,
                         std::function<void(const juce::File&)> onValidFileChosenCallback);

    bool fileHasValidExtension(const juce::File& file, const juce::String& filePatterns);

    // Network-specific file chooser function, using openFileChooser
    void openFileChooserForNetwork(int networkID);

private:
    AudioPluginAudioProcessor& processorRef;

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File dirToOpen;
    juce::File lastOpenedFolder;

    WarningWindow warningWindow;
};
