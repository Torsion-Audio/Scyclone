//
// Created by valentin.ackva on 22.06.2023.
//

#ifndef GUI_APP_EXAMPLE_DOWNLOADER_H
#define GUI_APP_EXAMPLE_DOWNLOADER_H

#include "JuceHeader.h"
#include "UtilClass.h"

class FileDownload : juce::URL::DownloadTask::Listener, public UtilClass
{
public:
    FileDownload();
    ~FileDownload();

    void downloadFile(juce::URL downloadURL, const juce::File destinationPath);
    void stopDownload();

    int getDownloadProgress();
private:
    void finished (juce::URL::DownloadTask*, bool success) override;

    std::unique_ptr<juce::URL::DownloadTask> downloadTask;

    int lastDownloadPercentage = 0;
};

#endif //GUI_APP_EXAMPLE_DOWNLOADER_H
