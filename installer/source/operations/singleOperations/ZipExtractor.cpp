//
// Created by valentin.ackva on 22.06.2023.
//

#include "ZipExtractor.h"

void ZipExtractor::setSourceFile(const juce::File& pathToFile)
{
    zipFile = std::make_unique<juce::File>(pathToFile);
    DBG("ZipExtractor::setSourceFile: " + pathToFile.getFullPathName());
}

void ZipExtractor::extractTo(const juce::File &destinationDirectory, bool shouldOverwriteExistingFiles) {
    if (!destinationDirectory.exists())
    {
        bool directoryCreated = destinationDirectory.createDirectory();
        if (!directoryCreated)
        {
            notifyFinished(this, false);
            return;
        }
    }

    juce::ZipFile zip(*zipFile);

    juce::Result result = zip.uncompressTo(destinationDirectory, shouldOverwriteExistingFiles);

    if (result.wasOk()) {
        notifyFinished(this, true);
    } else {
        notifyFinished(this, false);
    }
}