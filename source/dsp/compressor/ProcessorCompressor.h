//
//  ProcessorCompressor.hpp
//  VAESynth
//
//  Created by Fares Schulz on 17.03.23.
//

#ifndef ProcessorCompressor_h
#define ProcessorCompressor_h

#include <JuceHeader.h>
#include "Compressor.h"
#include "../../PluginParameters.h"

class ProcessorCompressor{
public:
    explicit ProcessorCompressor(juce::AudioProcessorValueTreeState &apvts);
    ~ProcessorCompressor();
    
    void prepare(const juce::dsp::ProcessSpec &spec);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void parameterChanged(const juce::String &parameterID, float newValue);

private:
    Compressor compressor;
};


#endif /* ProcessorCompressor_h */
