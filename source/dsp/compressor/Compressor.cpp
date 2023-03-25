//
//  Compressor.cpp
//  Compressor
//
//  Created by Fares Schulz on 21.12.22.
//

#include "Compressor.h"
#include "../utils/utils.h"

Compressor::Compressor() : envelope(parameter.attackTime, parameter.releaseTime){
}

Compressor::~Compressor() = default;

void Compressor::prepare(const juce::dsp::ProcessSpec &spec) {
    envelope.prepare(spec);
    autoMakeUpGain.BufferSize = (int) ((unsigned int) (spec.sampleRate/spec.maximumBlockSize) * spec.maximumBlockSize);
    autoMakeUpGain.inputBuffer.setSize(1, autoMakeUpGain.BufferSize);
    autoMakeUpGain.outputBuffer.setSize(1, autoMakeUpGain.BufferSize);
    autoMakeUpGain.inputBufferIndex = 0;
    autoMakeUpGain.outputBufferIndex = 0;
}

void Compressor::processBlock(juce::AudioBuffer<float> &buffer){
    envelope.processBlock(buffer);
    
    if (parameter.autoMakeUpGain){
        copyAutoMakeUpBuffer(autoMakeUpGain.inputBuffer, buffer, true);
        autoMakeUpGain.inputGain = autoMakeUpGain.inputBuffer.getRMSLevel(0, 0, autoMakeUpGain.inputBuffer.getNumSamples());
    }
    
    for (int i = 0; i < buffer.getNumChannels(); i++){
        for (int j = 0; j < buffer.getNumSamples(); j++){
            if (parameter.compType == CompressorType::Upward){
                if (2*(utils::amp2dB(envelope.getSample((unsigned long) j)) - parameter.threshold) > parameter.knee){
                    float controlVoltage;
                    controlVoltage = (1/parameter.ratio-1)*(utils::amp2dB(envelope.getSample((unsigned long) j))-parameter.threshold);
                    controlVoltage = std::max(controlVoltage, (-parameter.range));
                    if (! parameter.autoMakeUpGain) buffer.setSample(i, j, utils::dB2amp(controlVoltage + parameter.makeUpGain)*buffer.getSample(i, j));
                    else buffer.setSample(i, j, utils::dB2amp(controlVoltage)*buffer.getSample(i, j));
                }
                else if (2*std::abs(utils::amp2dB(envelope.getSample((unsigned long) j)) - parameter.threshold) <= parameter.knee){
                    float controlVoltage;
                    controlVoltage = ((1/parameter.ratio)-1) * (std::pow(utils::amp2dB(envelope.getSample((unsigned long) j)) - parameter.threshold + parameter.knee/2.f, 2.f)) / (2.f*parameter.knee);
                    controlVoltage = std::max(controlVoltage, (-parameter.range));
                    if (! parameter.autoMakeUpGain) buffer.setSample(i, j, utils::dB2amp(controlVoltage + parameter.makeUpGain)*buffer.getSample(i, j));
                    else buffer.setSample(i, j, utils::dB2amp(controlVoltage)*buffer.getSample(i, j));
                }
                else{
                    if (! parameter.autoMakeUpGain) buffer.setSample(i, j, utils::dB2amp(parameter.makeUpGain)*buffer.getSample(i, j));
                }
            }
            else if (parameter.compType == CompressorType::Expander){
                if (2*(parameter.threshold - utils::amp2dB(envelope.getSample((unsigned long) j))) > parameter.knee){
                    float controlVoltage;
                    controlVoltage = (1/parameter.ratio-1)*(parameter.threshold - utils::amp2dB(envelope.getSample((unsigned long) j)));
                    controlVoltage = std::max(controlVoltage, (-parameter.range));
                    if (! parameter.autoMakeUpGain) buffer.setSample(i, j, utils::dB2amp(controlVoltage + parameter.makeUpGain)*buffer.getSample(i, j));
                    else buffer.setSample(i, j, utils::dB2amp(controlVoltage)*buffer.getSample(i, j));
                }
                else if (2*std::abs(utils::amp2dB(envelope.getSample((unsigned long) j)) - parameter.threshold) <= parameter.knee){
                    float controlVoltage;
                    controlVoltage = ((1/parameter.ratio)-1) * (std::pow(parameter.threshold - utils::amp2dB(envelope.getSample((unsigned long) j)) + parameter.knee/2.f, 2.f)) / (2.f*parameter.knee);
                    controlVoltage = std::max(controlVoltage, (-parameter.range));
                    if (! parameter.autoMakeUpGain) buffer.setSample(i, j, utils::dB2amp(controlVoltage + parameter.makeUpGain)*buffer.getSample(i, j));
                    else buffer.setSample(i, j, utils::dB2amp(controlVoltage)*buffer.getSample(i, j));
                }
                else{
                    if (! parameter.autoMakeUpGain) buffer.setSample(i, j, utils::dB2amp(parameter.makeUpGain)*buffer.getSample(i, j));
                }
            }
        }
    }
    
    if (parameter.autoMakeUpGain){
        copyAutoMakeUpBuffer(autoMakeUpGain.outputBuffer, buffer, false);
        autoMakeUpGain.outputGain = autoMakeUpGain.outputBuffer.getRMSLevel(0, 0, autoMakeUpGain.outputBuffer.getNumSamples());

        parameter.makeUpGain = (autoMakeUpGain.inputGain > 0.f && autoMakeUpGain.inputGain > autoMakeUpGain.outputGain) ? autoMakeUpGain.inputGain / autoMakeUpGain.outputGain : 1.f;
        
        for (int i=0; i < buffer.getNumChannels(); ++i)
            buffer.applyGainRamp (i, 0, buffer.getNumSamples(), autoMakeUpGain.previousMakeUpGain, parameter.makeUpGain);

        autoMakeUpGain.previousMakeUpGain = parameter.makeUpGain;
        parameter.makeUpGain = utils::amp2dB(parameter.makeUpGain);
    }
    else autoMakeUpGain.previousMakeUpGain = utils::dB2amp(parameter.makeUpGain);
}

