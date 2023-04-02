//
// Created by schee on 14/03/2023.
//

#ifndef VAESYNTH_IIRCUTOFFFILTER_H
#define VAESYNTH_IIRCUTOFFFILTER_H

#include <JuceHeader.h>
#include "../../PluginParameters.h"

class IIRCutoffFilter
{
public:
    IIRCutoffFilter(const juce::AudioProcessorValueTreeState &apvts, int no);
    ~IIRCutoffFilter();
    void updateLPFilterParams(double freq, double q);
    void updateHPFilterParams(double freq, double q);

    void prepare(const juce::dsp::ProcessSpec &spec);

    void processLPFilter(juce::AudioBuffer<float>& buffer);
    void processHPFilter(juce::AudioBuffer<float>& buffer);
    void processFilters(juce::AudioBuffer<float>& buffer);

    void parameterChanged(const juce::String &parameterID, float newValue);
    void updateFilterParams(const float yPos);

    void setMuted(bool shouldBeMuted);

private:
    int index;
    bool isMuted = false;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowPassFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highPassFilter;
    juce::NormalisableRange<float> targetFreqRangeHPF;
    juce::NormalisableRange<float> targetFreqRangeLPF;


    struct FILTER_PARAMS{
        double freq;
        double q = 0.5;
        bool enabled = false;
    };

    juce::dsp::ProcessSpec currentSpec = {48000, 512, 1};
    FILTER_PARAMS filterParamsHP, filterParamsLP;
};


#endif //VAESYNTH_IIRCUTOFFFILTER_H
