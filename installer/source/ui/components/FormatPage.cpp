//
// Created by valentin.ackva on 06.07.2023.
//

#include "FormatPage.h"

FormatPage::FormatPage() {
    initComponent(&headerLabel);
    initComponent(&button);
    for (auto & formatOption : formatOptions) {
        initComponent(formatOption);
        formatOption->onClick = [this] { formatSelectionChanged();};
    }
}

std::vector<FormatOptions::Options> FormatPage::getSelectedOptions() {
    return selectedOptions;
}

void FormatPage::formatSelectionChanged() {
    selectedOptions.clear();
    for (auto & formatOption : formatOptions) {
        if (formatOption->getToggleState()) {
            if(formatOption == &vst3Format) {
                selectedOptions.push_back(FormatOptions::Options::VST3Plugin);
            } else if(formatOption == &standaloneFormat) {
                selectedOptions.push_back(FormatOptions::Options::Standalone);
            } else {
                selectedOptions.push_back(FormatOptions::Options::AUPlugin);
            }
        }
    }

    button.setEnabled(selectedOptions.size() > 0);
}

void FormatPage::resized() {
    auto area = getLocalBounds();
    area.removeFromTop(20);
    headerLabel.setBounds(area.removeFromTop(50));

    auto areaToggleButtons = area;
    areaToggleButtons.removeFromLeft(getWidth() * 35 / 100);

    for (auto & formatOption : formatOptions) {
        formatOption->setBounds(areaToggleButtons.removeFromTop(30));
    }

    button.setBounds(0, 250, 500, 50);
}
