//
// Created by valentin.ackva on 10.03.2023.
//

#ifndef VAESYNTH_RINGBUFFER_H
#define VAESYNTH_RINGBUFFER_H

#include "JuceHeader.h"

class RingBuffer
{
public:
    RingBuffer();

    void initialise(int numChannels, int numSamples);
    void reset();
    void pushSample(float sample, int channel);
    float popSample(int channel);
    int getAvailableSamples(int channel, bool debug = false);

private:
    juce::AudioBuffer<float> buffer;
    std::vector<int> readPos, writePos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RingBuffer)
};
#endif //VAESYNTH_RINGBUFFER_H
