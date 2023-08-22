//
// Created by valentin.ackva on 22.06.2023.
//

#include "FileDownload.h"

FileDownload::FileDownload () {

}

FileDownload::~FileDownload() {
    downloadTask.reset();
}

void FileDownload::stopDownload() {
    downloadTask = nullptr;
}

int FileDownload::getDownloadProgress() {
    if (downloadTask == nullptr) {
        return 0;
    }
    auto progress = (float) downloadTask->getLengthDownloaded() / (float) downloadTask->getTotalLength();
    return (int) (progress * 100);
}

void FileDownload::downloadFile(juce::URL downloadURL, const juce::File destinationPath) {
    if (destinationPath.exists()) {
        destinationPath.deleteFile();
    }

    downloadTask = downloadURL.downloadToFile(destinationPath, URL::DownloadTaskOptions().withListener(this));

    if (downloadTask == nullptr) {
        notifyFinished(this, false);
    }
}

void FileDownload::finished (juce::URL::DownloadTask*, bool success) {
    notifyFinished(this, success);
}