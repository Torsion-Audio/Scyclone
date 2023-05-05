//
// Created by valentin.ackva on 16.03.2023.
//

#include "LevelAnalyser.h"

void LevelAnalyser::processBlock(juce::AudioBuffer<float> &buffer) {
    switch (levelType)
    {
        case PEAK:
            currentPeak.store(buffer.getMagnitude(0, buffer.getNumSamples()));
            break;
        case RMS:
            float newRMS = 0;
            for (int i = 0; i < buffer.getNumChannels(); ++i) {
                newRMS += buffer.getRMSLevel(i, 0, buffer.getNumSamples()) / ((float) i + 1);
            }
            currentRMS.store(newRMS);
            break;
    }
}

void LevelAnalyser::setLevelType(LevelType newLevelType) {
    levelType = newLevelType;
}

float LevelAnalyser::getCurrentLevel() {
    if (levelType == PEAK) {
        return currentPeak.load();
    } else {
        return currentRMS.load();
    }
}
