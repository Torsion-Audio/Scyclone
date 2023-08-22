//
// Created by valentin.ackva on 22.06.2023.
//

#ifndef GUI_APP_EXAMPLE_OPERATIONMANAGER_H
#define GUI_APP_EXAMPLE_OPERATIONMANAGER_H

#include "singleOperations/FileDownload.h"
#include "singleOperations/RepoInspector.h"
#include "singleOperations/ZipExtractor.h"
#include "singleOperations/PluginInstaller.h"
#include "singleOperations/UtilClass.h"

enum InstallationState {
    Startup,
    GitHubAccessed,
    DownloadFinished,
    ExtractionFinished,
    InstallationFinished
};

enum ErrorCode {
    GitHubNotAccessible,
    InvalidDownloadLink
};

class OperationManager : private UtilClass::Listener, private juce::Thread {
public:
    OperationManager();
    ~OperationManager() override;

    void configure(std::vector<std::pair<FormatOptions::Options, juce::String>> config);

    void start();

    void stop();

    InstallationState getState();

    int getDownloadProgress();

private:
    void run() override;
    void processFinished(UtilClass* reference, bool status) override;
    void repoInspectorFinished(bool status);
    static void errorOccurred(ErrorCode errorCode);
    void changeState (InstallationState newState);

private:
    InstallationState state = Startup;

    const std::string repoURL = "https://api.github.com/repos/Torsion-Audio/Scyclone/releases";

    struct DownloadObject {
        juce::File targetFile;
        juce::URL sourceURL;
        juce::File extractedFolder;
    };
    std::unique_ptr<DownloadObject> latestScyclone;

    std::vector<std::pair<FormatOptions::Options, juce::String>> installConfig;

    RepoInspector repoInspector;
    FileDownload fileDownload;
    ZipExtractor zipExtractor;
    PluginInstaller pluginManager;
};



#endif //GUI_APP_EXAMPLE_OPERATIONMANAGER_H
