//
// Created by schee on 22/03/2023.
//

#include "GrainDelay.h"
#include "../../PluginParameters.h"

GrainDelay::GrainDelay(const int no) : number(no) {
    rnboObject.setParameterValue(0, 2.f);
}

GrainDelay::~GrainDelay() {

}

void GrainDelay::prepare(const juce::dsp::ProcessSpec &spec) {
    sampleRate = (int) spec.sampleRate;
    auto samplesPerBlock = spec.maximumBlockSize;
    rnboObject.prepareToProcess(sampleRate, static_cast<size_t> (samplesPerBlock));
}

void GrainDelay::processBlock(juce::AudioBuffer<float> &buffer) {

    if (!isMuted) {
        auto bufferSize = buffer.getNumSamples();
        rnboObject.prepareToProcess(sampleRate,
                                    static_cast<size_t> (bufferSize));

        rnboObject.process(buffer.getArrayOfWritePointers(),
                           static_cast<RNBO::Index> (buffer.getNumChannels()),
                           buffer.getArrayOfWritePointers(),
                           static_cast<RNBO::Index> (buffer.getNumChannels()),
                           static_cast<RNBO::Index> (bufferSize));
    }
}

void GrainDelay::setParameterValue(std::atomic<float>* parameterToConnect, int rnboParameterIdx){
    rnboObject.setParameterValue(rnboParameterIdx, parameterToConnect->load());

}

void GrainDelay::setMuted(bool newState) {
    isMuted = newState;
}

void GrainDelay::parameterChanged(const juce::String &parameterID, float newValue) {
    if (parameterID == PluginParameters::GRAIN_ON_OFF_NETWORK1_ID && number == 1)
        setMuted(!(bool) newValue);
    else if (parameterID == PluginParameters::GRAIN_ON_OFF_NETWORK2_ID && number == 2)
        setMuted(!(bool) newValue);
    else if ((parameterID == PluginParameters::GRAIN_NETWORK1_PITCH_ID && number == 1) || (parameterID == PluginParameters::GRAIN_NETWORK2_PITCH_ID && number == 2))
        rnboObject.setParameterValue(2, newValue);
    else if ((parameterID == PluginParameters::GRAIN_NETWORK1_SIZE_ID && number == 1 ) || (parameterID == PluginParameters::GRAIN_NETWORK2_SIZE_ID && number == 2))
        rnboObject.setParameterValue(1, newValue);
    else if ((parameterID == PluginParameters::GRAIN_NETWORK1_INTERVAL_ID && number == 1 ) || (parameterID == PluginParameters::GRAIN_NETWORK2_INTERVAL_ID && number == 2))
        rnboObject.setParameterValue(3, newValue);}
