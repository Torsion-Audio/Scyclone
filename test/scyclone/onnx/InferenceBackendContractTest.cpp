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

/// Writes the embedded model to a temp file and deletes it again on scope exit.
class ScopedTempModel
{
public:
    ScopedTempModel()
        : file(juce::File::getSpecialLocation(juce::File::tempDirectory)
                   .getChildFile("scyclone_contract_funk_drums.ort"))
    {
        file.deleteFile();
        file.create();
        juce::FileOutputStream stream(file);
        if (stream.openedOk())
            stream.write(BinaryData::funk_drums_ort, BinaryData::funk_drums_ortSize);
    }

    ~ScopedTempModel() { file.deleteFile(); }

    ScopedTempModel(const ScopedTempModel&) = delete;
    ScopedTempModel& operator=(const ScopedTempModel&) = delete;

    const juce::File& get() const { return file; }

private:
    juce::File file;
};

bool isSilent(const juce::AudioBuffer<float>& buffer)
{
    return buffer.getMagnitude(0, buffer.getNumSamples()) == 0.0f;
}

bool allFinite(const juce::AudioBuffer<float>& buffer)
{
    const float* data = buffer.getReadPointer(0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        if (!std::isfinite(data[i]))
            return false;
    return true;
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

// The stub is a pure delay line, so it round-trips its input exactly. The real backend is a
// generative model — it does not preserve the signal, so the assertable contract is that it
// honours its own reported latency and never emits non-finite samples.
TEST_F(InferenceBackendContractTest, ProcessBlock_HonoursReportedLatency)
{
    const juce::dsp::ProcessSpec spec{48000.0, 512, 1};
    backend->prepare(spec);
    const int latency = backend->getLatencyInSamples();
    ASSERT_GT(latency, 0);

    auto fillWithTone = [](juce::AudioBuffer<float>& buffer) {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.setSample(0, i, 0.25f);
    };

    juce::AudioBuffer<float> buffer(1, 512);

#if defined(SCYCLONE_INFERENCE_STUB)
    // The stub is an exact delay line: silent for the whole reported latency.
    const int silentBlocks = latency / 512;
#else
    // The real backend emits sooner than its reported latency. anira pre-fills the receive
    // buffer with (latency - internal_model_latency) zeros — 5632 - 2048 = 3584 samples at the
    // reference config — because the declared internal_model_latency accounts for the delay the
    // model itself contributes. Assert only what is unambiguous: nothing can come out before
    // RAVE's first 2048-sample hop has been consumed.
    const int silentBlocks = 2048 / 512;
#endif

    for (int n = 0; n < silentBlocks; ++n)
    {
        fillWithTone(buffer);
        backend->processBlock(buffer);
        EXPECT_EQ(buffer.getNumSamples(), 512);
        EXPECT_TRUE(allFinite(buffer)) << "block " << n;
        EXPECT_TRUE(isSilent(buffer)) << "output before the first hop must be silent, block " << n;
    }

#if defined(SCYCLONE_INFERENCE_STUB)
    // Past the latency the delay line hands the input straight back.
    for (int n = 0; n < 4; ++n)
    {
        fillWithTone(buffer);
        backend->processBlock(buffer);
    }
    for (int i = 0; i < 512; ++i)
        EXPECT_FLOAT_EQ(buffer.getSample(0, i), 0.25f);
#else
    for (int n = 0; n < 8; ++n)
    {
        fillWithTone(buffer);
        backend->processBlock(buffer);
        EXPECT_EQ(buffer.getNumSamples(), 512);
        EXPECT_TRUE(allFinite(buffer)) << "steady-state block " << n;
    }
#endif
}

// Muting must skip inference without desynchronising the pipeline, and unmuting must not
// replay audio captured before the mute.
TEST_F(InferenceBackendContractTest, Muted_EmitsSilenceAndDoesNotLeakPreMuteAudioOnUnmute)
{
    const juce::dsp::ProcessSpec spec{48000.0, 512, 1};
    backend->prepare(spec);
    const int latency = backend->getLatencyInSamples();
    ASSERT_GT(latency, 0);

    juce::AudioBuffer<float> buffer(1, 512);

    auto runBlocks = [&](int count, float value) {
        for (int n = 0; n < count; ++n)
        {
            for (int i = 0; i < 512; ++i)
                buffer.setSample(0, i, value);
            backend->processBlock(buffer);
        }
    };

    // Fill the pipeline with a loud signal, then mute.
    runBlocks(latency / 512 + 4, 0.5f);

    backend->setMuted(true);
    for (int n = 0; n < 4; ++n)
    {
        for (int i = 0; i < 512; ++i)
            buffer.setSample(0, i, 0.5f);
        backend->processBlock(buffer);
        EXPECT_TRUE(isSilent(buffer)) << "a muted backend must emit silence, block " << n;
    }

    // Unmute with silent input: anything non-zero coming out is pre-mute audio.
    backend->setMuted(false);
    for (int n = 0; n < latency / 512; ++n)
    {
        buffer.clear();
        backend->processBlock(buffer);
        EXPECT_TRUE(allFinite(buffer));
        EXPECT_TRUE(isSilent(buffer)) << "pre-mute audio leaked after unmute, block " << n;
    }
}

TEST_F(InferenceBackendContractTest, LoadExternalModel_FiresCallbackAndSucceeds)
{
    initEvents.clear();
    const ScopedTempModel model;
    ASSERT_TRUE(model.get().existsAsFile());

    bool loaded = false;
    ASSERT_NO_THROW(loaded = backend->loadExternalModel(model.get()));
    EXPECT_TRUE(loaded);

    ASSERT_GE(initEvents.size(), 2u);
    EXPECT_TRUE(initEvents.front());
    EXPECT_FALSE(initEvents.back());
    EXPECT_EQ(lastModelName, model.get().getFileNameWithoutExtension());
}

// Regression: the completion callback used to be skipped when the rebuild threw, leaving the
// host suspended. It must fire on the failure path too, and the previous model must survive.
TEST_F(InferenceBackendContractTest, LoadExternalModel_InvalidFile_ReportsFailureAndKeepsWorking)
{
    const juce::dsp::ProcessSpec spec{48000.0, 512, 1};
    backend->prepare(spec);
    const int latencyBefore = backend->getLatencyInSamples();

    const juce::File garbage = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                   .getChildFile("scyclone_contract_invalid.onnx");
    garbage.deleteFile();
    garbage.replaceWithText("definitely not a model");

    initEvents.clear();
    bool loaded = true;
    ASSERT_NO_THROW(loaded = backend->loadExternalModel(garbage));

#if defined(SCYCLONE_INFERENCE_STUB)
    // The stub has no real ORT session to fail against.
    EXPECT_TRUE(loaded);
#else
    EXPECT_FALSE(loaded);
    EXPECT_EQ(backend->getLatencyInSamples(), latencyBefore) << "previous model should survive";
#endif

    ASSERT_GE(initEvents.size(), 2u);
    EXPECT_TRUE(initEvents.front());
    EXPECT_FALSE(initEvents.back()) << "completion callback must fire even when the load fails";

    juce::AudioBuffer<float> buffer(1, 512);
    buffer.clear();
    EXPECT_NO_THROW(backend->processBlock(buffer));
    EXPECT_TRUE(allFinite(buffer));

    garbage.deleteFile();
}
