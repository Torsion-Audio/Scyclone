//
//  TransientSplitter.h
//  Dynamics
//
//  Created by Fares Schulz on 16.03.23.
//

#ifndef TransientSplitter_h
#define TransientSplitter_h

#include <JuceHeader.h>
#include "../utils/Envelope.h"

struct TransientSplitterParameter{
    float attack;
    float sustain;
    float attackTime;
    float attackTimeDetector;
    float releaseTime;
    float releaseTimeRatio;
};

class TransientSplitter{
public:
    TransientSplitter();
    ~TransientSplitter();

    void prepare(const juce::dsp::ProcessSpec &spec);
    void processBlock(juce::AudioBuffer<float>& buffer);

public:
    void setAttack(float newAttack);
    float getAttack() const;

    void setSustain(float newSustain);
    float getSustain() const;
    
    void setAttackTime(float newAttackTime);
    float getAttackTime() const;
    
    void setAttackTimeDetector(float newAttackTimeDetector);
    float getAttackTimeDetector() const;
    
    void setReleaseTime(float newReleaseTime);
    float getReleaseTime() const;
    
    void setReleaseTimeRatio(float newReleaseTimeRatio);
    float getReleaseTimeRatio() const;

private:
    TransientSplitterParameter parameter {1.f, 1.f, .5f, 0.f, .3f, 10.f};

    Envelope detector;
    Envelope envelope1;
    Envelope envelope2;
    
};

#endif /* TransientSplitter_h */
