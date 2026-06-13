#include "FileChooserManager.h"

namespace
{
    struct ChooserOpenGuard
    {
        std::shared_ptr<std::atomic<bool>> flag;

        explicit ChooserOpenGuard(std::shared_ptr<std::atomic<bool>> openFlag)
            : flag(std::move(openFlag))
        {
        }

        ~ChooserOpenGuard()
        {
            if (flag != nullptr)
                flag->store(false);
        }
    };

    // Clears chooserOpen if launchAsync never invokes its callback (e.g. FileChooser destroyed early).
    struct ChooserOpenSession
    {
        std::shared_ptr<std::atomic<bool>> flag;
        bool callbackStarted = false;

        explicit ChooserOpenSession(std::shared_ptr<std::atomic<bool>> openFlag)
            : flag(std::move(openFlag))
        {
        }

        void onCallbackStarted()
        {
            callbackStarted = true;
        }

        ~ChooserOpenSession()
        {
            if (! callbackStarted && flag != nullptr)
                flag->store(false);
        }
    };

    bool isPlatformShortcut(const juce::File& file)
    {
       #if JUCE_WINDOWS
        return file.isShortcut();
       #else
        return false;
       #endif
    }

    juce::File getChosenLocalFile(const juce::FileChooser& chooser)
    {
        for (const auto& result : chooser.getURLResults())
        {
            if (result.isLocalFile())
                return result.getLocalFile();
        }

        return {};
    }

    bool fileHasValidExtension(const juce::File& file, const juce::String& filePatterns)
    {
        juce::StringArray validExtensions;
        validExtensions.addTokens(filePatterns, ";,", "*");

        for (auto& ext : validExtensions)
        {
            ext = ext.trimCharactersAtStart("*.");
            if (file.hasFileExtension(ext))
                return true;
        }

        return false;
    }

    bool isSupportedFile(const juce::File& file, const juce::String& filePatterns)
    {
        return file.existsAsFile()
            && file.getSize() > 0
            && ! isPlatformShortcut(file)
            && fileHasValidExtension(file, filePatterns);
    }

    bool isDestroyed(const std::shared_ptr<std::atomic<bool>>& alive,
                     const std::shared_ptr<std::atomic<bool>>& open)
    {
        if (alive->load())
            return false;

        open->store(false);
        return true;
    }

    void handleAsyncFileChooserResult(const juce::FileChooser& chooser,
                                      const std::shared_ptr<std::atomic<bool>>& alive,
                                      const std::shared_ptr<std::atomic<bool>>& open,
                                      const std::shared_ptr<ChooserOpenSession>& session,
                                      const juce::String& filePatterns,
                                      const std::function<void(const juce::File&)>& onValidFileChosenCallback,
                                      juce::File& lastOpenedFolder,
                                      WarningWindow& warningWindow)
    {
        if (isDestroyed(alive, open))
            return;

        session->onCallbackStarted();
        ChooserOpenGuard chooserGuard { open };

        if (chooser.getURLResults().isEmpty())
            return;

        const auto chosen = getChosenLocalFile(chooser);

        if (isDestroyed(alive, open))
            return;

        if (chosen.existsAsFile())
            lastOpenedFolder = chosen.getParentDirectory();

        if (isDestroyed(alive, open))
            return;

        if (isSupportedFile(chosen, filePatterns))
            onValidFileChosenCallback(chosen);
        else
            warningWindow.showWarningWindow(UnsupportedFileType, filePatterns);
    }
}

FileChooserManager::FileChooserManager(AudioPluginAudioProcessor& processor)
        : processorRef(processor),
          callbackAlive(std::make_shared<std::atomic<bool>>(true)),
          chooserOpen(std::make_shared<std::atomic<bool>>(false))
{
    lastOpenedFolder = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userHomeDirectory);
}

FileChooserManager::~FileChooserManager()
{
    callbackAlive->store(false);
    chooserOpen->store(false);
}

// Generalized function, atm for single files only
void FileChooserManager::openFileChooser(const juce::String& dialogTitle,
                                         const juce::File& initialDirectory,
                                         const juce::String& filePatterns,
                                         std::function<void(const juce::File&)> onValidFileChosenCallback)
{
    if (chooserOpen->exchange(true))
        return;

    auto session = std::make_shared<ChooserOpenSession>(chooserOpen);

    if (lastOpenedFolder.exists())
        dirToOpen = lastOpenedFolder;
    else
        dirToOpen = initialDirectory;

    try
    {
        fileChooser = std::make_unique<juce::FileChooser>(dialogTitle, dirToOpen, filePatterns, true);

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                 [alive = callbackAlive,
                                  open = chooserOpen,
                                  session,
                                  filePatterns,
                                  onValidFileChosenCallback,
                                  lastOpenedFolder = std::ref(lastOpenedFolder),
                                  warningWindow = std::ref(warningWindow)](const juce::FileChooser& chooser)
                                 {
                                     handleAsyncFileChooserResult(chooser,
                                                                  alive,
                                                                  open,
                                                                  session,
                                                                  filePatterns,
                                                                  onValidFileChosenCallback,
                                                                  lastOpenedFolder.get(),
                                                                  warningWindow.get());
                                 });
    }
    catch (...)
    {
        session->onCallbackStarted();
        chooserOpen->store(false);
        throw;
    }
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
