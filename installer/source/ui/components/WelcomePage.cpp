//
// Created by valentin.ackva on 06.07.2023.
//

#include "WelcomePage.h"

WelcomePage::WelcomePage() {
    initComponent(&headerLabel);
    initComponent(&descriptionLabel);
    initComponent(&button);
}

void WelcomePage::resized() {
    auto area = getLocalBounds();
    area.removeFromTop(20);
    headerLabel.setBounds(area.removeFromTop(50));
    area.removeFromTop(20);
    descriptionLabel.setBounds(area.removeFromTop(100));
    button.setBounds(0, 250, 500, 50);
}
