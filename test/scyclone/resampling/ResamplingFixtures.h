#pragma once

/// @file ResamplingFixtures.h
/// @brief gtest fixture classes and parameterized case builders for resampling tests.
///
/// Fixtures inherit `JuceAudioTest` + `WithParamInterface`. Case builders expand
/// host-config matrices into `INSTANTIATE_TEST_SUITE_P` value lists.
///
/// @namespace scyclone::test::resampling

#include <string>
#include <vector>

#include "HostConfigFixtures.h"
#include "JuceFixture.h"
#include "ScycloneHostPresets.h"
#include "ResamplingTopology.h"

namespace scyclone::test::resampling
{

    using torsion::test::HostConfig;
    using torsion::test::HostConfigParamTest;

    /// Up-only vs down-only structural test axis.
    enum class ProcessorDirection
    {
        Up,
        Down
    };

    /// Host config × processor direction for `ProcessorStructuralTest`.
    struct ProcessorStructuralCase
    {
        HostConfig cfg;
        ProcessorDirection direction;
    };

    /// Round-trip chain contracts (block size, swept-sine RMS, impulse latency).
    class RoundTripChainContractTest : public HostConfigParamTest
    {
    };

    /// Block-size contract only (all default CI host configs).
    class ProductionChainContractTest : public HostConfigParamTest
    {
    };

    /// Swept-sine RMS + impulse latency (48 kHz only — cross-rate group delay ≠ bulk latency).
    class Production48kSignalContractTest : public HostConfigParamTest
    {
    };

    /// Default CI round-trip tests (long-run, silence-out).
    class RoundTripCiTest : public HostConfigParamTest
    {
    };

    /// Extended host matrix (release / manual CI label).
    class ExtendedHostMatrixTest : public HostConfigParamTest
    {
    };

    /// Per-processor structural invariants (up + down).
    class ProcessorStructuralTest : public torsion::test::JuceAudioTest,
                                    public ::testing::WithParamInterface<ProcessorStructuralCase>
    {
    };

    inline std::string processorStructuralCaseName(const ::testing::TestParamInfo<ProcessorStructuralCase> &info)
    {
        const char *dir = (info.param.direction == ProcessorDirection::Up) ? "Up" : "Down";
        return std::string(dir) + "_" + std::to_string(static_cast<int>(info.param.cfg.hostSR)) + "_" + std::to_string(info.param.cfg.hostBlock);
    }

    inline std::vector<ProcessorStructuralCase> processorStructuralCasesFor(const std::vector<HostConfig> &configs)
    {
        std::vector<ProcessorStructuralCase> cases;
        cases.reserve(configs.size() * 2);
        for (const auto &cfg : configs)
        {
            cases.push_back({cfg, ProcessorDirection::Up});
            cases.push_back({cfg, ProcessorDirection::Down});
        }
        return cases;
    }

    inline auto processorStructuralValuesFor(const std::vector<HostConfig> &configs)
    {
        return ::testing::ValuesIn(processorStructuralCasesFor(configs));
    }

    inline void setupProcessorStructuralCase(const ProcessorStructuralCase &testCase,
                                             ResamplingProcessor &proc,
                                             juce::AudioBuffer<float> &buf,
                                             int &expectedIn,
                                             int &expectedOut)
    {
        if (testCase.direction == ProcessorDirection::Up)
        {
            prepareUpOnly(testCase.cfg.hostSR, testCase.cfg.hostBlock, proc);
            buf.setSize(1, testCase.cfg.hostBlock);
            expectedIn = testCase.cfg.hostBlock;
            expectedOut = proc.getOutputBufferSize();
            return;
        }

        const auto ratioCase = makeProductionRatioCase(testCase.cfg.hostSR, testCase.cfg.hostBlock);
        prepareDownOnly(testCase.cfg.hostSR, ratioCase.expectedUpOut, testCase.cfg.hostBlock, proc, true);
        buf.setSize(1, ratioCase.expectedUpOut);
        expectedIn = ratioCase.expectedUpOut;
        expectedOut = testCase.cfg.hostBlock;
    }

} // namespace scyclone::test::resampling
