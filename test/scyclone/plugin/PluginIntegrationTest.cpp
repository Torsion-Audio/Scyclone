// Plugin-level smoke and latency ordering properties.

#include <gtest/gtest.h>
#include "JuceFixture.h"
#include "PluginProcessor.h"
#include "TestTiming.h"

using namespace torsion::test;

class PluginIntegrationTest : public JuceAudioTest {
protected:
    void SetUp() override
    {
        JuceAudioTest::SetUp();
#if defined(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST)
        GTEST_SKIP() << "PluginIntegrationTest skipped: prebuilt ORT triggers Linux UBSan false positives (see ScycloneSanitizers.cmake)";
#endif
    }
};

TEST_F(PluginIntegrationTest, ReportedLatency_IsPositiveAndIdempotent) {
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(48000.0, 512);
    const int first = proc.getLatencySamples();
    EXPECT_GT(first, 0);
    proc.releaseResources();
    proc.prepareToPlay(48000.0, 512);
    EXPECT_EQ(proc.getLatencySamples(), first);
    proc.releaseResources();
}

#if !defined(SCYCLONE_INFERENCE_STUB)
TEST_F(PluginIntegrationTest, ReportedLatency_IsBelowLegacyAt48k) {
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(48000.0, 512);
    EXPECT_GT(proc.getLatencySamples(), 0);
    EXPECT_LT(proc.getLatencySamples(), 20480);
    proc.releaseResources();
}
#endif

TEST_F(PluginIntegrationTest, ProcessBlock_PreservesHostBlockSize) {
    AudioPluginAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;
    buffer.clear();

    for (int n = 0; n < kPreRollBlocks; ++n) {
        processor.processBlock(buffer, midi);
        EXPECT_EQ(buffer.getNumSamples(), 512);
    }

    processor.releaseResources();
}
