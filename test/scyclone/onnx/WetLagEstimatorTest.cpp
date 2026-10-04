// Checks the wet-lag estimator on synthetic signals with a known delay, so the wet-alignment
// probe and tests built on it measure the model, not the estimator. No ONNX Runtime needed.

#include <gtest/gtest.h>
#include "WetLagMeasurement.h"

using namespace scyclone::test::wetlag;

namespace {

/// A transient-preserving "model": each burst convolved with a decaying noise response, delayed
/// by @p delay, at a random level, on top of a noise floor about 60 dB below the bursts.
std::vector<float> syntheticResponse(const Stimulus& stimulus, int delay, std::uint32_t seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> noise(-1.0f, 1.0f);
    std::uniform_real_distribution<float> level(0.3f, 1.0f);

    std::vector<float> response(512);
    for (size_t n = 0; n < response.size(); ++n)
        response[n] = noise(rng) * std::exp(-static_cast<float>(n) / 150.0f);

    std::vector<float> output(stimulus.samples.size());
    for (auto& sample : output)
        sample = 0.001f * noise(rng);

    constexpr int kBurstLength = 256;
    for (const int start : stimulus.burstStarts)
    {
        const float gain = level(rng);
        for (int n = 0; n < kBurstLength; ++n)
        {
            const float in = stimulus.samples[static_cast<size_t>(start + n)];
            for (size_t m = 0; m < response.size(); ++m)
            {
                const size_t index = static_cast<size_t>(start + n + delay) + m;
                if (index < output.size())
                    output[index] += gain * in * response[m];
            }
        }
    }
    return output;
}

} // namespace

TEST(WetLagEstimator, RecoversKnownDelay)
{
    const auto stimulus = makeBurstStimulus();

    for (const int delay : {2048, 3584, 5632})
    {
        const auto output = syntheticResponse(stimulus, delay, 99);

        LagSearch search;
        search.minLag = delay - 1024;
        search.maxLag = delay + 6144;
        const auto estimate = estimateLag(stimulus, output, search);

        EXPECT_EQ(estimate.responses, estimate.bursts) << "delay " << delay;
        EXPECT_NEAR(estimate.medianLag, delay, 16.0) << "delay " << delay;
        EXPECT_LE(estimate.maxPhaseDeviation, 16.0) << "delay " << delay;
        EXPECT_NEAR(estimate.crossCorrelationLag, delay, 32.0) << "delay " << delay;
        // Phases are spread evenly over the hop (128 ... 1920), so their median is 1024.
        EXPECT_NEAR(estimate.medianHopAlignedOnset, delay + 1024, 16.0) << "delay " << delay;
    }
}

// Negative control: output that has nothing to do with the bursts must not produce a confident
// lag, so a test asserting on the estimate cannot pass on garbage.
TEST(WetLagEstimator, UncorrelatedOutput_IsInconclusive)
{
    const auto stimulus = makeBurstStimulus();

    std::mt19937 rng(5);
    std::uniform_real_distribution<float> noise(-1.0f, 1.0f);
    std::vector<float> output(stimulus.samples.size());
    for (auto& sample : output)
        sample = 0.3f * noise(rng);

    LagSearch search;
    search.minLag = 2048;
    search.maxLag = 2048 + 6144;
    const auto estimate = estimateLag(stimulus, output, search);

    EXPECT_LT(estimate.responseRate, 0.75);
}
