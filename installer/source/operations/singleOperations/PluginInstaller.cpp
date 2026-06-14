//
// Created by valentin.ackva on 25.06.2023.
//

#include "PluginInstaller.h"
#include <filesystem>

PluginInstaller::PluginInstaller() {
}

void PluginInstaller::install(const File &sourceDirectory,
                              std::vector<std::pair<FormatOptions::Options, juce::String>> config) {

    for (auto & i : config) {
        auto targetDirectory = juce::File{i.second};
        if (!targetDirectory.exists()) {
            targetDirectory.createDirectory();
        }

        auto targetType = i.first;
        if (targetType == FormatOptions::Options::VST3Plugin) {
            vst3Target = std::make_unique<juce::File>(targetDirectory);
        } else if (targetType == FormatOptions::Options::Standalone) {
            standaloneTarget = std::make_unique<juce::File>(targetDirectory);
        } else if (targetType == FormatOptions::Options::AUPlugin) {
            auTarget = std::make_unique<juce::File>(targetDirectory);
        }
    }

    source = sourceDirectory;
    copyFiles();
}

void PluginInstaller::reset() {
    standaloneTarget = nullptr;
    vst3Target = nullptr;
    auTarget = nullptr;
}

void PluginInstaller::copyFiles() {
    for (const auto& entry : source.findChildFiles(juce::File::findFilesAndDirectories, false))
    {
        DBG(entry.getFileName());
#if JUCE_MAC
        if (entry.getFileExtension() == ".app" && standaloneTarget != nullptr) {
            auto targetFile = standaloneTarget->getChildFile(entry.getFileName());
            entry.copyFileTo(targetFile);
        }
#elif JUCE_WINDOWS
        if (entry.getFileExtension() == ".exe" && standaloneTarget != nullptr) {
            auto targetFile = standaloneTarget->getChildFile(entry.getFileName());
            entry.copyFileTo(targetFile);
        }
#elif JUCE_LINUX
        if (entry.getFileExtension().isEmpty() && entry.existsAsFile() && standaloneTarget != nullptr) {
            auto targetFile = standaloneTarget->getChildFile(entry.getFileName());
            entry.copyFileTo(targetFile);
        }
#endif
        if (entry.getFileExtension() == ".vst3" && vst3Target != nullptr) {
            auto targetFile = vst3Target->getChildFile(entry.getFileName());
#if JUCE_LINUX
            std::filesystem::copy(
                entry.getFullPathName().toStdString(),
                targetFile.getFullPathName().toStdString(),
                std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing
            );
#else
            entry.copyFileTo(targetFile);
#endif
        } else if (entry.getFileExtension() == ".component" && auTarget != nullptr) {
            auto targetFile = auTarget->getChildFile(entry.getFileName());
            entry.copyFileTo(targetFile);
        }
    }
    notifyFinished(this, true);
}
