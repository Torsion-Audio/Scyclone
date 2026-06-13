// Signal-level resampling checks — invariant 6

#include <gtest/gtest.h>
#include <random>
#include "ResamplingTestHelpers.h"

using namespace resampling_test;

namespace {

struct HostConfig {
    double hostSR;
    int hostBlock;
};

class ResamplingSignalFast : public ::testing::TestWithParam<HostConfig> {};

} // namespace

TEST_P(ResamplingSignalFast, SilenceIn_SilenceOut_AfterPreRoll) {
    juce::ScopedJuceInitialiser_GUI init;
    const auto& cfg = GetParam();
    if (!isFeasibleHostConfig(cfg.hostSR, cfg.hostBlock)) {
        GTEST_SKIP();
    }
    auto chain = prepareRoundTripChain(cfg.hostSR, cfg.hostBlock);
    runSilencePreRoll(chain, 4);
    chain.hostBuffer.clear();
    for (int n = 0; n < 8; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        for (int i = 0; i < downOut.getNumSamples(); ++i) {
            EXPECT_LT(std::abs(downOut.getSample(0, i)), 1.0e-5f);
        }
    }
}

TEST(ResamplingSignal, Passthrough_RoundTrip_NoNaNs) {
    juce::ScopedJuceInitialiser_GUI init;
    auto chain = prepareRoundTripChain(44100.0, 512);
    PassthroughProcessor passthrough;
    runSilencePreRoll(chain, 4);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

    for (int n = 0; n < 100; ++n) {
        for (int i = 0; i < chain.hostBuffer.getNumSamples(); ++i) {
            chain.hostBuffer.setSample(0, i, dist(rng));
        }
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        passthrough.processBlock(upOut);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        for (int i = 0; i < downOut.getNumSamples(); ++i) {
            const float s = downOut.getSample(0, i);
            EXPECT_FALSE(std::isnan(s));
            EXPECT_FALSE(std::isinf(s));
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    TierA,
    ResamplingSignalFast,
    ::testing::Values(
        HostConfig{ 44100.0, 128 },
        HostConfig{ 44100.0, 512 },
        HostConfig{ 48000.0, 128 },
        HostConfig{ 48000.0, 512 }),
    [](const ::testing::TestParamInfo<HostConfig>& info) {
        return std::to_string(static_cast<int>(info.param.hostSR)) + "_" + std::to_string(info.param.hostBlock);
    });
