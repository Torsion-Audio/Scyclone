//
// Created by valentin.ackva on 12.06.2023.
//

#ifndef GUI_APP_EXAMPLE_CUSTOMLABEL_H
#define GUI_APP_EXAMPLE_CUSTOMLABEL_H

#include "JuceHeader.h"

class CustomLabel : public juce::Label {
public:
    CustomLabel(const juce::String & componentText, const juce::String & labelText);

private:
    virtual TextEditor* createEditorComponent() override;

    void resized() override;

};



#endif //GUI_APP_EXAMPLE_CUSTOMLABEL_H
