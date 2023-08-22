//
// Created by valentin.ackva on 06.07.2023.
//

#ifndef GUI_APP_EXAMPLE_PATHPAGE_H
#define GUI_APP_EXAMPLE_PATHPAGE_H

#include <JuceHeader.h>
#include "template/TemplateInterfacePage.h"
#include "../../utils/FormatOptions.h"
#include "../../utils/DefaultPaths.h"
#include "customComponent/CustomLabel.h"
#include "../lookAndFeel/InstallerLookAndFeel.h"


class SinglePathSelector : public juce::Component {
public:
    SinglePathSelector(FormatOptions::Options format) : pathLabel("pathLabel", DefaultPaths::getDestinationPath(format).getFullPathName())
    {

        resultPath.first = format;
        resultPath.second = DefaultPaths::getDestinationPath(format).getFullPathName();

        pathLabel.onEditorShow = [this] {
            openFileChooser("vst3", juce::File{"C:/Program Files/Common Files/VST3"});
        };
        pathLabel.setEditable(true, false);
        pathLabel.setLookAndFeel(&installerLookAndFeel);
        addAndMakeVisible(pathLabel);

        juce::String headerText = (format == FormatOptions::Options::AUPlugin) ? "AU Plugin Path"
                                   : (format == FormatOptions::Options::VST3Plugin) ? "VST3 Plugin Path"
                                                                                    : "Standalone Path";

        headerLabel.setText(headerText, juce::dontSendNotification);
        headerLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(headerLabel);
    }

    ~SinglePathSelector() override {
        pathLabel.setLookAndFeel(nullptr);
    }

    std::pair<FormatOptions::Options, juce::String> getResult() {
        return resultPath;
    }

private:

    void openFileChooser(String applicationFormat, File defaultPath) {
        fc = std::make_unique<juce::FileChooser> (juce::String{"Choose a path to save Scyclone."}+applicationFormat, defaultPath,
                                                  "*", true);

        fc->launchAsync (juce::FileBrowserComponent::openMode
                         | juce::FileBrowserComponent::canSelectDirectories,
                         [this, defaultPath] (const juce::FileChooser& chooser)
                         {
                             auto path = chooser.getResult();
                             if (!path.exists()) path = defaultPath;
                             pathLabel.setText(path.getFullPathName(), juce::dontSendNotification);
                             resultPath.second = path.getFullPathName();
                         });
    }

    void resized() override {
        auto area = getLocalBounds().reduced(20);
        headerLabel.setBounds(area.removeFromTop(20));
        area.removeFromTop(5);
        pathLabel.setBounds(area.removeFromTop(80));
    }

    std::unique_ptr<juce::FileChooser> fc;

    std::pair<float, int> test;
    std::pair<FormatOptions::Options, juce::String> resultPath;

    InstallerLookAndFeel installerLookAndFeel;
    juce::Label headerLabel;
    CustomLabel pathLabel;
};


class PathPage : public TemplateInterfacePage {
public:
    PathPage() {
        initComponent(&button);
    }

    void setFormatOptions(std::vector<FormatOptions::Options> selectedOptions) {
        options.clear();

        for (auto & selectedOption : selectedOptions) {
            options.push_back(selectedOption);

            if (selectedOption == FormatOptions::Options::AUPlugin) {
                auPathComponent = std::make_unique<SinglePathSelector>(FormatOptions::Options::AUPlugin);
                addAndMakeVisible(*auPathComponent);
            } else if (selectedOption == FormatOptions::Options::VST3Plugin) {
                vst3PathComponent = std::make_unique<SinglePathSelector>(FormatOptions::Options::VST3Plugin);
                addAndMakeVisible(*vst3PathComponent);
            } else {
                standalonePathComponent = std::make_unique<SinglePathSelector>(FormatOptions::Options::Standalone);
                addAndMakeVisible(*standalonePathComponent);
            }
        }
        selectedOptionIndex = 0;
        resized();
        setSinglePathSelectorVisible();
    }

    std::vector<std::pair<FormatOptions::Options, juce::String>> getPathResults() {
        return results;
    }

    void buttonClicked() override {
        if (selectedOptionIndex == options.size() - 1) {
            results.clear();

            if (auPathComponent != nullptr) {
                results.push_back(auPathComponent->getResult());
            }
            if (vst3PathComponent != nullptr) {
                results.push_back(vst3PathComponent->getResult());
            }
            if (standalonePathComponent != nullptr) {
                results.push_back(standalonePathComponent->getResult());
            }

            notifyFinished(this);
            return;
        } else if (selectedOptionIndex == options.size() - 2) {
            button.setButtonText("Install");
        }

        selectedOptionIndex++;
        setSinglePathSelectorVisible();
    }

private:
    void setSinglePathSelectorVisible() {
        auto currentPathSelection = options[selectedOptionIndex];

        if (auPathComponent != nullptr) {
            auPathComponent->setVisible(false);
        }
        if (vst3PathComponent != nullptr) {
            vst3PathComponent->setVisible(false);
        }
        if (standalonePathComponent != nullptr) {
            standalonePathComponent->setVisible(false);
        }

        if (currentPathSelection == FormatOptions::Options::AUPlugin) {
            auPathComponent->setVisible(true);
        } else if (currentPathSelection == FormatOptions::Options::VST3Plugin) {
            vst3PathComponent->setVisible(true);
        } else {
            standalonePathComponent->setVisible(true);
        }
    }

    void resized() override {
        auto area = getLocalBounds();
        area.removeFromTop(20);

        area.removeFromBottom(100);

        if (auPathComponent != nullptr) {
            auPathComponent->setBounds(area);
        }
        if (vst3PathComponent != nullptr) {
            vst3PathComponent->setBounds(area);
        }
        if (standalonePathComponent != nullptr) {
            standalonePathComponent->setBounds(area);
        }

        button.setBounds(0, 250, 500, 50);
    }


private:
    std::vector<FormatOptions::Options> options;
    std::vector<std::pair<FormatOptions::Options, juce::String>> results;

    std::unique_ptr<SinglePathSelector> auPathComponent;
    std::unique_ptr<SinglePathSelector> vst3PathComponent;
    std::unique_ptr<SinglePathSelector> standalonePathComponent;

    CustomButton button {"Next"};

    int selectedOptionIndex = 0;
};

#endif //GUI_APP_EXAMPLE_PATHPAGE_H
