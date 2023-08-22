//
// Created by valentin.ackva on 07.07.2023.
//

#ifndef GUI_APP_EXAMPLE_ENDPAGE_H
#define GUI_APP_EXAMPLE_ENDPAGE_H

#include <JuceHeader.h>
#include "template/TemplateInterfacePage.h"

class EndPage : public TemplateInterfacePage {
public:
    EndPage();

private:
    void resized() override;
    void buttonClicked() override;

private:
    juce::Label headerLabel {"headerLabel", ""};
    juce::Label descriptionLabel {"descriptionLabel", "Installation successful!"};
    CustomButton button {"close"};
};


#endif //GUI_APP_EXAMPLE_ENDPAGE_H
