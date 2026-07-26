//
// Created by valentin.ackva on 16.03.2023.
//

#include "RingBuffer.h"

RingBuffer::RingBuffer() = default;

void RingBuffer::initialise(int numChannels, int numSamples) {
    readPos = std::make_unique<std::atomic<int>[]>((size_t) numChannels);
    writePos = std::make_unique<std::atomic<int>[]>((size_t) numChannels);
    numChannelsAllocated = numChannels;

    buffer.setSize(numChannels, numSamples);
    buffer.clear();
}

void RingBuffer::reset() {
    buffer.clear();
    for (int i = 0; i < numChannelsAllocated; i++) {
        readPos[(size_t) i].store(0, std::memory_order_relaxed);
        writePos[(size_t) i].store(0, std::memory_order_relaxed);
    }
}

void RingBuffer::pushSample(float sample, int channel) {
    if (std::isnan(sample)){
        sample = 0.f;
//        std::cout << "Sample is nan! push" << std::endl; //DBG
    }
    const int pos = writePos[(size_t) channel].load(std::memory_order_relaxed);
    buffer.setSample(channel, pos, sample);

    int next = pos + 1;
    if (next >= buffer.getNumSamples()) {
        next = 0;
    }
    writePos[(size_t) channel].store(next, std::memory_order_release);
}

float RingBuffer::popSample(int channel) {
    const int pos = readPos[(size_t) channel].load(std::memory_order_relaxed);
    auto sample = buffer.getSample(channel, pos);

    int next = pos + 1;
    if (next >= buffer.getNumSamples()) {
        next = 0;
    }
    readPos[(size_t) channel].store(next, std::memory_order_release);

    if (std::isnan(sample)){
//        std::cout << "Sample is nan! pop" << std::endl; //DBG
        return 0.f;
    }
    else return sample;
}

int RingBuffer::getAvailableSamples(int channel, bool debug) {
    juce::ignoreUnused(debug);
    const int read = readPos[(size_t) channel].load(std::memory_order_acquire);
    const int write = writePos[(size_t) channel].load(std::memory_order_acquire);

    if (read <= write) {
        return write - read;
    }

    return write + buffer.getNumSamples() - read;
}
