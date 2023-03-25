//
//  Envelop.h
//  Compressor
//
//  Created by Fares Schulz on 21.12.22.
//

#ifndef envelope_h
#define envelope_h

#include <JuceHeader.h>

class Envelope{
public:
    Envelope(float initAttackTime, float initReleaseTime);
    ~Envelope();
    void prepare(const juce::dsp::ProcessSpec &spec);
    void setAttackTime(float attackTime);
    float getAttackTime() const;
    void setReleaseTime(float releaseTime);
    float getReleaseTime() const;
    void processBlock(juce::AudioBuffer<float>& buffer);
    float getSample(unsigned long sample);

private:
    void setSampleRate(float newSampleRate);
    float getSampleRate() const;

private:
    std::vector<float> envelope;
    float lastValue = 0.f;
    std::vector<float> detector;
    float attackTime;
    float releaseTime;
    float attackCoefficient = 1.f;
    float releaseCoefficient = 1.f;
    float sampleRate = 48000.f;
};

#endif
