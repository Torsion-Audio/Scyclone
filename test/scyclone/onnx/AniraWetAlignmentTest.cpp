// Pins the wet path's timing against the latency the backend reports. The reported latency delays
// the dry path and drives host delay compensation, so a wrong internal_model_latency in
// ScycloneModelConfig.cpp misaligns dry and wet by the difference — which no other test can see,
// because they never let real inference finish in time. Uses the burst estimator from
// WetLagMeasurement.h; OnnxWetLagProbeTest.cpp prints the full measurement.

#include <gtest/gtest.h>
#include "JuceHeader.h"

#ifndef SCYCLONE_INFERENCE_STUB

#include "AniraInferenceBackend.h"
#include "BackendRender.h"
#include "WetLagMeasurement.h"
#include <anira/anira.h>

using namespace scyclone::test::wetlag;

// These ONNX exports process each 2048-sample hop on its own and resolve time only to the hop:
// FunkDrum emits the response to a transient at the start of the output hop, wherever in the
// input hop the transient was. So the per-burst lag varies by up to one hop, but the hop-aligned
// onset (lag + the burst's position within its hop) lands on the reported latency. A declared
// internal_model_latency that is off by a hop moves it by a whole hop.
//
// Only FunkDrum is pinned: Djembe adds transients of its own, which the onset detector cannot
// tell apart from responses (see the probe), and both models share one config and latency path.
TEST(AniraWetAlignmentTest, FunkDrum_HopAlignedWetOnset_MatchesReportedLatency)
{
    anira::ContextConfig contextConfig{2, anira::WaitStrategy::SpinBackoff, anira::LogLevel::Error};
    AniraInferenceBackend backend(FunkDrum, contextConfig);
    backend.setNonRealtime(true); // every hop is computed, so output positions are deterministic
    backend.prepare(juce::dsp::ProcessSpec{48000.0, 512, 1});
    const int reported = backend.getLatencyInSamples();

    StimulusSpec spec;
    spec.burstsPerPhase = 2;
    const auto stimulus = makeBurstStimulus(spec);

    // Wide enough to see the wet signal arriving a hop and a half early or two hops late, so a
    // misdeclared latency shows up as a wrong onset rather than as missing responses.
    LagSearch search;
    search.minLag = std::max(0, reported - 3072);
    search.maxLag = reported + 4096;
    const auto estimate = estimateLag(stimulus, renderThroughBackend(backend, stimulus.samples, 512), search);

    ASSERT_GE(estimate.responseRate, 0.75)
        << "too few bursts produced a detectable response (" << estimate.responses << "/"
        << estimate.bursts << ") — the model output may be silent or the estimator inconclusive";
    EXPECT_NEAR(estimate.medianHopAlignedOnset, reported, 512.0)
        << "the wet path starts " << (estimate.medianHopAlignedOnset - reported)
        << " samples off the reported latency " << reported
        << " — re-measure internal_model_latency with OnnxWetLagProbe";
}

#endif
