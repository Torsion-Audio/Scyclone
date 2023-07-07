//
// Created by valentin.ackva on 25.06.2023.
//

#ifndef GUI_APP_EXAMPLE_PLUGININSTALLER_H
#define GUI_APP_EXAMPLE_PLUGININSTALLER_H


#include "JuceHeader.h"
#include "../../utils/FormatOptions.h"
#include "UtilClass.h"

class PluginInstaller : public UtilClass
{
public:
    PluginInstaller();

    void install (const juce::File& sourceDirectory, std::vector<std::pair<FormatOptions::Options, juce::String>> config);
    void reset();

private:
    void copyFiles();

private:
    juce::File source;
    std::unique_ptr<juce::File> standaloneTarget;
    std::unique_ptr<juce::File> vst3Target;
    std::unique_ptr<juce::File> auTarget;
};

#endif //GUI_APP_EXAMPLE_PLUGININSTALLER_H
