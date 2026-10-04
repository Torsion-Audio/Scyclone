#include <gtest/gtest.h>
#include "JuceHeader.h"
#include "InferenceBackend.h"
#include "OnnxInferenceLatency.h"
#include "BinaryData.h"
#include "ScopedTempModel.h"

#ifndef SCYCLONE_INFERENCE_STUB
#include <anira/anira.h>
#include "AniraInferenceBackend.h"
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
    anira::ContextConfig contextConfig{2, anira::WaitStrategy::SpinBackoff, anira::LogLevel::Error};
#endif
    std::unique_ptr<InferenceBackend> backend;
    std::vector<bool> initEvents;
    juce::String lastModelName;
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

// anira's latency calculator throws for a zero block size or sample rate. A host that briefly
// reports one must not take the plugin down, and a later valid prepare must still work.
TEST_F(InferenceBackendContractTest, Prepare_IgnoresUnusableSpec)
{
    EXPECT_NO_THROW(backend->prepare(juce::dsp::ProcessSpec{0.0, 0, 1}));
    EXPECT_NO_THROW(backend->prepare(juce::dsp::ProcessSpec{48000.0, 0, 1}));

    juce::AudioBuffer<float> buffer(1, 512);
    buffer.clear();
    EXPECT_NO_THROW(backend->processBlock(buffer));

    backend->prepare(juce::dsp::ProcessSpec{48000.0, 512, 1});
    EXPECT_GT(backend->getLatencyInSamples(), 0);
}

#ifndef SCYCLONE_INFERENCE_STUB
// Regression: before anira v2.3.0 a session started on the CUSTOM backend, which copies input
// to output, and nothing selected ONNX — the RAVE model was loaded but never run. Output-based
// checks cannot catch that reliably (late inference also yields silence), so assert the
// selected backend directly, both after the first build and after a model swap rebuilds it.
TEST(AniraInferenceBackendTest, RunsTheOnnxModelNotPassthrough)
{
    anira::ContextConfig contextConfig{2, anira::WaitStrategy::SpinBackoff, anira::LogLevel::Error};
    AniraInferenceBackend backend(FunkDrum, contextConfig);

    EXPECT_EQ(backend.activeAniraBackend(), anira::InferenceBackend::CUSTOM)
        << "no pipeline exists before prepare";

    backend.prepare(juce::dsp::ProcessSpec{48000.0, 512, 1});
    EXPECT_EQ(backend.activeAniraBackend(), anira::InferenceBackend::ONNX);

    ASSERT_TRUE(backend.setInternalModel());
    EXPECT_EQ(backend.activeAniraBackend(), anira::InferenceBackend::ONNX)
        << "a rebuilt pipeline must select ONNX again";
}

// Regression: releaseResources() drops the session but keeps the model, and the next prepare()
// rebuilds it. If that rebuild re-read an external model from disk, a file that was moved or
// deleted in between made prepare() throw straight out of prepareToPlay into the host.
TEST(AniraInferenceBackendTest, Prepare_AfterReleaseWithDeletedExternalModel_DoesNotThrow)
{
    anira::ContextConfig contextConfig{2, anira::WaitStrategy::SpinBackoff, anira::LogLevel::Error};
    AniraInferenceBackend backend(FunkDrum, contextConfig);
    const juce::dsp::ProcessSpec spec{48000.0, 512, 1};
    backend.prepare(spec);
    const int latency = backend.getLatencyInSamples();

    juce::File modelFile;
    {
        const ScopedTempModel model("scyclone_vanishing_model");
        modelFile = model.get();
        ASSERT_TRUE(backend.loadExternalModel(modelFile));
        backend.releaseResources();
    }
    ASSERT_FALSE(modelFile.exists()) << "the temp model must be gone for this test to mean anything";

    EXPECT_NO_THROW(backend.prepare(spec));
    EXPECT_EQ(backend.activeAniraBackend(), anira::InferenceBackend::ONNX)
        << "the external model should still be loaded";
    EXPECT_EQ(backend.getLatencyInSamples(), latency);
}

