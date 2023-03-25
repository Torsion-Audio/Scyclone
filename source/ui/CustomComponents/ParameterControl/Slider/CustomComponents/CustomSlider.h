//
// Created by valentin.ackva on 12.02.2023.
//

#ifndef VAESYNTH_CUSTOMSLIDER_H
#define VAESYNTH_CUSTOMSLIDER_H

#include "JuceHeader.h"

class CustomSlider : public juce::Slider {
public:
    std::function<void()> openManualValueBox;

private:
    void mouseDoubleClick (const juce::MouseEvent&) override;
};


#endif //VAESYNTH_CUSTOMSLIDER_H
