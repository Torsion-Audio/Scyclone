//
// Created by valentin.ackva on 07.07.2023.
//

#ifndef GUI_APP_EXAMPLE_MAIN_H
#define GUI_APP_EXAMPLE_MAIN_H

#include "operations/OperationManager.h"
#include "ui/MainWindow.h"

//==============================================================================
class GuiAppApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    GuiAppApplication();

    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;

    //==============================================================================
    void initialise (const juce::String& commandLine) override;
    void shutdown() override;

    //==============================================================================
    void systemRequestedQuit() override;
    void anotherInstanceStarted (const juce::String& commandLine) override;


private:
    OperationManager installManager;
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (GuiAppApplication)


#endif //GUI_APP_EXAMPLE_MAIN_H
