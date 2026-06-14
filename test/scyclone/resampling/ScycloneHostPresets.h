#pragma once

/// @file ScycloneHostPresets.h
/// @brief Scyclone CI host SR/block presets filtered by resampler feasibility.
///
/// @namespace scyclone::test::resampling

#include <array>
#include <vector>

#include "HostConfigCatalog.h"
#include "ResamplingTopology.h"

namespace scyclone::test::resampling
{

    inline constexpr std::array kDefaultCiSampleRates = {44100.0, 48000.0};
    inline constexpr std::array kDefaultCiBlockSizes = {128, 512};

    inline constexpr std::array kAlignmentEdgeBlocks44100 = {32, 64, 2048, 8192};
    inline constexpr std::array kAlignmentEdgeBlocks48000 = {2048};

    /// Drops configs where up-path ONNX block < kMinOnnxBlock.
    inline std::vector<torsion::test::HostConfig> filterFeasibleHostConfigs(
        std::vector<torsion::test::HostConfig> configs)
    {
        return torsion::test::filterHostConfigs(
            std::move(configs),
            [](const torsion::test::HostConfig &cfg)
            { return isFeasibleHostConfig(cfg.hostSR, cfg.hostBlock); });
    }

    /** PR / sanitizer matrix: 44.1/48 kHz × 128/512 blocks. */
    inline std::vector<torsion::test::HostConfig> defaultCiHostConfigs()
    {
        return filterFeasibleHostConfigs(
            torsion::test::cartesianHostConfigs(kDefaultCiSampleRates, kDefaultCiBlockSizes));
    }

    /** Extra block sizes for alignment-sensitive tests (dirac at block center). */
    inline std::vector<torsion::test::HostConfig> alignmentEdgeHostConfigs()
    {
        std::vector<torsion::test::HostConfig> configs;
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
    inline std::vector<torsion::test::HostConfig> dryWetHostConfigs()
    {
        const auto core = defaultCiHostConfigs();
        const auto edge = alignmentEdgeHostConfigs();
        return torsion::test::dedupeHostConfigs(torsion::test::mergeHostConfigs(core, edge));
    }

    /** Curated extended matrix (release / manual CI). */
    inline std::vector<torsion::test::HostConfig> extendedHostMatrixConfigs()
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

} // namespace scyclone::test::resampling
