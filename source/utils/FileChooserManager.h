#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WarningWindow.h"

class FileChooserManager
{
public:
    FileChooserManager(AudioPluginAudioProcessor &processor);
    ~FileChooserManager();

    // Generalized file chooser function, atm for single files only
    void openFileChooser(const juce::String &dialogTitle,
                         const juce::File &initialDirectory,
                         const juce::String &filePatterns,
                         std::function<void(const juce::File &)> onValidFileChosenCallback,
                         juce::Component *parentComponent = nullptr);

    // Network-specific file chooser function, using openFileChooser
    void openFileChooserForNetwork(int networkID, juce::Component *parentComponent = nullptr);

private:
    AudioPluginAudioProcessor &processorRef;

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File dirToOpen;
    juce::File lastOpenedFolder;

    WarningWindow warningWindow;

    std::shared_ptr<std::atomic<bool>> callbackAlive;
    std::shared_ptr<std::atomic<bool>> chooserOpen;
};
