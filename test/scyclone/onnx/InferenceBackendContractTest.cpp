#include <gtest/gtest.h>
#include "JuceHeader.h"
#include "InferenceBackend.h"
#include "OnnxInferenceLatency.h"
#include "BinaryData.h"

#ifndef SCYCLONE_INFERENCE_STUB
#include <anira/anira.h>
#endif

namespace {

class InferenceBackendContractTest : public ::testing::Test {
protected:
    void SetUp() override
    {
#ifndef SCYCLONE_INFERENCE_STUB
        backend = makeInferenceBackend(FunkDrum, contextConfig);
#else
        backend = makeInferenceBackend(FunkDrum);
#endif
        backend->onModelLoad = [this](bool initLoading, juce::String modelName) {
            initEvents.push_back(initLoading);
            if (modelName.isNotEmpty())
                lastModelName = modelName;
        };
    }

#ifndef SCYCLONE_INFERENCE_STUB
    anira::ContextConfig contextConfig{2};
#endif
    std::unique_ptr<InferenceBackend> backend;
    std::vector<bool> initEvents;
    juce::String lastModelName;
};

juce::File writeEmbeddedModelToTemp()
{
    const juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("scyclone_contract_funk_drums.ort");
    tempFile.deleteFile();
    tempFile.create();
    juce::FileOutputStream stream(tempFile);
    if (stream.openedOk())
        stream.write(BinaryData::funk_drums_ort, BinaryData::funk_drums_ortSize);
    return tempFile;
}

} // namespace

TEST_F(InferenceBackendContractTest, Prepare_ReportsConsistentLatency)
{
    const juce::dsp::ProcessSpec spec{48000.0, 512, 1};
    backend->prepare(spec);
    const int first = backend->getLatencyInSamples();
    EXPECT_GT(first, 0);
    backend->prepare(spec);
    EXPECT_EQ(backend->getLatencyInSamples(), first);
}

TEST_F(InferenceBackendContractTest, ProcessBlock_PreservesAudio)
{
    const juce::dsp::ProcessSpec spec{48000.0, 512, 1};
    backend->prepare(spec);

    juce::AudioBuffer<float> buffer(1, 512);
    for (int i = 0; i < 512; ++i)
        buffer.setSample(0, i, 0.25f);

#if defined(SCYCLONE_INFERENCE_STUB)
    const int preRollBlocks = static_cast<int>(kOnnxInferenceLatencySamples / 512) + 4;
    for (int n = 0; n < preRollBlocks; ++n)
        backend->processBlock(buffer);

    for (int i = 0; i < 512; ++i)
        EXPECT_FLOAT_EQ(buffer.getSample(0, i), 0.25f);
#else
    for (int n = 0; n < 4; ++n)
    {
        const int numSamples = buffer.getNumSamples();
        backend->processBlock(buffer);
        EXPECT_EQ(buffer.getNumSamples(), numSamples);
    }
#endif
}

TEST_F(InferenceBackendContractTest, LoadExternalModel_FiresCallback)
{
    initEvents.clear();
    const juce::File tempFile = writeEmbeddedModelToTemp();
    ASSERT_TRUE(tempFile.existsAsFile());

    ASSERT_NO_THROW(backend->loadExternalModel(tempFile));

    ASSERT_GE(initEvents.size(), 2u);
    EXPECT_TRUE(initEvents.front());
    EXPECT_FALSE(initEvents.back());
}