void Compressor::setThreshold(float newThreshold){
    parameter.threshold = newThreshold;
}

[[maybe_unused]] float Compressor::getThreshold() const{
    return parameter.threshold;
}

void Compressor::setRatio(float newRatio){
    parameter.ratio = newRatio;
}

float Compressor::getRatio() const{
    return parameter.ratio;
}

void Compressor::setKnee(float newKnee){
    parameter.knee = newKnee;
}

float Compressor::getKnee() const{
    return parameter.knee;
}

void Compressor::setMakeUpGain(float newMakeUpGain){
    parameter.makeUpGain = newMakeUpGain;
}

float Compressor::getMakeUpGain() const{
    return parameter.makeUpGain;
}

void Compressor::setRange(float newRange){
    parameter.range = newRange;
}

float Compressor::getRange() const{
    return parameter.range;
}

void Compressor::setAttackTime(float newAttackTime){
    parameter.attackTime = newAttackTime;
    envelope.setAttackTime(newAttackTime);
}

float Compressor::getAttackTime() const{
    return parameter.attackTime;
}

void Compressor::setReleaseTime(float newReleaseTime){
    parameter.releaseTime = newReleaseTime;
    envelope.setReleaseTime(newReleaseTime);
}

float Compressor::getReleaseTime() const {
    return parameter.releaseTime;
}

void Compressor::setAutoMakeUpGain(bool newBool) {
    parameter.autoMakeUpGain = newBool;
}

bool Compressor::getAutoMakeUpGain() const{
    return parameter.autoMakeUpGain;
}

void Compressor::setCompressionTypeIndex(int newCompressionTypeIndex) {
    if (newCompressionTypeIndex == 0)
        parameter.compType = CompressorType::Upward;
    else if (newCompressionTypeIndex == 1)
        parameter.compType = CompressorType::Expander;
}

int Compressor::getCompressionTypeIndex() const {
    if (parameter.compType == CompressorType::Expander)
        return 1;
    else
        return 0;
}

void Compressor::copyAutoMakeUpBuffer(juce::AudioBuffer<float>& target, juce::AudioBuffer<float>& source, bool input) {
    auto writePointer = target.getWritePointer(0);
    for (int channel = 0; channel < source.getNumChannels(); channel++){
        auto readPointer = source.getReadPointer(channel);
        if (input){
            for (int sample = 0; sample < source.getNumSamples(); sample++){
                writePointer[autoMakeUpGain.inputBufferIndex] = readPointer[sample];
                autoMakeUpGain.inputBufferIndex++;
                if (autoMakeUpGain.inputBufferIndex >= autoMakeUpGain.BufferSize) autoMakeUpGain.inputBufferIndex = 0;
            }
        } else {
            for (int sample = 0; sample < source.getNumSamples(); sample++){
                writePointer[autoMakeUpGain.outputBufferIndex] = readPointer[sample];
                autoMakeUpGain.outputBufferIndex++;
                if (autoMakeUpGain.outputBufferIndex >= autoMakeUpGain.BufferSize) autoMakeUpGain.outputBufferIndex = 0;
            }
        }
    }
}
