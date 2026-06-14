// Plugin-level smoke and latency ordering properties.
// Why: full graph uses real ONNX — we test block contract and monotonicity, not impulse fidelity.

#include <gtest/gtest.h>
#include "PluginProcessor.h"
#include "TestInfrastructure.h"

using namespace resampling_test;

class PluginIntegrationTest : public JuceAudioTest {};

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
    const int latency32 = proc.getLatencySamples();
    proc.releaseResources();
    proc.prepareToPlay(44100.0, 512);
    const int latency512 = proc.getLatencySamples();
    EXPECT_GT(latency32, latency512) << "larger host blocks should reduce block-aligned ONNX delay";
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
