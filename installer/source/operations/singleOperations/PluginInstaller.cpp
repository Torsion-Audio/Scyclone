//
// Created by valentin.ackva on 25.06.2023.
//

#include "PluginInstaller.h"

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
    for (const auto& entry : source.findChildFiles(juce::File::findFiles, true))
    {
        std::cout << entry.getFileName() << std::endl;
        if (JUCE_MAC) {
            if (entry.getFileExtension() == ".app" && standaloneTarget != nullptr) {
            auto targetFile = standaloneTarget->getChildFile(entry.getFileName());
            entry.copyFileTo(targetFile);
            }
        }
        else {
            if (entry.getFileExtension() == ".exe" && standaloneTarget != nullptr) {
                auto targetFile = standaloneTarget->getChildFile(entry.getFileName());
                entry.copyFileTo(targetFile);
            }
        } 
        if (entry.getFileExtension() == ".vst3" && vst3Target != nullptr) {
            auto targetFile = vst3Target->getChildFile(entry.getFileName());
            entry.copyFileTo(targetFile);
        } else if (entry.getFileExtension() == ".component" && auTarget != nullptr) {
            auto targetFile = auTarget->getChildFile(entry.getFileName());
            entry.copyFileTo(targetFile);
        }
    }
    notifyFinished(this, true);
}
