#include <gtest/gtest.h>
#include "PluginProcessor.h"
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

TEST_F(OnnxProcessorContractTest, Prepare_ReportsLowerLatencyThanLegacy)
{
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(48000.0, 512);
    EXPECT_GT(proc.getLatencySamples(), 0);
    EXPECT_LT(proc.getLatencySamples(), 20480);
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

TEST_F(OnnxProcessorContractTest, DjembeInternalModel_Loads)
{
    AudioPluginAudioProcessor proc;
    proc.prepareToPlay(48000.0, 512);
    EXPECT_GT(proc.getLatencySamples(), 0);
    proc.releaseResources();
}
