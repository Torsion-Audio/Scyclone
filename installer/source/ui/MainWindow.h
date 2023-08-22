//
// Created by valentin.ackva on 07.07.2023.
//

#include <JuceHeader.h>
#include "MainComponent.h"

class MainWindow    : public juce::DocumentWindow {
public:
    MainWindow (juce::String name, OperationManager& ref);

private:
    void closeButtonPressed() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};