// An empty file is rejected before anira sees it (anira's ModelData asserts on a zero size), the
// completion callback still fires, and the previous model keeps running.
TEST(AniraInferenceBackendTest, LoadExternalModel_EmptyFile_ReportsFailureAndResumes)
{
    anira::ContextConfig contextConfig{2, anira::WaitStrategy::SpinBackoff, anira::LogLevel::Error};
    AniraInferenceBackend backend(FunkDrum, contextConfig);
    std::vector<bool> events;
    backend.onModelLoad = [&events](bool initLoading, juce::String) { events.push_back(initLoading); };
    backend.prepare(juce::dsp::ProcessSpec{48000.0, 512, 1});
    const int latency = backend.getLatencyInSamples();

    const juce::File empty = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                 .getNonexistentChildFile("scyclone_empty_model", ".onnx", false);
    ASSERT_TRUE(empty.create());

    EXPECT_FALSE(backend.loadExternalModel(empty));
    ASSERT_EQ(events.size(), 2u);
    EXPECT_FALSE(events.back()) << "completion callback must fire for a rejected file";
    EXPECT_EQ(backend.activeAniraBackend(), anira::InferenceBackend::ONNX);
    EXPECT_EQ(backend.getLatencyInSamples(), latency);

    empty.deleteFile();
}

// Windows paths used to be widened byte by byte from UTF-8, so a model under e.g.
// C:\Users\Jürgen could not be opened. Models are now read through juce::File.
TEST(AniraInferenceBackendTest, LoadExternalModel_NonAsciiPath_Loads)
{
    anira::ContextConfig contextConfig{2, anira::WaitStrategy::SpinBackoff, anira::LogLevel::Error};
    AniraInferenceBackend backend(FunkDrum, contextConfig);
    backend.prepare(juce::dsp::ProcessSpec{48000.0, 512, 1});

    const ScopedTempModel model(
        juce::String::fromUTF8("scyclone_\xc3\xbcml\xc3\xa4ut_\xe6\xa8\xa1\xe5\x9e\x8b"));
    ASSERT_TRUE(model.get().existsAsFile());

    EXPECT_TRUE(backend.loadExternalModel(model.get()));
    EXPECT_EQ(backend.activeAniraBackend(), anira::InferenceBackend::ONNX);
}
#endif

#ifndef SCYCLONE_INFERENCE_STUB
namespace {

/// Feeds noise as fast as possible (like an offline bounce) and counts the blocks after the
/// reported latency that came back entirely silent, i.e. dropped hops.
int countSilentBlocksInTightLoop(InferenceBackend& backend, int checkedBlocks, juce::Random& random)
{
    constexpr int kBlockSize = 512;
    const int settleBlocks = backend.getLatencyInSamples() / kBlockSize + 1;
    juce::AudioBuffer<float> buffer(1, kBlockSize);
    int silentBlocks = 0;

    for (int n = 0; n < settleBlocks + checkedBlocks; ++n)
    {
        for (int i = 0; i < kBlockSize; ++i)
            buffer.setSample(0, i, 0.5f * (random.nextFloat() * 2.0f - 1.0f));

        backend.processBlock(buffer);

        if (n >= settleBlocks && isSilent(buffer))
            ++silentBlocks;
    }
    return silentBlocks;
}

} // namespace

// The offline flag is set before the first pipeline exists (hosts call setNonRealtime before
// prepareToPlay) and every model swap builds a new anira session that starts in realtime mode,
// so the flag must be re-applied to each new session.
TEST(AniraInferenceBackendTest, NonRealtime_SetBeforePrepare_AppliesAndSurvivesModelSwap)
{
    anira::ContextConfig contextConfig{2, anira::WaitStrategy::SpinBackoff, anira::LogLevel::Error};
    AniraInferenceBackend backend(FunkDrum, contextConfig);
    juce::Random random(11);

    backend.setNonRealtime(true);
    backend.prepare(juce::dsp::ProcessSpec{48000.0, 512, 1});
    EXPECT_EQ(countSilentBlocksInTightLoop(backend, 32, random), 0)
        << "hops were dropped although the backend was set to non-realtime before prepare";

    ASSERT_TRUE(backend.setInternalModel());
    EXPECT_EQ(countSilentBlocksInTightLoop(backend, 32, random), 0)
        << "hops were dropped after a model swap: the new session lost the non-realtime flag";
}
#endif

// A backend with no prepared pipeline (before prepare, or after a failed rebuild) must emit
// silence. Passing its input through would put undelayed audio on the wet path while the dry
// path is delayed by the reported latency.
TEST_F(InferenceBackendContractTest, ProcessBlock_WhenNotPrepared_EmitsSilence)
{
    juce::AudioBuffer<float> buffer(1, 512);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        buffer.setSample(0, i, 0.25f);

    backend->processBlock(buffer);

    EXPECT_TRUE(isSilent(buffer)) << "an unprepared backend passed its input through";
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

    // Both backends are silent for exactly the reported latency: the stub is a delay line, and
    // anira pre-fills its receive buffer with (latency - internal_model_latency) zeros, which is
    // the whole latency since the RAVE config declares no internal model latency.
    const int silentBlocks = latency / 512;

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
