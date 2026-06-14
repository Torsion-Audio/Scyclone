// Grain dry/wet buffer contract — dry copy must not alias wet buffer.
// Why: mirrors PluginProcessor grain path (makeCopyOf before setDrySamples).

#include <gtest/gtest.h>
#include "dsp/mixer/DryWetMixer.h"

// Signal: networkOut copied to grainDryBuffer; mutating wet must not change the dry copy.
TEST(GrainDryWet, DryCopyDoesNotAliasWetBuffer) {
    const int hostBlock = 512;
    juce::dsp::ProcessSpec monoSpec{ 44100.0, static_cast<uint32_t>(hostBlock), 1 };
    juce::AudioBuffer<float> networkOut(1, hostBlock);
    networkOut.setSample(0, 0, 1.0f);

    DryWetMixer mixer;
    mixer.prepare(monoSpec);
    mixer.setDryWetProportion(0.5f);

    juce::AudioBuffer<float> grainDryBuffer;
    grainDryBuffer.makeCopyOf(networkOut);
    const float dryBefore = grainDryBuffer.getSample(0, 0);
    mixer.setDrySamples(grainDryBuffer);
    networkOut.setSample(0, 0, 2.0f);
    mixer.setWetSamples(networkOut);

    EXPECT_EQ(grainDryBuffer.getSample(0, 0), dryBefore)
        << "dry copy must not alias wet buffer before mix";
}
