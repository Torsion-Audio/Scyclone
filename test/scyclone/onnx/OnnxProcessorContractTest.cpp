// Plugin-level ONNX contracts that need the real anira/ORT backend.
// Block-size ordering lives in PluginIntegrationTest; per-backend behaviour in
// InferenceBackendContractTest.

#include <gtest/gtest.h>
#include "PluginProcessor.h"
#include "OnnxInferenceLatency.h"
#include "TestTiming.h"

using namespace torsion::test;

#if defined(SCYCLONE_INFERENCE_STUB) || defined(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST)
#define SCYCLONE_SKIP_ONNX_PROCESSOR_CONTRACT 1
#endif

class OnnxProcessorContractTest : public ::testing::Test {
protected:
    void SetUp() override
    {
#if defined(SCYCLONE_SKIP_ONNX_PROCESSOR_CONTRACT)
        GTEST_SKIP() << "OnnxProcessorContractTest requires Anira/ORT (release build)";
#endif
    }
};

// Pins kOnnxInferenceLatencySamples against the live backend. The stub backend and every
// resampling contract test are built on that constant, so silent drift after an anira or ORT
// bump would invalidate them without failing anything. Re-measure with
// test/scyclone/calibration/OnnxLatencyProbeTest.cpp if this fires.
TEST_F(OnnxProcessorContractTest, ReportedLatency_MatchesCalibratedConstantAtReferenceConfig)
{
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(kOnnxInferenceLatencyReferenceSampleRate,
                       kOnnxInferenceLatencyReferenceBlockSize);

    // At the reference sample rate there is no resampling, so the plugin reports the raw
    // ONNX-island latency plus one host block for the fixed-block FIFO.
    EXPECT_EQ(proc.getLatencySamples() - kOnnxInferenceLatencyReferenceBlockSize,
              kOnnxInferenceLatencySamples)
        << "kOnnxInferenceLatencySamples is stale — re-run OnnxLatencyProbeTest";

    proc.releaseResources();
}

TEST_F(OnnxProcessorContractTest, Prepare_IsIdempotent)
{
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(48000.0, 512);
    const int first = proc.getLatencySamples();
    proc.releaseResources();
    proc.prepareToPlay(48000.0, 512);
    EXPECT_EQ(proc.getLatencySamples(), first);
    proc.releaseResources();
}

TEST_F(OnnxProcessorContractTest, ProcessBlock_PreservesNumSamples)
{
    AudioPluginAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;
    buffer.clear();

    for (int n = 0; n < kPreRollBlocks; ++n)
    {
        processor.processBlock(buffer, midi);
        EXPECT_EQ(buffer.getNumSamples(), 512);
    }

    processor.releaseResources();
}

// Regression: a model file that is not a valid RAVE export used to throw out of
// AniraInferenceBackend, so onOnnxModelLoad(false, ...) never fired and the processor stayed
// suspended forever — a permanently silent plugin — with the exception escaping into JUCE's
// async file-chooser callback.
TEST_F(OnnxProcessorContractTest, LoadExternalModel_InvalidFile_KeepsPreviousModelAndResumes)
{
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(48000.0, 512);
    const int latencyBefore = proc.getLatencySamples();
    ASSERT_GT(latencyBefore, 0);

    const juce::File garbage = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                   .getChildFile("scyclone_not_a_model.onnx");
    garbage.deleteFile();
    garbage.replaceWithText("this is not an ONNX model");
    ASSERT_TRUE(garbage.existsAsFile());

    bool loaded = true;
    EXPECT_NO_THROW(loaded = proc.loadExternalModel(garbage, 1));
    EXPECT_FALSE(loaded) << "an invalid model must be reported as a failure";

    EXPECT_FALSE(proc.isSuspended()) << "a failed load must not leave the processor suspended";
    EXPECT_EQ(proc.getLatencySamples(), latencyBefore) << "previous model should still be loaded";

    // And the plugin still processes audio.
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;
    buffer.clear();
    for (int n = 0; n < kPreRollBlocks; ++n)
    {
        EXPECT_NO_THROW(proc.processBlock(buffer, midi));
        EXPECT_EQ(buffer.getNumSamples(), 512);
    }

    proc.releaseResources();
    garbage.deleteFile();
}
