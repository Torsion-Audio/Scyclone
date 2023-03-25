//
// Created by valentin.ackva on 12.02.2023.
//

#ifndef VAESYNTH_CUSTOMLABEL_H
#define VAESYNTH_CUSTOMLABEL_H

#include "JuceHeader.h"
#include "../../../../LookAndFeel/CustomFontLookAndFeel.h"

class CustomLabel : public juce::Label {
private:
    juce::TextEditor* createEditorComponent() override;
};



#endif //VAESYNTH_CUSTOMLABEL_H
