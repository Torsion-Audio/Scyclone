// Resampling graph contract tests — see docs/resampling_architecture.md

#include <gtest/gtest.h>
#include <cmath>
#include "ResamplingTestHelpers.h"

using namespace resampling_test;

namespace {

struct HostConfig {
    double hostSR;
    int hostBlock;
};

class ResamplingContractsFast : public ::testing::TestWithParam<HostConfig> {};
class ResamplingContractsFull : public ::testing::TestWithParam<HostConfig> {};

constexpr int kPreRollBlocks = 4;
constexpr int kSteadyBlocks = 8;
constexpr int kLongRunBlocks = 100;

void skipIfInfeasible(const HostConfig& cfg) {
    if (!isFeasibleHostConfig(cfg.hostSR, cfg.hostBlock)) {
        GTEST_SKIP() << "up output < " << kMinOnnxBlock << " for hostSR=" << cfg.hostSR
                     << " block=" << cfg.hostBlock;
    }
}

} // namespace

// --- Isolation (1.4a) ---

TEST(ResamplingIsolation, UpOnly_OutputSize) {
    juce::ScopedJuceInitialiser_GUI init;
    ResamplingProcessor up;
    const int upOut = prepareUpOnly(44100.0, 512, up);
    EXPECT_EQ(upOut, 558);
    EXPECT_EQ(up.getOutputBufferSize(), 558);
}

TEST(ResamplingIsolation, DownOnly_OutputSize) {
    juce::ScopedJuceInitialiser_GUI init;
    ResamplingProcessor down;
    const int downOut = prepareDownOnly(44100.0, 558, 512, down);
    EXPECT_EQ(downOut, 513) << "uncoupled ceil on down produces +1 at 44.1k/512";
    EXPECT_EQ(down.getOutputBufferSize(), 513);
}

TEST(ResamplingIsolation, SameRate_Passthrough) {
    juce::ScopedJuceInitialiser_GUI init;
    ResamplingProcessor up, down;
    prepareUpOnly(48000.0, 512, up);
    prepareDownOnly(48000.0, 512, 512, down);
    EXPECT_EQ(up.getOutputBufferSize(), 512);
    EXPECT_EQ(down.getOutputBufferSize(), 512);
}

// --- Contract tests ---

TEST_P(ResamplingContractsFast, RoundTrip_OutputBlockEqualsHostBlock) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    chain.hostBuffer.clear();
    for (int n = 0; n < kSteadyBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        EXPECT_EQ(downOut.getNumSamples(), cfg.hostBlock)
            << "hostSR=" << cfg.hostSR << " block=" << cfg.hostBlock;
    }
}

TEST_P(ResamplingContractsFast, Upsampler_DefaultPrepare_Unchanged) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    ResamplingProcessor up;
    const int upOut = prepareUpOnly(cfg.hostSR, cfg.hostBlock, up);
    EXPECT_EQ(upOut, upOutputBlockSize(cfg.hostSR, cfg.hostBlock));
}

TEST_P(ResamplingContractsFast, Chain_OnnxBoundaryBlockSizesMatch) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    EXPECT_EQ(chain.down.getInputBufferSize(), chain.up.getOutputBufferSize());
}

TEST_P(ResamplingContractsFast, Warmup_AllowsPartialFirstBlock) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    chain.hostBuffer.clear();
    (void) chain.up.processBlock(chain.hostBuffer);
    assertWarmupAllowsPartialOutput(chain.up);
}

TEST_P(ResamplingContractsFast, SteadyState_FullFillAndInputConsumed) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    chain.hostBuffer.clear();
    for (int n = 0; n < kSteadyBlocks; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        assertSteadyStateFullOutput(chain.up);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        assertSteadyStateFullOutput(chain.down);
        juce::ignoreUnused(downOut);
    }
}

