//
// Created by valentin.ackva on 22.06.2023.
//

#ifndef GUI_APP_EXAMPLE_REPOINSPECTOR_H
#define GUI_APP_EXAMPLE_REPOINSPECTOR_H

#include <regex>
#include "JuceHeader.h"
#include "../../utils/OperatingSystem.h"
#include "UtilClass.h"

class RepoInspector : public UtilClass {
public:
    RepoInspector();
    void inspectURL(std::string repoURL);
    std::pair<std::string, juce::URL> getLatestVersion();

private:
    void inspectRepo(var json);

    struct ReleaseData {
        std::string version;
        std::string url;
    };

    std::map<OperatingSystem::SystemType, std::vector<ReleaseData>> releases;
};


#endif //GUI_APP_EXAMPLE_REPOINSPECTOR_H
