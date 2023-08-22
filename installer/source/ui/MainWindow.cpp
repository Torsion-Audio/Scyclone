//
// Created by valentin.ackva on 07.07.2023.
//

#include "MainWindow.h"

MainWindow::MainWindow(juce::String name, OperationManager &ref)
        : DocumentWindow (name,
                          juce::Colours::black,
                          DocumentWindow::closeButton)
{
    setUsingNativeTitleBar (true);
    setContentOwned (new MainComponent(ref), true);

    setResizable (false, false);
    centreWithSize (getWidth(), getHeight());

    setVisible (true);
}

void MainWindow::closeButtonPressed() {
    JUCEApplication::getInstance()->systemRequestedQuit();
}
