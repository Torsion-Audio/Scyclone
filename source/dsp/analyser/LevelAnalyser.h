//
// Created by valentin.ackva on 16.03.2023.
//

#ifndef VAESYNTH_LEVELANALYSER_H
#define VAESYNTH_LEVELANALYSER_H

#include "JuceHeader.h"

enum LevelType{
    RMS,
    PEAK
};

class LevelAnalyser {
public:
    void processBlock(juce::AudioBuffer<float>& buffer);
    void setLevelType(LevelType newLevelType);
    float getCurrentLevel();

private:
    std::atomic<float> currentRMS;
    std::atomic<float> currentPeak;

    LevelType levelType = PEAK;
};


#endif //VAESYNTH_LEVELANALYSER_H
