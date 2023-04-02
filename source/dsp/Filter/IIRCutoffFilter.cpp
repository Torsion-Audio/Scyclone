//
// Created by schee on 14/03/2023.
//

#include "IIRCutoffFilter.h"

IIRCutoffFilter::IIRCutoffFilter(const juce::AudioProcessorValueTreeState &apvts, int no) : index(no),   lowPassFilter(juce::dsp::IIR::Coefficients<float>::makeLowPass(48000, 20000.0f, 1.0)),
                                                   highPassFilter(juce::dsp::IIR::Coefficients<float>::makeHighPass(48000, 20.0f, 1.0))
{
    targetFreqRangeHPF = {20.0, 8000.0};
    targetFreqRangeLPF = {100.0, 20000.0};
    targetFreqRangeHPF.setSkewForCentre(500.0);
    targetFreqRangeLPF.setSkewForCentre(500.0);
    if (index == 1) updateFilterParams(apvts.getRawParameterValue(PluginParameters::FILTER_NETWORK1_ID)->load());
    if (index == 2) updateFilterParams(apvts.getRawParameterValue(PluginParameters::FILTER_NETWORK2_ID)->load());
}

IIRCutoffFilter::~IIRCutoffFilter()
{
}

void IIRCutoffFilter::updateLPFilterParams(double freq, double q)
{
    filterParamsLP.freq = freq;
    filterParamsLP.q = q;
    filterParamsLP.enabled = (freq < 20000);
    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSpec.sampleRate,
                                                                          (float)freq,
                                                                          (float)q);
}

void IIRCutoffFilter::updateHPFilterParams(double freq, double q)
{
    filterParamsHP.freq = freq;
    filterParamsHP.q = q;
    filterParamsHP.enabled = (freq > 20);
    *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSpec.sampleRate,
                                                                            (float)freq,
                                                                            (float)q);
}

void IIRCutoffFilter::prepare(const juce::dsp::ProcessSpec &spec)
{
    currentSpec = spec;
    
    lowPassFilter.reset();
    lowPassFilter.prepare(spec);

    highPassFilter.reset();
    highPassFilter.prepare(spec);
}

void IIRCutoffFilter::updateFilterParams(const float yPos)
{
    juce::NormalisableRange<float> sourceRange;
    if (yPos >= 0.5f) // highpass
    {
        const auto clampedYPos = std::clamp(yPos, 0.5f, 1.f);
        sourceRange =  {0.5, 1.0};
        filterParamsHP.enabled = true;
        filterParamsLP.enabled = false;
        const auto normalizedValue = sourceRange.convertTo0to1(clampedYPos);
//        std::cout << "normalized Value HPF: " << normalizedValue << std::endl; //DBG
        const auto targetFreq =  targetFreqRangeHPF.convertFrom0to1(normalizedValue);
        updateHPFilterParams(targetFreq, filterParamsHP.q);
    }
    else if (yPos < 0.5f) // lowpass
    {
        const auto clampedYPos = std::clamp(yPos, 0.0f, 0.5f);
        sourceRange = {0.0, 0.5};
        filterParamsLP.enabled = true;
        filterParamsHP.enabled = false;
        const auto normalizedValue = sourceRange.convertTo0to1(clampedYPos);
//        std::cout << "normalized Value LPF: " << normalizedValue << std::endl; //DBG
        const auto targetFreq =  targetFreqRangeLPF.convertFrom0to1(normalizedValue);
        updateLPFilterParams(targetFreq, filterParamsLP.q);
    }
}

void IIRCutoffFilter::processHPFilter(juce::AudioBuffer<float>& buffer) {

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    if (filterParamsHP.enabled)
        highPassFilter.process(context);
    else
        return;
}

void IIRCutoffFilter::processLPFilter(juce::AudioBuffer<float>& buffer) {
     juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    if (filterParamsLP.enabled)
        lowPassFilter.process(context);
    else return;
}

void IIRCutoffFilter::processFilters(juce::AudioBuffer<float> &buffer) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                float sampleToCheck = buffer.getSample(channel, sample);
                if (isnan(sampleToCheck)) {
                    return;
                }
            }
        }
    processHPFilter(buffer);
    processLPFilter(buffer);
}

void IIRCutoffFilter::parameterChanged(const juce::String &parameterID, float newValue) {
    if (parameterID == PluginParameters::FILTER_NETWORK1_ID && index == 1) {
        updateFilterParams(newValue);
    } else if (parameterID == PluginParameters::FILTER_NETWORK2_ID && index == 2) {
        updateFilterParams(newValue);
    } else if (parameterID == PluginParameters::ON_OFF_NETWORK1_ID && index == 1) {
        setMuted((bool)!newValue);
    } else if (parameterID == PluginParameters::ON_OFF_NETWORK2_ID && index == 2) {
        setMuted((bool)!newValue);
    }
}

void IIRCutoffFilter::setMuted(bool shouldBeMuted) {
    isMuted = shouldBeMuted;
}
