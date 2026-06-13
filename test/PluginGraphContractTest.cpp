// Plugin graph wiring contracts — grain path and parameter init

#include <gtest/gtest.h>
#include "ResamplingTestHelpers.h"
#include "dsp/mixer/DryWetMixer.h"
#include "PluginParameters.h"

using namespace resampling_test;

namespace {

juce::String loadPluginProcessorSource() {
    const juce::File src = juce::File(__FILE__).getParentDirectory().getParentDirectory()
                               .getChildFile("source/PluginProcessor.cpp");
    if (!src.existsAsFile()) {
        return {};
    }
    return src.loadFileAsString();
}

} // namespace

TEST(PluginGraph, GrainMixer_DrySamplesFitFifo) {
    juce::ScopedJuceInitialiser_GUI init;
    const double hostSR = 44100.0;
    const int hostBlock = 512;
    auto chain = prepareRoundTripChain(hostSR, hostBlock);
    runSilencePreRoll(chain, 4);
    chain.hostBuffer.clear();
    juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
    juce::AudioBuffer<float>& networkOut = chain.down.processBlock(upOut);

    juce::dsp::ProcessSpec monoSpec{ hostSR, static_cast<uint32_t>(hostBlock), 1 };
    DryWetMixer grainDryWetMixer;
    grainDryWetMixer.prepare(monoSpec);

    juce::AudioBuffer<float> grainDryBuffer;
    grainDryBuffer.makeCopyOf(networkOut);

    EXPECT_EQ(networkOut.getNumSamples(), hostBlock)
        << "down output must match host block for grain fifo (currently fails at 44.1k: 513 vs 512)";
    EXPECT_EQ(grainDryBuffer.getNumSamples(), hostBlock);
}

TEST(PluginGraph, GrainMixer_WetIsPostGrainNetworkOut) {
    const juce::String src = loadPluginProcessorSource();
    ASSERT_FALSE(src.isEmpty());
    EXPECT_TRUE(src.contains("grain1DryWetMixer.setWetSamples(networkOut1)"))
        << "network 1 wet must be post-grain networkOut1";
    EXPECT_TRUE(src.contains("grain2DryWetMixer.setWetSamples(networkOut2)"))
        << "network 2 wet must be post-grain networkOut2";
    EXPECT_FALSE(src.contains("grain1DryWetMixer.setWetSamples(network1Buffer)"))
        << "network 1 wet must not use pre-ONNX network1Buffer";
    EXPECT_FALSE(src.contains("grain2DryWetMixer.setWetSamples(network2Buffer)"))
        << "network 2 wet must not use pre-ONNX network2Buffer";
    EXPECT_TRUE(src.contains("network1Buffer.makeCopyOf(networkOut1)"))
        << "downstream network1Buffer must copy mixed post-grain output";
    EXPECT_TRUE(src.contains("network2Buffer.makeCopyOf(networkOut2)"))
        << "downstream network2Buffer must copy mixed post-grain output";
}

TEST(PluginGraph, GrainMixer_DryWetNoAliasing) {
    juce::ScopedJuceInitialiser_GUI init;
    const int hostBlock = 512;
    juce::dsp::ProcessSpec monoSpec{ 44100.0, static_cast<uint32_t>(hostBlock), 1 };
    juce::AudioBuffer<float> networkOut(1, hostBlock);
    juce::AudioBuffer<float> networkBuffer(1, hostBlock);
    networkOut.setSample(0, 0, 1.0f);
    networkBuffer.clear();

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

TEST(PluginGraph, Parameters_Network2OnOffReadsOwnId) {
    EXPECT_NE(PluginParameters::ON_OFF_NETWORK1_ID.getParamID(),
              PluginParameters::ON_OFF_NETWORK2_ID.getParamID());

    const juce::String src = loadPluginProcessorSource();
    ASSERT_FALSE(src.isEmpty()) << "Could not read PluginProcessor.cpp for regression check";

    const int line410 = src.indexOf("auto onOffNetwork2");
    ASSERT_GE(line410, 0) << "setInitialMuteParameters onOffNetwork2 line not found";

    const juce::String snippet = src.substring(line410, line410 + 120);
    EXPECT_TRUE(snippet.contains("ON_OFF_NETWORK2_ID"))
        << "onOffNetwork2 must read ON_OFF_NETWORK2_ID";
    EXPECT_FALSE(snippet.contains("ON_OFF_NETWORK1_ID"))
        << "onOffNetwork2 must not read ON_OFF_NETWORK1_ID";
}
