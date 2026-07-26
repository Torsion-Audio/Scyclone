// Plugin-level smoke and latency ordering properties.
// Why: full graph uses real ONNX — we test block contract and monotonicity, not impulse fidelity.

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
#if defined(SCYCLONE_ONNX_STUB)
        GTEST_SKIP() << "PluginIntegrationTest requires ONNX Runtime (disabled under sanitizer stub build)";
#elif defined(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST)
        GTEST_SKIP() << "PluginIntegrationTest skipped: prebuilt ORT triggers Linux UBSan false positives (see ScycloneSanitizers.cmake)";
#endif
    }
};

// Signal: prepareToPlay at 44.1k/512; latency positive and idempotent across release/re-prepare.
TEST_F(PluginIntegrationTest, ReportedLatency_IsPositiveAndIdempotent) {
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(44100.0, 512);
    const int first = proc.getLatencySamples();
    EXPECT_GT(first, 0);
    proc.releaseResources();
    proc.prepareToPlay(44100.0, 512);
    EXPECT_EQ(proc.getLatencySamples(), first);
    proc.releaseResources();
}

// Why: ONNX block-aligned delay shrinks as host block grows — ordering property, not a magic number.
TEST_F(PluginIntegrationTest, ReportedLatency_IsMonotonicDecreasingWithHostBlockAt44k) {
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(44100.0, 32);
    const int chainLatency32 = proc.getLatencySamples() - 32;
    proc.releaseResources();
    proc.prepareToPlay(44100.0, 512);
    const int chainLatency512 = proc.getLatencySamples() - 512;
    EXPECT_GT(chainLatency32, chainLatency512) << "larger host blocks should reduce block-aligned ONNX delay";
    proc.releaseResources();
}

// Signal: silence → processBlock × pre-roll; host buffer size unchanged after each callback.
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
