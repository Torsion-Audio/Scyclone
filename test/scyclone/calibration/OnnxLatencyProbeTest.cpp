// Calibration probe — measures anira's reported ONNX-island latency across host block sizes.
// Re-run when the model config, anira, or ONNX Runtime changes; update
// kOnnxInferenceLatencySamples in source/dsp/onnx/OnnxInferenceLatency.h from the 48 kHz /
// 512-sample row, which is the pair the constant is defined at.
//
// Run: Test.exe --gtest_filter=*PrintOnnxLatencyMeasurements* --gtest_also_run_disabled_tests

#include <gtest/gtest.h>
#include <iostream>

#include "JuceHeader.h"
#include "OnnxInferenceLatency.h"

#if !defined(SCYCLONE_INFERENCE_STUB) && !defined(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST)

#include "ScycloneModelConfig.h"
#include <anira/anira.h>

namespace
{
    unsigned int measureLatency(RaveModel model, double sampleRate, int blockSize)
    {
        auto inferenceConfig = makeScycloneInferenceConfig(model);
        anira::PrePostProcessor ppProcessor(inferenceConfig);
        anira::ContextConfig contextConfig(2);
        anira::InferenceHandler handler(ppProcessor, inferenceConfig, contextConfig);

        handler.prepare(anira::HostConfig{static_cast<float>(blockSize),
                                         static_cast<float>(sampleRate)});
        return handler.get_latency();
    }
} // namespace

TEST(DISABLED_OnnxLatencyProbe, PrintOnnxLatencyMeasurements)
{
    for (const int blockSize : {32, 64, 128, 256, 512, 1024, 2048})
    {
        const unsigned int funk = measureLatency(FunkDrum, 48000.0, blockSize);
        const unsigned int djembe = measureLatency(Djembe, 48000.0, blockSize);

        std::cout << "CALIBRATION onnx_latency sampleRate=48000 block=" << blockSize
                  << " funk=" << funk << " djembe=" << djembe << "\n";
    }

    std::cout << "\nkOnnxInferenceLatencySamples is currently " << kOnnxInferenceLatencySamples
              << " (defined at 48 kHz / 512-sample blocks).\n";
}

#else

TEST(DISABLED_OnnxLatencyProbe, PrintOnnxLatencyMeasurements)
{
    GTEST_SKIP() << "OnnxLatencyProbe requires linked ONNX Runtime / anira";
}

#endif
