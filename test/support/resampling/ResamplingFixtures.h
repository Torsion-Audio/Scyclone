#pragma once

/// @file ResamplingFixtures.h
/// @brief gtest fixture classes and parameterized case builders for resampling tests.
///
/// Fixtures inherit `JuceAudioTest` + `WithParamInterface`. Case builders expand
/// host-config matrices into `INSTANTIATE_TEST_SUITE_P` value lists.
///
/// @namespace resampling_test

#include <string>
#include <vector>
#include "HostConfigCatalog.h"
#include "ResamplingTopology.h"
#include "TestInfrastructure.h"

namespace resampling_test
{

    /// Chain topology under test in parameterized chain contracts.
    enum class ChainKind
    {
        RoundTrip,
        Production
    };

    /// Up-only vs down-only structural test axis.
    enum class ProcessorDirection
    {
        Up,
        Down
    };

    /// Host config × chain kind for `ChainContractTest`.
    struct ChainContractCase
    {
        HostConfig cfg;
        ChainKind chain;
    };

    /// Host config × processor direction for `ProcessorStructuralTest`.
    struct ProcessorStructuralCase
    {
        HostConfig cfg;
        ProcessorDirection direction;
    };

    /// Parameterized dry/wet dirac alignment over host configs.
    class DryWetHostConfigTest : public JuceAudioTest, public ::testing::WithParamInterface<HostConfig>
    {
    };

    /// Round-trip and production chain contracts.
    class ChainContractTest : public JuceAudioTest, public ::testing::WithParamInterface<ChainContractCase>
    {
    };

    /// Default CI round-trip tests (long-run, silence-out).
    class RoundTripCiTest : public JuceAudioTest, public ::testing::WithParamInterface<HostConfig>
    {
    };

    /// Extended host matrix (release / manual CI label).
    class ExtendedHostMatrixTest : public JuceAudioTest, public ::testing::WithParamInterface<HostConfig>
    {
    };

    /// Per-processor structural invariants (up + down).
    class ProcessorStructuralTest : public JuceAudioTest,
                                    public ::testing::WithParamInterface<ProcessorStructuralCase>
    {
    };

    inline std::string chainKindTag(ChainKind kind)
    {
        return (kind == ChainKind::RoundTrip) ? "RoundTrip" : "Production";
    }

    inline std::string chainContractCaseName(const ::testing::TestParamInfo<ChainContractCase> &info)
    {
        return chainKindTag(info.param.chain) + "_" + std::to_string(static_cast<int>(info.param.cfg.hostSR)) + "_" + std::to_string(info.param.cfg.hostBlock);
    }

    inline std::string processorStructuralCaseName(const ::testing::TestParamInfo<ProcessorStructuralCase> &info)
    {
        const char *dir = (info.param.direction == ProcessorDirection::Up) ? "Up" : "Down";
        return std::string(dir) + "_" + std::to_string(static_cast<int>(info.param.cfg.hostSR)) + "_" + std::to_string(info.param.cfg.hostBlock);
    }

    inline std::vector<ChainContractCase> chainContractCasesFor(const std::vector<HostConfig> &configs)
    {
        std::vector<ChainContractCase> cases;
        cases.reserve(configs.size() * 2);
        for (const auto &cfg : configs)
        {
            cases.push_back({cfg, ChainKind::RoundTrip});
            cases.push_back({cfg, ChainKind::Production});
        }
        return cases;
    }

    inline auto chainContractValuesFor(const std::vector<HostConfig> &configs)
    {
        return ::testing::ValuesIn(chainContractCasesFor(configs));
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

    /// Prepares processor + buffer for structural tests; sets expected I/O frame counts.
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

} // namespace resampling_test
