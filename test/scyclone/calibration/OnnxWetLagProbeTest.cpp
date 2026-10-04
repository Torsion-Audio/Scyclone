// Calibration probe — measures how far the real RAVE models delay transients and compares that
// with the latency anira reports. The reported latency is anira's buffering P plus the
// internal_model_latency declared in ScycloneModelConfig.cpp (P = reported - declared); the
// measured lag minus P is the model's actual latency L_m. If L_m differs from the declared value,
// the wet path is shifted against the dry path by the difference.
//
// Runs in non-realtime mode, so every hop is computed and output positions are deterministic.
// Several host block sizes are measured: P changes with the block size, L_m must not.
//
// Run: Test.exe --gtest_filter=*PrintWetLagMeasurements* --gtest_also_run_disabled_tests

#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>

#include "JuceHeader.h"

#if !defined(SCYCLONE_INFERENCE_STUB) && !defined(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST)

#include "AniraInferenceBackend.h"
#include "BackendRender.h"
#include "ScycloneModelConfig.h"
#include "WetLagMeasurement.h"
#include <anira/anira.h>

using namespace scyclone::test::wetlag;

namespace
{
    const char* modelName(RaveModel model)
    {
        return model == Djembe ? "djembe" : "funk_drums";
    }
} // namespace

TEST(DISABLED_OnnxWetLagProbe, PrintWetLagMeasurements)
{
    const auto stimulus = makeBurstStimulus();

    for (const RaveModel model : {FunkDrum, Djembe})
    {
        const int declared = static_cast<int>(
            makeScycloneInferenceConfig(model).get_internal_model_latency().at(0));

        for (const int blockSize : {128, 512, 2048})
        {
            anira::ContextConfig contextConfig{2, anira::WaitStrategy::SpinBackoff, anira::LogLevel::Error};
            AniraInferenceBackend backend(model, contextConfig);
            backend.setNonRealtime(true);
            backend.prepare(juce::dsp::ProcessSpec{48000.0, static_cast<juce::uint32>(blockSize), 1});

            const int reported = backend.getLatencyInSamples();
            const int buffering = reported - declared;

            // Wide enough to catch a wet signal up to a hop and a half early or two hops late.
            LagSearch search;
            search.minLag = std::max(0, buffering - 3072);
            search.maxLag = buffering + 4096;
            const auto output = renderThroughBackend(backend, stimulus.samples, blockSize);
            const auto estimate = estimateLag(stimulus, output, search);

            std::cout << std::fixed << std::setprecision(0)
                      << "CALIBRATION wet_lag model=" << modelName(model) << " block=" << blockSize
                      << " reported=" << reported << " declared_model_latency=" << declared
                      << " buffering=" << buffering
                      << " responses=" << estimate.responses << "/" << estimate.bursts
                      << " median_lag=" << estimate.medianLag
                      << " measured_model_latency=" << (estimate.medianLag - buffering)
                      << " mad=" << estimate.medianAbsoluteDeviation
                      << " max_phase_dev=" << estimate.maxPhaseDeviation
                      << " xcorr_lag=" << estimate.crossCorrelationLag
                      << " hop_aligned_onset=" << estimate.medianHopAlignedOnset
                      << " (vs reported " << (estimate.medianHopAlignedOnset - reported) << ")\n";

            std::cout << "  per-phase median lag:";
            for (const auto& [phase, lag] : estimate.phaseMedians)
                std::cout << " " << phase << "->" << lag;
            std::cout << "\n";
        }
    }
}

#else

TEST(DISABLED_OnnxWetLagProbe, PrintWetLagMeasurements)
{
    GTEST_SKIP() << "OnnxWetLagProbe requires linked ONNX Runtime / anira";
}

#endif
