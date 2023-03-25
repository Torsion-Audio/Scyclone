//
// Created by valentin.ackva on 22.02.2023.
//

#ifndef VAESYNTH_TRANSIENTGRID_H
#define VAESYNTH_TRANSIENTGRID_H

#include "JuceHeader.h"
#include "../../../utils/colors.h"

class TransientGrid : public juce::Component {
public:
    TransientGrid();

    void paint(juce::Graphics& g) override;
};

#endif //VAESYNTH_TRANSIENTGRID_H
