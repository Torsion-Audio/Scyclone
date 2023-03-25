//
//  ProcessorTransientSplitter.h
//  VAESynth
//
//  Created by Fares Schulz on 17.03.23.
//

#ifndef ProcessorTransientSplitter_h
#define ProcessorTransientSplitter_h

#include <JuceHeader.h>
#include "TransientSplitter.h"
#include "../../PluginParameters.h"

class ProcessorTransientSplitter{
public:
    ProcessorTransientSplitter(const juce::AudioProcessorValueTreeState &apvts, int no);
    ~ProcessorTransientSplitter();
    
    void prepare(const juce::dsp::ProcessSpec &spec);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void parameterChanged(const juce::String &parameterID, float newValue);
    void setMuted (bool shouldBeMuted);

private:
    int index;
    TransientSplitter transientSplitter;
    bool isMuted = false;

};

#endif /* ProcessorTransientSplitter_h */
