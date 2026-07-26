#include <gtest/gtest.h>
#include "JuceHeader.h"
#include "TestTiming.h"

#if !defined(SCYCLONE_INFERENCE_STUB) && !defined(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST)
#include "ScycloneModelConfig.h"
#include <anira/anira.h>

TEST(AniraModelValidation, FunkDrumEmbedded_RunsAt2048Hop)
{
    auto inferenceConfig = makeScycloneInferenceConfig(FunkDrum);
    anira::PrePostProcessor ppProcessor(inferenceConfig);
    anira::ContextConfig contextConfig(2);
    anira::InferenceHandler handler(ppProcessor, inferenceConfig, contextConfig);

    anira::HostConfig hostConfig{512.0f, 48000.0f};
    ASSERT_NO_THROW(handler.prepare(hostConfig));

    juce::AudioBuffer<float> buffer(1, 512);
    buffer.clear();

    for (int block = 0; block < torsion::test::kPreRollBlocks; ++block)
    {
        ASSERT_NO_THROW(
            handler.process(buffer.getArrayOfWritePointers(),
                            static_cast<size_t>(buffer.getNumSamples())));
    }

    EXPECT_GT(handler.get_latency(), 0u);
    EXPECT_LT(static_cast<int>(handler.get_latency()), 20480);
}
#else
TEST(AniraModelValidation, SkippedOnStubOrUbsan)
{
    GTEST_SKIP() << "AniraModelValidation requires linked ONNX Runtime / Anira";
}
#endif
