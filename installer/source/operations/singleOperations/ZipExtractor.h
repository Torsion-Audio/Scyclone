//
// Created by valentin.ackva on 22.06.2023.
//

#ifndef GUI_APP_EXAMPLE_ZIPEXTRACTOR_H
#define GUI_APP_EXAMPLE_ZIPEXTRACTOR_H

#include "JuceHeader.h"
#include "UtilClass.h"
class ZipExtractor : public UtilClass {
public:
    ZipExtractor() = default;

    void setSourceFile(const juce::File& pathToFile);
    void extractTo(const juce::File& destinationDirectory, bool shouldOverwriteExistingFiles = true);

private:
    std::unique_ptr<juce::File> zipFile;
};


#endif //GUI_APP_EXAMPLE_ZIPEXTRACTOR_H
