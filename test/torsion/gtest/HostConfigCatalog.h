#pragma once

/// @file HostConfigCatalog.h
/// @brief Composable host SR/block matrices and gtest parameter adapters.
///
/// @namespace torsion::test

#include <algorithm>
#include <functional>
#include <gtest/gtest.h>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>

#include "HostConfig.h"

namespace torsion::test
{

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

    /// Retains configs for which @p predicate returns true.
    inline std::vector<HostConfig> filterHostConfigs(
        std::vector<HostConfig> configs,
        const std::function<bool(const HostConfig &)> &predicate)
    {
        configs.erase(std::remove_if(configs.begin(), configs.end(),
                                     [&](const HostConfig &cfg)
                                     { return !predicate(cfg); }),
                      configs.end());
        return configs;
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

} // namespace torsion::test
