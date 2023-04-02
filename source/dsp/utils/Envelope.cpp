//
//  Envelope.cpp
//  Compressor
//
//  Created by Fares Schulz on 21.12.22.
//

#include "Envelope.h"

Envelope::Envelope(float initAttackTime, float initReleaseTime) {
    attackTime = initAttackTime;
    releaseTime = initReleaseTime;
}

Envelope::~Envelope() = default;

void Envelope::prepare(const juce::dsp::ProcessSpec &spec){
    setSampleRate((float) spec.sampleRate);
    setAttackTime(getAttackTime());
    setReleaseTime(getReleaseTime());
    for (int i = 0; i < (int) spec.maximumBlockSize; i++){
        envelope.push_back(0.);
        detector.push_back(0.);
    }
}

void Envelope::processBlock(juce::AudioBuffer<float>& buffer){
    bool mono = (buffer.getNumChannels() == 1);
    if (attackTime == 0.f){
        for (unsigned long i = 0; i < (unsigned long) buffer.getNumSamples(); i++){
            detector[i] = (mono) ? buffer.getSample(0, (int) i)
                           : std::max(std::abs(buffer.getSample(0, (int) i)), std::abs(buffer.getSample(1, (int) i)));
            if (lastValue < detector[i]) envelope[i] = detector[i];
            else envelope[i] = releaseCoefficient*lastValue + (1-releaseCoefficient)*detector[i];
            lastValue = envelope[i];
        }
    }
    else {
        for (unsigned long i = 0; i < (unsigned long) buffer.getNumSamples(); i++){
            detector[i] = (mono) ? buffer.getSample(0, (int) i)
                            : std::max(std::abs(buffer.getSample(0, (int) i)), std::abs(buffer.getSample(1, (int) i)));
            if (isnan(lastValue)) lastValue = 0.f;
            if (lastValue < detector[i]) envelope[i] = attackCoefficient*lastValue + (1-attackCoefficient)*detector[i];
            else envelope[i] = releaseCoefficient*lastValue + (1-releaseCoefficient)*detector[i];
            lastValue = envelope[i];
        }
    }
}

void Envelope::setAttackTime(float newAttackTime){
    attackTime = newAttackTime;
    attackCoefficient = (float) std::exp(-1/(newAttackTime*getSampleRate()));
}

float Envelope::getAttackTime() const{
    return attackTime;
}

void Envelope::setReleaseTime(float newReleaseTime){
    releaseTime = newReleaseTime;
    releaseCoefficient = (float) std::exp(-1/(newReleaseTime*getSampleRate()));
}

float Envelope::getReleaseTime() const{
    return releaseTime;
}

float Envelope::getSample(unsigned long sample){
    return envelope[sample];
}

void Envelope::setSampleRate(float newSampleRate) {
    sampleRate = newSampleRate;
}

float Envelope::getSampleRate() const {
    return sampleRate;
}
