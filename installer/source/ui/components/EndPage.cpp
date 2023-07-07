//
// Created by valentin.ackva on 07.07.2023.
//

#include "EndPage.h"

EndPage::EndPage() {
    initComponent(&headerLabel);
    initComponent(&descriptionLabel);
    initComponent(&button);
}

void EndPage::resized() {
    auto area = getLocalBounds();
    area.removeFromTop(20);
    headerLabel.setBounds(area.removeFromTop(50));
    area.removeFromTop(20);
    descriptionLabel.setBounds(area.removeFromTop(100));
    button.setBounds(0, 250, 500, 50);
}

void EndPage::buttonClicked() {
    JUCEApplication::getInstance()->systemRequestedQuit();
}
