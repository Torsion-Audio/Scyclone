//
//  TransientSplitter.cpp
//  Dynamics
//
//  Created by Fares Schulz on 16.03.23.
//

#include "TransientSplitter.h"
#include "../utils/utils.h"

TransientSplitter::TransientSplitter() : detector(parameter.attackTimeDetector, (parameter.releaseTimeRatio*parameter.releaseTime)), envelope1(0.f, parameter.releaseTime), envelope2(parameter.attackTime, parameter.releaseTime){}

TransientSplitter::~TransientSplitter() = default;

void TransientSplitter::prepare(const juce::dsp::ProcessSpec &spec) {
    detector.prepare(spec);
    envelope1.prepare(spec);
    envelope2.prepare(spec);
}

void TransientSplitter::processBlock(juce::AudioBuffer<float> &buffer){
    detector.processBlock(buffer);
    envelope1.processBlock(buffer);
    envelope2.processBlock(buffer);
    for (int i = 0; i < buffer.getNumChannels(); i++){
        for (int j = 0; j < buffer.getNumSamples(); j++){
            float attack;
            float sustain;
            attack = std::abs(envelope1.getSample((unsigned long) j) - envelope2.getSample((unsigned long) j));
            attack = std::min(attack/std::abs(detector.getSample((unsigned long) j)), 1.f);
            sustain = 1.f - attack;
            float signalAttack = attack * parameter.attack * buffer.getSample(i, j);
            float signalSustain = sustain * parameter.sustain * buffer.getSample(i, j);
            buffer.setSample(i, j, signalAttack + signalSustain);
        }
    }
}

void TransientSplitter::setAttack(float newAttack){
    parameter.attack = newAttack;
}

float TransientSplitter::getAttack() const{
    return parameter.attack;
}

void TransientSplitter::setSustain(float newSustain){
    parameter.sustain = newSustain;
}

float TransientSplitter::getSustain() const{
    return parameter.sustain;
}

void TransientSplitter::setAttackTime(float newAttackTime){
    parameter.attackTime = newAttackTime;
    envelope2.setAttackTime(newAttackTime);
}

float TransientSplitter::getAttackTime() const{
    return parameter.attackTime;
}

void TransientSplitter::setAttackTimeDetector(float newAttackTimeDetector){
    parameter.attackTimeDetector = newAttackTimeDetector;
    detector.setAttackTime(newAttackTimeDetector);
}

float TransientSplitter::getAttackTimeDetector() const{
    return parameter.attackTimeDetector;
}

void TransientSplitter::setReleaseTime(float newReleaseTime){
    parameter.releaseTime = newReleaseTime;
    envelope1.setReleaseTime(newReleaseTime);
    envelope2.setReleaseTime(newReleaseTime);
}

float TransientSplitter::getReleaseTime() const{
    return parameter.releaseTime;
}

void TransientSplitter::setReleaseTimeRatio(float newReleaseTimeRatio){
    parameter.releaseTimeRatio = newReleaseTimeRatio;
    detector.setReleaseTime(parameter.releaseTime * newReleaseTimeRatio);
}

float TransientSplitter::getReleaseTimeRatio() const{
    return parameter.releaseTimeRatio;
}
