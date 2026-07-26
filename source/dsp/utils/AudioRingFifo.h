#ifndef SCYCLONE_AUDIORINGFIFO_H
#define SCYCLONE_AUDIORINGFIFO_H

#include "JuceHeader.h"

class AudioRingFifo
{
public:
    void prepare(int numChannels, int capacityInSamples)
    {
        buffer.setSize(numChannels, capacityInSamples);
        buffer.clear();
        readIndex = 0;
        writeIndex = 0;
        numAvailable = 0;
    }

    int available() const noexcept { return numAvailable; }

    void pushZeros(int numSamples)
    {
        jassert(numAvailable + numSamples <= buffer.getNumSamples());

        for (int done = 0; done < numSamples;) {
            const int chunk = juce::jmin(numSamples - done, buffer.getNumSamples() - writeIndex);
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.clear(ch, writeIndex, chunk);
            writeIndex = (writeIndex + chunk) % buffer.getNumSamples();
            done += chunk;
        }

        numAvailable += numSamples;
    }

    void push(const juce::AudioBuffer<float>& source, int sourceStartSample, int numSamples)
    {
        jassert(numAvailable + numSamples <= buffer.getNumSamples());

        for (int done = 0; done < numSamples;) {
            const int chunk = juce::jmin(numSamples - done, buffer.getNumSamples() - writeIndex);
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.copyFrom(ch, writeIndex,
                                source, juce::jmin(ch, source.getNumChannels() - 1),
                                sourceStartSample + done, chunk);
            writeIndex = (writeIndex + chunk) % buffer.getNumSamples();
            done += chunk;
        }

        numAvailable += numSamples;
    }

    void pop(juce::AudioBuffer<float>& destination, int destinationStartSample, int numSamples)
    {
        jassert(numAvailable >= numSamples);

        for (int done = 0; done < numSamples;) {
            const int chunk = juce::jmin(numSamples - done, buffer.getNumSamples() - readIndex);
            for (int ch = 0; ch < destination.getNumChannels(); ++ch)
                destination.copyFrom(ch, destinationStartSample + done,
                                     buffer, juce::jmin(ch, buffer.getNumChannels() - 1),
                                     readIndex, chunk);
            readIndex = (readIndex + chunk) % buffer.getNumSamples();
            done += chunk;
        }

        numAvailable -= numSamples;
    }

private:
    juce::AudioBuffer<float> buffer;
    int readIndex = 0;
    int writeIndex = 0;
    int numAvailable = 0;
};

#endif // SCYCLONE_AUDIORINGFIFO_H