TEST_P(ResamplingContractsFast, LongRun_SampleCountConservation) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    long hostIn = 0;
    long hostOut = 0;
    chain.hostBuffer.clear();
    for (int n = 0; n < kLongRunBlocks; ++n) {
        hostIn += chain.hostBuffer.getNumSamples();
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        hostOut += downOut.getNumSamples();
    }
    const double effectiveRatio = static_cast<double>(hostOut) / static_cast<double>(hostIn);
    const int tol = longRunTolerance(effectiveRatio);
    EXPECT_LE(std::abs(hostOut - hostIn), tol)
        << "host in/out should match within streaming_test tolerance; hostSR=" << cfg.hostSR;
}

TEST_P(ResamplingContractsFast, PerBlock_InputOutputMonotonic) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    const auto acc = accumulateRoundTripPerBlock(chain, kSteadyBlocks);
    EXPECT_GT(acc.totalIn, 0);
    EXPECT_GT(acc.totalOut, 0);
}

TEST_P(ResamplingContractsFast, Latency_ReportedMatchesStreamingImpulse) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    const int upLat = chain.up.getLatencyInSamples();
    const int downLat = chain.down.getLatencyInSamples();
    EXPECT_GT(upLat, 0);
    EXPECT_GT(downLat, 0);
    EXPECT_LE(upLat, 100);
    EXPECT_LE(downLat, 100);

    runSilencePreRoll(chain, kPreRollBlocks);
    chain.hostBuffer.setSample(0, 0, 1.0f);
    const int expectedPeak = upLat + static_cast<int>(std::round(static_cast<double>(downLat) * cfg.hostSR / kOnnxRate));
    const int collectSamples = expectedPeak + cfg.hostBlock;
    std::vector<float> collected;
    collected.reserve(static_cast<size_t>(collectSamples));
    while (static_cast<int>(collected.size()) < collectSamples) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        for (int i = 0; i < downOut.getNumSamples(); ++i) {
            collected.push_back(downOut.getSample(0, i));
        }
        chain.hostBuffer.clear();
    }
    const int peakPos = findImpulsePeak(collected, expectedPeak - 5, expectedPeak + cfg.hostBlock);
    EXPECT_GE(peakPos, expectedPeak - 5);
    EXPECT_LE(peakPos, expectedPeak + 5);
}

INSTANTIATE_TEST_SUITE_P(
    TierA,
    ResamplingContractsFast,
    ::testing::Values(
        HostConfig{ 44100.0, 128 },
        HostConfig{ 44100.0, 512 },
        HostConfig{ 48000.0, 128 },
        HostConfig{ 48000.0, 512 }),
    [](const ::testing::TestParamInfo<HostConfig>& info) {
        return std::to_string(static_cast<int>(info.param.hostSR)) + "_" + std::to_string(info.param.hostBlock);
    });

INSTANTIATE_TEST_SUITE_P(
    TierB,
    ResamplingContractsFull,
    ::testing::Values(
        HostConfig{ 44100.0, 32 },
        HostConfig{ 44100.0, 64 },
        HostConfig{ 44100.0, 128 },
        HostConfig{ 44100.0, 256 },
        HostConfig{ 44100.0, 512 },
        HostConfig{ 44100.0, 1024 },
        HostConfig{ 44100.0, 2048 },
        HostConfig{ 44100.0, 8192 },
        HostConfig{ 48000.0, 32 },
        HostConfig{ 48000.0, 512 },
        HostConfig{ 48000.0, 2048 },
        HostConfig{ 88200.0, 512 },
        HostConfig{ 96000.0, 512 },
        HostConfig{ 96000.0, 32 }),
    [](const ::testing::TestParamInfo<HostConfig>& info) {
        return std::to_string(static_cast<int>(info.param.hostSR)) + "_" + std::to_string(info.param.hostBlock);
    });

TEST_P(ResamplingContractsFull, RoundTrip_OutputBlockEqualsHostBlock) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    skipIfInfeasible(cfg);
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, kPreRollBlocks);
    chain.hostBuffer.clear();
    juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
    juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
    EXPECT_EQ(downOut.getNumSamples(), cfg.hostBlock);
}
