#pragma once

// Core test fixtures and host-config math for Scyclone behavioral tests.
// Matrices and CI presets live in HostConfigCatalog.h.
// See test/README.md and docs/resampling_architecture.md.

#include <JuceHeader.h>
#include <cmath>
#include <gtest/gtest.h>
#include <vector>

namespace resampling_test
{

    constexpr double kOnnxRate = 48000.0;
    constexpr int kMinOnnxBlock = 32;
    constexpr int kPreRollBlocks = 4;
    constexpr int kSteadyBlocks = 8;
    constexpr int kLongRunBlocks = 100;

    struct HostConfig
    {
        double hostSR;
        int hostBlock;
    };

    inline int upOutputBlockSize(double hostSR, int hostBlock)
    {
        return static_cast<int>(std::ceil(kOnnxRate / hostSR * static_cast<double>(hostBlock)));
    }

    /** Production up-path sizing at a host rate/block (data-driven expected values). */
    struct ProductionRatioCase
    {
        double hostSR;
        int hostBlock;
        int expectedUpOut;
        double upSrcRatio;
    };

    inline ProductionRatioCase makeProductionRatioCase(double hostSR, int hostBlock)
    {
        const int expectedUpOut = upOutputBlockSize(hostSR, hostBlock);
        return {hostSR, hostBlock, expectedUpOut,
                static_cast<double>(expectedUpOut) / static_cast<double>(hostBlock)};
    }

    /** Uncoupled down output size (ceil footgun — not production mode). */
    inline int uncoupledDownOutputSize(double hostSR, int upBlock)
    {
        return static_cast<int>(std::ceil(hostSR / kOnnxRate * static_cast<double>(upBlock)));
    }

    inline bool isFeasibleHostConfig(double hostSR, int hostBlock)
    {
        return upOutputBlockSize(hostSR, hostBlock) >= kMinOnnxBlock;
    }

    inline void skipIfInfeasible(const HostConfig &cfg)
    {
        if (!isFeasibleHostConfig(cfg.hostSR, cfg.hostBlock))
        {
            GTEST_SKIP() << "up output < " << kMinOnnxBlock << " for hostSR=" << cfg.hostSR
                         << " block=" << cfg.hostBlock;
        }
    }

    /** Long-run sample-count tolerance: terminate = ceil(max(ratio, 1/ratio)); return 2 * terminate. */
    inline int longRunTolerance(double ratio)
    {
        const int terminate = static_cast<int>(std::ceil((ratio >= 1.0) ? ratio : 1.0 / ratio));
        return 2 * terminate;
    }

    class JuceAudioTest : public ::testing::Test
    {
    protected:
        juce::ScopedJuceInitialiser_GUI juceInit;
    };

} // namespace resampling_test
