#include "FileChooserManager.h"

FileChooserManager::FileChooserManager(AudioPluginAudioProcessor& processor)
        : processorRef(processor)
{
    lastOpenedFolder = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userHomeDirectory);
}

// Generalized function, atm for single files only
void FileChooserManager::openFileChooser(const juce::String& dialogTitle,
                     const juce::File& initialDirectory,
                     const juce::String& filePatterns,
                     std::function<void(const juce::File&)> onValidFileChosenCallback)
{
    if (lastOpenedFolder.exists()) {
        dirToOpen = lastOpenedFolder;
    } else {
        dirToOpen = initialDirectory;;
    }

    fileChooser = std::make_unique<juce::FileChooser>(dialogTitle, dirToOpen, filePatterns, true);

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                             [this, filePatterns, onValidFileChosenCallback](const juce::FileChooser& chooser)
                             {
                                 juce::File chosen;
                                 auto results = chooser.getURLResults();

                                 // Check if the results are empty (user closed without selecting a file)
                                 if (results.isEmpty())
                                 {
                                     return;
                                 }

                                 for (const auto& result : results)
                                 {
                                     if (result.isLocalFile())
                                     {
                                         chosen = result.getLocalFile();
                                         lastOpenedFolder = chosen.getParentDirectory();
                                         break; // We only need one valid file
                                     }
                                     else {
                                         return;
                                     }
                                 }

                                 // Check if the file is valid (non-empty, correct extension based on filePatterns)
                                 if (chosen.getSize() != 0 && fileHasValidExtension(chosen, filePatterns))
                                     onValidFileChosenCallback(chosen);
                                 else
                                     warningWindow.showWarningWindow(UnsupportedFileType);
                             });
}

bool FileChooserManager::fileHasValidExtension(const juce::File& file, const juce::String& filePatterns)
{
    juce::StringArray validExtensions;
    validExtensions.addTokens(filePatterns, ";,", "*");

    for (auto& ext : validExtensions)
    {
        ext = ext.trimCharactersAtStart("*."); // Remove wildcard characters
        if (file.hasFileExtension(ext))
            return true;
    }

    return false;
}

// Use case: Network-specific file chooser
void FileChooserManager::openFileChooserForNetwork(int networkID)
{
    if (networkID != 1 && networkID != 2)
    {
        jassertfalse;
        return;
    }

    auto onFileChosen = [this, networkID](const juce::File& file)
    {
        processorRef.loadExternalModel(file.getFullPathName(), networkID);
    };

    openFileChooser("Choose a model file...",
                    juce::File::getSpecialLocation(juce::File::SpecialLocationType::userHomeDirectory),
                    "*.ort",
                    onFileChosen);
}