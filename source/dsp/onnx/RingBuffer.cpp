//
// Created by valentin.ackva on 16.03.2023.
//

#include "RingBuffer.h"

RingBuffer::RingBuffer() = default;

void RingBuffer::initialise(int numChannels, int numSamples) {
    readPos.resize(numChannels);
    writePos.resize(numChannels);

    for (int i = 0; i < readPos.size(); i++) {
        readPos[i] = 0;
        writePos[i] = 0;
    }

    buffer.setSize(numChannels, numSamples);
}

void RingBuffer::reset() {
    buffer.clear();
    for (int i = 0; i < readPos.size(); i++) {
        readPos[i] = 0;
        writePos[i] = 0;
    }
}

void RingBuffer::pushSample(float sample, int channel) {
    buffer.setSample(channel, writePos[channel], sample);

    ++writePos[channel];

    if (writePos[channel] >= buffer.getNumSamples()) {
        writePos[channel] = 0;
    }
}

float RingBuffer::popSample(int channel) {
    auto sample = buffer.getSample(channel, readPos[channel]);

    ++readPos[channel];

    if (readPos[channel] >= buffer.getNumSamples()) {
        readPos[channel] = 0;
    }
    return sample;
}

int RingBuffer::getAvailableSamples(int channel, bool debug) {
    if (debug) {
        std::cout << "getAvailableSamples["<< readPos[channel] << ", " << writePos[channel] << "]: ";
    }
    int returnValue;

    if (readPos[channel] <= writePos[channel]) {
        returnValue = writePos[channel] - readPos[channel];
    } else {
        returnValue = writePos[channel] + buffer.getNumSamples() - readPos[channel];
    }

    if (debug) {
        std::cout << returnValue << std::endl;
    }


    return returnValue;
}
