//
//  Compressor.h
//  Compressor
//
//  Created by Fares Schulz on 21.12.22.
//
#ifndef compressor_h
#define compressor_h

#include <JuceHeader.h>
#include "../utils/Envelope.h"

enum CompressorType {
    Upward,
    Expander
};

struct CompressorParameter{
    float threshold;
    float ratio;
    float knee;
    float makeUpGain;
    float range;
    float attackTime;
    float releaseTime;
    bool autoMakeUpGain;
    CompressorType compType;
};

struct AutoMakeUpGain{
    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> outputBuffer;
    int inputBufferIndex;
    int outputBufferIndex;
    int BufferSize;
    float inputGain;
    float outputGain;
    float previousMakeUpGain;
};


class Compressor{
public:
    Compressor();
    ~Compressor();

    void prepare(const juce::dsp::ProcessSpec &spec);
    void processBlock(juce::AudioBuffer<float>& buffer);

public:
    void setThreshold(float newThreshold);
    float getThreshold() const;

    void setRatio(float newRatio);

    float getRatio() const;

    [[maybe_unused]] void setKnee(float newKnee);
    float getKnee() const;

    void setMakeUpGain(float newMakeUpGain);
    float getMakeUpGain() const;

    void setRange(float newRange);
    float getRange() const;

    void setAttackTime(float newAttackTime);
    float getAttackTime() const;

    void setReleaseTime(float newReleaseTime);
    float getReleaseTime() const;
    
    void setAutoMakeUpGain(bool newBool);
    bool getAutoMakeUpGain() const;

    void setCompressionTypeIndex(int newCompressionTypeIndex);

    int getCompressionTypeIndex() const;
    
private:
    CompressorParameter parameter {0.f, 4.0f, 4.0f, 0.0f, 80.0f, 0.05f, 0.3f, true, Upward};
    AutoMakeUpGain autoMakeUpGain;
    Envelope envelope;
    
    void copyAutoMakeUpBuffer(juce::AudioBuffer<float>& target, juce::AudioBuffer<float>& source, bool input);
};

#endif
