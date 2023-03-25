//
//  ProcessorTransientSplitter.cpp
//  VAESynth
//
//  Created by Fares Schulz on 17.03.23.
//

#include "ProcessorTransientSplitter.h"

ProcessorTransientSplitter::ProcessorTransientSplitter(const juce::AudioProcessorValueTreeState &apvts, int no): index(no), transientSplitter(){
    if (!isMuted){
        if (index == 1) {
            transientSplitter.setAttackTime(apvts.getRawParameterValue(PluginParameters::TRAN_ATTACK_TIME_NETWORK1_ID)->load());
            float tran1_value = apvts.getRawParameterValue(PluginParameters::TRAN_SHAPER_NETWORK1_ID)->load();
            if (tran1_value < 0.5f) {
                transientSplitter.setAttack(1.f);
                transientSplitter.setSustain(tran1_value*2.f);
            }
            else {
                transientSplitter.setAttack(1.f - ((tran1_value-0.5f)*2.f));
                transientSplitter.setAttack(1.f);
            }
        } else if (index == 2) {
            transientSplitter.setAttackTime(apvts.getRawParameterValue(PluginParameters::TRAN_ATTACK_TIME_NETWORK2_ID)->load());
            float tran2_value = apvts.getRawParameterValue(PluginParameters::TRAN_SHAPER_NETWORK2_ID)->load();
            if (tran2_value < 0.5f) {
                transientSplitter.setAttack(1.f);
                transientSplitter.setSustain(tran2_value*2.f);
            }
            else {
                transientSplitter.setAttack(1.f - ((tran2_value-0.5f)*2.f));
                transientSplitter.setAttack(1.f);
            }
        }
    }
}

ProcessorTransientSplitter::~ProcessorTransientSplitter() = default;

void ProcessorTransientSplitter::prepare(const juce::dsp::ProcessSpec &spec){
    transientSplitter.prepare(spec);
}

void ProcessorTransientSplitter::processBlock(juce::AudioBuffer<float>& buffer){
    transientSplitter.processBlock(buffer);
}

void ProcessorTransientSplitter::parameterChanged(const juce::String &parameterID, float newValue){
    if (parameterID == PluginParameters::TRAN_ATTACK_TIME_NETWORK1_ID && index == 1) {
        transientSplitter.setAttackTime(newValue);
    } else if (parameterID == PluginParameters::TRAN_SHAPER_NETWORK1_ID && index == 1) {
//            std::cout << "New Value: " << newValue << ", Index: " << index << std::endl; // DBG
        if (newValue < 0.5f) {
            transientSplitter.setAttack(1.f);
            transientSplitter.setSustain(newValue*2.f);
        }
        else {
            transientSplitter.setAttack(1.f - ((newValue-0.5f)*2.f));
            transientSplitter.setSustain(1.f);
        }
    } else if (parameterID == PluginParameters::TRAN_ATTACK_TIME_NETWORK2_ID && index == 2) {
            transientSplitter.setAttackTime(newValue);
    } else if (parameterID == PluginParameters::TRAN_SHAPER_NETWORK2_ID && index == 2) {
//            std::cout << "New Value: " << newValue << ", Index: " << index << std::endl; // DBG
        if (newValue < 0.5f) {
            transientSplitter.setAttack(1.f);
            transientSplitter.setSustain(newValue*2.f);
        }
        else {
            transientSplitter.setAttack(1.f - ((newValue-0.5f)*2.f));
            transientSplitter.setSustain(1.f);
        }
    } else if (parameterID == PluginParameters::ON_OFF_NETWORK1_ID && index == 1){
        setMuted(!newValue);
    } else if (parameterID == PluginParameters::ON_OFF_NETWORK2_ID && index == 2){
        setMuted(!newValue);
    }
}

void ProcessorTransientSplitter::setMuted(bool shouldBeMuted) {
    isMuted = shouldBeMuted;
}