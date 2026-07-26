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

// Why: anira derives ONNX latency from the HostConfig, so a larger host block absorbs more of
// the block-alignment delay. Ordering property, not a magic number — the measured 48 kHz curve
// runs 6112 samples at block 32 down to 4096 at block 2048 (see OnnxInferenceLatency.h).
// Stub-excluded: SanitizerInferenceBackend reports a fixed latency by design.
#if !defined(SCYCLONE_INFERENCE_STUB)
TEST_F(PluginIntegrationTest, ReportedLatency_IsMonotonicDecreasingWithHostBlockAt48k) {
    AudioPluginAudioProcessor proc;

    proc.prepareToPlay(48000.0, 32);
    const int latency32 = proc.getLatencySamples();
    proc.releaseResources();

    proc.prepareToPlay(48000.0, 512);
    const int latency512 = proc.getLatencySamples();
    proc.releaseResources();

    proc.prepareToPlay(48000.0, 2048);
    const int latency2048 = proc.getLatencySamples();
    proc.releaseResources();

    EXPECT_GT(latency32, latency512) << "larger host blocks should reduce block-aligned ONNX delay";
    EXPECT_GT(latency512, latency2048) << "larger host blocks should reduce block-aligned ONNX delay";
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
