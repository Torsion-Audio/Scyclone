#pragma once

#include <string>
#include <vector>
#include "HostConfigCatalog.h"
#include "TestInfrastructure.h"

namespace resampling_test
{

    enum class ChainKind
    {
        RoundTrip,
        Production
    };

    enum class ProcessorDirection
    {
        Up,
        Down
    };

    struct ChainContractCase
    {
        HostConfig cfg;
        ChainKind chain;
    };

    struct ProcessorStructuralCase
    {
        HostConfig cfg;
        ProcessorDirection direction;
    };

    class DryWetHostConfigTest : public JuceAudioTest, public ::testing::WithParamInterface<HostConfig>
    {
    };

    class ChainContractTest : public JuceAudioTest, public ::testing::WithParamInterface<ChainContractCase>
    {
    };

    class RoundTripCiTest : public JuceAudioTest, public ::testing::WithParamInterface<HostConfig>
    {
    };

    class ExtendedHostMatrixTest : public JuceAudioTest, public ::testing::WithParamInterface<HostConfig>
    {
    };

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
