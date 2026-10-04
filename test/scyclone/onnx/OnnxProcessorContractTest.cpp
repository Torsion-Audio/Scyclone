// Plugin-level ONNX contracts that need the real anira/ORT backend.
// Block-size ordering lives in PluginIntegrationTest; per-backend behaviour in
// InferenceBackendContractTest.

#include <gtest/gtest.h>
#include "PluginProcessor.h"
#include "OnnxInferenceLatency.h"
#include "ParameterHelpers.h"
#include "PluginParameters.h"
#include "ScopedTempModel.h"
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
        << "kOnnxInferenceLatencySamples is stale â€” re-run OnnxLatencyProbeTest";

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
// suspended forever â€” a permanently silent plugin â€” with the exception escaping into JUCE's
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

// Regression: a host that deactivates and reactivates the plugin (sample-rate or buffer-size
// change) after the user's external model file was moved or deleted must not get an exception
// out of prepareToPlay, and the plugin must not be left suspended.
TEST_F(OnnxProcessorContractTest, PrepareToPlay_AfterReleaseWithDeletedExternalModel_DoesNotThrow)
{
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(48000.0, 512);
    const int latency = proc.getLatencySamples();

    juce::File modelFile;
    {
        const ScopedTempModel model("scyclone_vanishing_plugin_model");
        modelFile = model.get();
        ASSERT_TRUE(proc.loadExternalModel(modelFile, 1));
        proc.releaseResources();
    }
    ASSERT_FALSE(modelFile.exists());

    EXPECT_NO_THROW(proc.prepareToPlay(48000.0, 512));
    EXPECT_FALSE(proc.isSuspended());
    EXPECT_EQ(proc.getLatencySamples(), latency);

    proc.releaseResources();
}

// Regression: anira is non-blocking by default â€” a hop whose inference has not finished comes
// back as zeros and is dropped. An offline bounce runs faster than real time, exactly like this
// loop, so without honouring setNonRealtime() the rendered wet signal is full of silent gaps.
// getCurrentLevel(n) is the unsmoothed magnitude of network n's inference output for the last
// block, so it is exactly 0 only for a dropped (zero-filled) block.
TEST_F(OnnxProcessorContractTest, SetNonRealtime_TightLoop_NetworksNeverDropHops)
{
    AudioPluginAudioProcessor proc;
    setBoolParameterById(proc, PluginParameters::ON_OFF_NETWORK1_ID.getParamID(), true);
    setBoolParameterById(proc, PluginParameters::ON_OFF_NETWORK2_ID.getParamID(), true);
    proc.setNonRealtime(true);
    proc.prepareToPlay(48000.0, 512);

    constexpr int kBlockSize = 512;
    const int settleBlocks = proc.getLatencySamples() / kBlockSize + 2;
    constexpr int kCheckedBlocks = 64;

    juce::Random random(7);
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    juce::MidiBuffer midi;
    int silentBlocks1 = 0;
    int silentBlocks2 = 0;

    for (int n = 0; n < settleBlocks + kCheckedBlocks; ++n)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            for (int i = 0; i < kBlockSize; ++i)
                buffer.setSample(ch, i, 0.5f * (random.nextFloat() * 2.0f - 1.0f));

        proc.processBlock(buffer, midi);

        if (n >= settleBlocks)
        {
            silentBlocks1 += proc.getCurrentLevel(1) == 0.0f ? 1 : 0;
            silentBlocks2 += proc.getCurrentLevel(2) == 0.0f ? 1 : 0;
        }
    }

    EXPECT_EQ(silentBlocks1, 0) << "network 1 dropped hops while rendering offline";
    EXPECT_EQ(silentBlocks2, 0) << "network 2 dropped hops while rendering offline";

    proc.releaseResources();
}
