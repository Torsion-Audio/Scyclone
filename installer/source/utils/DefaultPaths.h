//
// Created by valentin.ackva on 06.07.2023.
//

#ifndef GUI_APP_EXAMPLE_DEFAULTPATHS_H
#define GUI_APP_EXAMPLE_DEFAULTPATHS_H

#include "JuceHeader.h"
#include "FormatOptions.h"
#include "OperatingSystem.h"

using OS = OperatingSystem;

class DefaultPaths {
public:
    static juce::File getDestinationPath(FormatOptions::Options format)
    {
        OS::SystemType systemType = OS::getOperatingSystem();

        juce::File destinationPath;

        switch (format)
        {
            case FormatOptions::AUPlugin:
                destinationPath = getAUPluginPath(systemType);
                break;
            case FormatOptions::VST3Plugin:
                destinationPath = getVST3PluginPath(systemType);
                break;
            case FormatOptions::Standalone:
                destinationPath = getStandalonePath(systemType);
                break;
        }

        return destinationPath;
    }

private:
    static juce::File getAUPluginPath(OS::SystemType systemType) {
        if (systemType == OS::MacOS_x64 || systemType == OS::MacOS_arm64) {
            return juce::File("~/Library/Audio/Plug-Ins/Components/");
        } else {
            return juce::File("");
        }
    }

    static juce::File getVST3PluginPath(OperatingSystem::SystemType systemType) {
        if (systemType == OS::MacOS_x64 || systemType == OS::MacOS_arm64) {
            return juce::File("~/Library/Audio/Plug-Ins/VST3/");
        } else {
            return juce::File("C:\\Program Files\\Common Files\\VST3");
        }
    }

    static juce::File getStandalonePath(OperatingSystem::SystemType) {
        auto globalApplicationDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::globalApplicationsDirectory);
        return globalApplicationDir.getChildFile("Torsion Audio");
    }
};

#endif //GUI_APP_EXAMPLE_DEFAULTPATHS_H
