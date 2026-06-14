// Wrapper-level structural tests — libsamplerate streaming_test / termination_test patterns.
// Validates per-block frame invariants on ResamplingProcessor after warmup.

#include <gtest/gtest.h>
#include "HostConfigCatalog.h"
#include "ResamplingContractAssertions.h"
#include "ResamplingFixtures.h"

using namespace resampling_test;

// Signal: silence immediately after prepare; first partial SRC block must zero unwritten tail.
TEST_P(ProcessorStructuralTest, Warmup_PartialBlockTailZeroed)
{
    const auto &testCase = GetParam();
    skipIfInfeasible(testCase.cfg);

    ResamplingProcessor proc;
    juce::AudioBuffer<float> buf;
    int expectedIn = 0;
    int expectedOut = 0;
    setupProcessorStructuralCase(testCase, proc, buf, expectedIn, expectedOut);
    juce::ignoreUnused(expectedIn, expectedOut);

    assertWarmupPartialBlockTailZeroed(proc, buf);
}

// Signal: silence → up or down; steady state full input consumed and full output generated.
TEST_P(ProcessorStructuralTest, SteadyState_FullInputOutput)
{
    const auto &testCase = GetParam();
    skipIfInfeasible(testCase.cfg);

    ResamplingProcessor proc;
    juce::AudioBuffer<float> buf;
    int expectedIn = 0;
    int expectedOut = 0;
    setupProcessorStructuralCase(testCase, proc, buf, expectedIn, expectedOut);

    runProcessorSilencePreRoll(proc, buf, kPreRollBlocks);
    assertProcessorSteadyStateFullIo(proc, buf, expectedIn, expectedOut);
}

// Signal: 100 blocks silence → up or down; frame count conservation over long run.
TEST_P(ProcessorStructuralTest, LongRun_FrameCountConservation)
{
    const auto &testCase = GetParam();
    skipIfInfeasible(testCase.cfg);

    ResamplingProcessor proc;
    juce::AudioBuffer<float> buf;
    int expectedIn = 0;
    int expectedOut = 0;
    setupProcessorStructuralCase(testCase, proc, buf, expectedIn, expectedOut);
    juce::ignoreUnused(expectedOut);

    runProcessorSilencePreRoll(proc, buf, kPreRollBlocks);

    const auto counts = runLongBlockLoop(kLongRunBlocks, [&](int, LongRunCounts &c)
    {
        buf.clear();
        c.inputTotal += buf.getNumSamples();
        (void)proc.processBlock(buf);
        c.outputTotal += proc.getLastOutputFramesGenerated();
    });

    const double effectiveRatio = static_cast<double>(counts.outputTotal) / static_cast<double>(counts.inputTotal);
    const int tol = longRunTolerance(effectiveRatio);
    const long expectedTotalOut = static_cast<long>(std::llround(effectiveRatio * static_cast<double>(counts.inputTotal)));
    EXPECT_LE(std::abs(counts.outputTotal - expectedTotalOut), tol)
        << "hostSR=" << testCase.cfg.hostSR << " srcRatio=" << proc.getSrcRatio();
}

// Lifecycle: prepare → pre-roll → re-prepare; reported latency stable and positive after warmup.
TEST(ResamplingStructural, PrepareRelease_Prepare_LatencyStable)
{
    int latencyAfterWarmup = 0;
    {
        ResamplingProcessor up;
        prepareUpOnly(44100.0, 512, up);
        juce::AudioBuffer<float> buf(1, 512);
        runProcessorSilencePreRoll(up, buf, kPreRollBlocks);
        latencyAfterWarmup = up.getLatencyInSamples();
    }
    {
        ResamplingProcessor up;
        prepareUpOnly(44100.0, 512, up);
        EXPECT_EQ(up.getLatencyInSamples(), latencyAfterWarmup);
    }

    const std::vector<std::pair<double, int>> warmupCases{
        {48000.0, 512},
    };
    for (const auto &[hostSR, hostBlock] : warmupCases)
    {
        ResamplingProcessor up;
        prepareUpOnly(hostSR, hostBlock, up);
        juce::AudioBuffer<float> buf(1, hostBlock);
        runProcessorSilencePreRoll(up, buf, kPreRollBlocks);
        EXPECT_GT(up.getLatencyInSamples(), 0) << "hostSR=" << hostSR;
    }
}

INSTANTIATE_TEST_SUITE_P(ProcessorStructural, ProcessorStructuralTest,
                         processorStructuralValuesFor(defaultCiHostConfigs()), processorStructuralCaseName);
