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
    juce::Label descriptionLabel {"descriptionLabel", "Please ensure all SCYCLONE instances are closed \n \n and that you have a stable Internet connection."};
    CustomButton button {"Start installation"};
};


#endif //GUI_APP_EXAMPLE_WELCOMEPAGE_H
