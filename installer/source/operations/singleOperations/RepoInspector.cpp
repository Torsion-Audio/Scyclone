//
// Created by valentin.ackva on 22.06.2023.
//

#include "RepoInspector.h"

RepoInspector::RepoInspector() {

}

void RepoInspector::inspectURL(std::string repoURL) {
    if (auto inputStream = URL (repoURL)
            .createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                        .withConnectionTimeoutMs (1000)
                                        .withNumRedirectsToFollow (0))) {
        var json = JSON::parse(*inputStream);
        inspectRepo(json);
    } else {
        notifyFinished(this, false);
    }
}

void RepoInspector::inspectRepo(var json) {
    std::regex osRegex("-osx-x64|-osx-arm64|-win-x64");
    std::regex versionRegex("v\\.\\d\\.\\d\\.\\d");

    std::map<std::string, OperatingSystem::SystemType> osMap = {
            {"-osx-x64", OperatingSystem::MacOS_x64},
            {"-osx-arm64", OperatingSystem::MacOS_arm64},
            {"-win-x64", OperatingSystem::Windows_x64}
    };

    releases.clear();

    for (int releaseIndex = 0; releaseIndex < json.size(); ++releaseIndex) {
        auto& assets = json[releaseIndex]["assets"];
        for (int assetIndex = 0; assetIndex < assets.size(); ++assetIndex) {
            auto& asset = assets[assetIndex];
            if(!asset.hasProperty("browser_download_url")) continue;

            std::string url = asset["browser_download_url"].toString().toStdString();
            std::smatch osMatch;

            if (!std::regex_search(url, osMatch, osRegex) || osMatch.size() <= 0) continue;

            auto osIterator = osMap.find(osMatch.str(0));
            if (osIterator == osMap.end()) continue;

            OperatingSystem::SystemType os = osIterator->second;

            std::smatch versionMatch;
            if (!std::regex_search(url, versionMatch, versionRegex) || versionMatch.size() <= 0) continue;

            std::string releaseVersion = versionMatch.str(0);
            releases[os].push_back({releaseVersion, url});
        }
    }
    notifyFinished(this, true);
}

std::pair<std::string, juce::URL>  RepoInspector::getLatestVersion() {
    auto os = OperatingSystem::getOperatingSystem();
    if (releases[os].empty()) {
        return { "", juce::URL("") };
    }

    std::string latestUrlString (releases[os][0].url);
    std::string latestVersion = releases[os][0].version;
    return { latestVersion, juce::URL(latestUrlString) };
}