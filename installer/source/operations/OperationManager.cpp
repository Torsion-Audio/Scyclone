//
// Created by valentin.ackva on 22.06.2023.
//

#include "OperationManager.h"

OperationManager::OperationManager() : Thread("OperationManager") {
    repoInspector.addListener(this);
    fileDownload.addListener(this);
    zipExtractor.addListener(this);
    pluginManager.addListener(this);
}

OperationManager::~OperationManager() {
    repoInspector.removeListener(this);
    fileDownload.removeListener(this);
    zipExtractor.removeListener(this);
    pluginManager.removeListener(this);
    stop();
}

void OperationManager::configure(std::vector<std::pair<FormatOptions::Options, juce::String>> config) {
    installConfig.clear();
    for (const auto& conf : config) {
        installConfig.push_back(conf);
    }
}

void OperationManager::start() {
    startThread();
}

void OperationManager::stop() {
    stopThread(1000);
    fileDownload.stopDownload();
    pluginManager.reset();
}

InstallationState OperationManager::getState() {
    return state;
}

int OperationManager::getDownloadProgress() {
    return fileDownload.getDownloadProgress();
}

void OperationManager::run() {
    changeState(Startup);
}

void OperationManager::processFinished(UtilClass *reference, bool status) {
    if (!status) {
        stop();
        return;
    }

    if (auto* inspector = dynamic_cast<RepoInspector*>(reference)) {
        repoInspectorFinished(status);
    } else if (auto* downloader = dynamic_cast<FileDownload*>(reference)) {
        changeState(DownloadFinished);
    } else if (auto* extractor = dynamic_cast<ZipExtractor*>(reference)) {
        changeState(ExtractionFinished);
    } else if (auto *installer = dynamic_cast<PluginInstaller*>(reference)) {
        changeState(InstallationFinished);
    }
}

void OperationManager::repoInspectorFinished(bool status) {
    auto [downloadVersion, downloadUrl] = repoInspector.getLatestVersion();

    if (status && !downloadUrl.isEmpty()) {
        juce::String downloadFileName = juce::String{"Scyclone-"} + downloadVersion + juce::String{".zip"};
        juce::File targetFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory).getChildFile(downloadFileName);
        auto folderName = targetFile.getFileNameWithoutExtension();
        juce::File targetDirectory = targetFile.getParentDirectory().getChildFile(folderName);

        latestScyclone = std::make_unique<DownloadObject>(DownloadObject{targetFile, downloadUrl, targetDirectory});
        changeState(GitHubAccessed);
    } else {
        errorOccurred(GitHubNotAccessible);
    }
}

void OperationManager::errorOccurred(ErrorCode errorCode) {
    switch (errorCode) {
        case GitHubNotAccessible:
            std::cout << "GitHub not accessible" << std::endl;
            break;
        case InvalidDownloadLink:
            std::cout << "Invalid download link" << std::endl;
            break;
    }
}

void OperationManager::changeState(InstallationState newState) {
    if (state != newState || newState == Startup)
    {
        state = newState;

        if (state == Startup) {
            repoInspector.inspectURL(repoURL);
        } else if (state == GitHubAccessed) {
            fileDownload.downloadFile(latestScyclone->sourceURL, latestScyclone->targetFile);
        } else if (state == DownloadFinished) {
            zipExtractor.setSourceFile(latestScyclone->targetFile);
            zipExtractor.extractTo(latestScyclone->extractedFolder);
        } else if (state == ExtractionFinished) {
            pluginManager.install(latestScyclone->extractedFolder, installConfig);
        } else {
            stop();
        }
    }
}
