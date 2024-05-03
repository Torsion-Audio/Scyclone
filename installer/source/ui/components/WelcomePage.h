//
// Created by valentin.ackva on 06.07.2023.
//

#ifndef GUI_APP_EXAMPLE_WELCOMEPAGE_H
#define GUI_APP_EXAMPLE_WELCOMEPAGE_H

#include <JuceHeader.h>
#include "template/TemplateInterfacePage.h"

class WelcomePage : public TemplateInterfacePage {
public:
    WelcomePage();

private:
    void resized() override;


private:
    juce::Label headerLabel {"headerLabel", "Welcome to SCYCLONE Setup"};
    juce::Label descriptionLabel {"descriptionLabel", " Real-time Neural Timbre Transfer "};
    CustomButton button {"Start"};
};


#endif //GUI_APP_EXAMPLE_WELCOMEPAGE_H
