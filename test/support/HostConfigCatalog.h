#pragma once

/// @file HostConfigCatalog.h
/// @brief Host sample-rate / block-size axes, composable matrices, and CI presets.
///
/// Builds gtest parameter lists from rate/block axes. All presets filter through
/// `isFeasibleHostConfig` (ONNX min-block constraint). See test/README.md.
///
/// @namespace resampling_test

#include <algorithm>
#include <array>
#include <gtest/gtest.h>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>
#include "TestInfrastructure.h"

namespace resampling_test
{

    inline constexpr std::array kDefaultCiSampleRates = {44100.0, 48000.0};
    inline constexpr std::array kDefaultCiBlockSizes = {128, 512};

    inline constexpr std::array kAlignmentEdgeBlocks44100 = {32, 64, 2048, 8192};
    inline constexpr std::array kAlignmentEdgeBlocks48000 = {2048};

    /// Cartesian product of rates × blocks.
    inline std::vector<HostConfig> cartesianHostConfigs(std::span<const double> rates,
                                                        std::span<const int> blocks)
    {
        std::vector<HostConfig> configs;
        configs.reserve(rates.size() * blocks.size());
        for (const double rate : rates)
        {
            for (const int block : blocks)
            {
                configs.push_back({rate, block});
            }
        }
        return configs;
    }

    inline std::vector<HostConfig> mergeHostConfigs(std::span<const HostConfig> a,
                                                    std::span<const HostConfig> b)
    {
        std::vector<HostConfig> merged;
        merged.reserve(a.size() + b.size());
        merged.insert(merged.end(), a.begin(), a.end());
        merged.insert(merged.end(), b.begin(), b.end());
        return merged;
    }

    inline std::vector<HostConfig> dedupeHostConfigs(std::vector<HostConfig> configs)
    {
        struct KeyHash
        {
            std::size_t operator()(const HostConfig &cfg) const
            {
                return std::hash<int>{}(static_cast<int>(cfg.hostSR)) ^
                       (std::hash<int>{}(cfg.hostBlock) << 1);
            }
        };

        struct KeyEqual
        {
            bool operator()(const HostConfig &a, const HostConfig &b) const
            {
                return a.hostSR == b.hostSR && a.hostBlock == b.hostBlock;
            }
        };

        std::vector<HostConfig> unique;
        std::unordered_set<HostConfig, KeyHash, KeyEqual> seen;
        unique.reserve(configs.size());
        for (const auto &cfg : configs)
        {
            if (seen.insert(cfg).second)
            {
                unique.push_back(cfg);
            }
        }
        return unique;
    }

    /// Drops configs where up-path ONNX block < kMinOnnxBlock.
    inline std::vector<HostConfig> filterFeasibleHostConfigs(std::vector<HostConfig> configs)
    {
        configs.erase(std::remove_if(configs.begin(), configs.end(),
                                     [](const HostConfig &cfg)
                                     { return !isFeasibleHostConfig(cfg.hostSR, cfg.hostBlock); }),
                      configs.end());
        return configs;
    }

    /** PR / sanitizer matrix: 44.1/48 kHz × 128/512 blocks. */
    inline std::vector<HostConfig> defaultCiHostConfigs()
    {
        return filterFeasibleHostConfigs(
            cartesianHostConfigs(kDefaultCiSampleRates, kDefaultCiBlockSizes));
    }

    /** Extra block sizes for alignment-sensitive tests (dirac at block center). */
    inline std::vector<HostConfig> alignmentEdgeHostConfigs()
    {
        std::vector<HostConfig> configs;
        configs.reserve(kAlignmentEdgeBlocks44100.size() + kAlignmentEdgeBlocks48000.size());
        for (const int block : kAlignmentEdgeBlocks44100)
        {
            configs.push_back({44100.0, block});
        }
        for (const int block : kAlignmentEdgeBlocks48000)
        {
            configs.push_back({48000.0, block});
        }
        return filterFeasibleHostConfigs(std::move(configs));
    }

    /** Default CI plus alignment edge blocks — dry/wet dirac is block-size sensitive. */
    inline std::vector<HostConfig> dryWetHostConfigs()
    {
        const auto core = defaultCiHostConfigs();
        const auto edge = alignmentEdgeHostConfigs();
        return dedupeHostConfigs(mergeHostConfigs(core, edge));
    }

    /** Curated extended matrix (release / manual CI). */
    inline std::vector<HostConfig> extendedHostMatrixConfigs()
    {
        return filterFeasibleHostConfigs({
            {44100.0, 32},
            {44100.0, 64},
            {44100.0, 128},
            {44100.0, 256},
            {44100.0, 512},
            {44100.0, 1024},
            {44100.0, 2048},
            {44100.0, 8192},
            {48000.0, 32},
            {48000.0, 512},
            {48000.0, 2048},
            {88200.0, 512},
            {96000.0, 512},
            {96000.0, 32},
        });
    }

    inline auto hostConfigValues(const std::vector<HostConfig> &configs)
    {
        return ::testing::ValuesIn(configs);
    }

    /// gtest param name: "{hostSR}_{hostBlock}".
    inline std::string hostConfigName(const ::testing::TestParamInfo<HostConfig> &info)
    {
        return std::to_string(static_cast<int>(info.param.hostSR)) + "_" +
               std::to_string(info.param.hostBlock);
    }

} // namespace resampling_test
