#pragma once

/// @file HostConfig.h
/// @brief Host sample-rate / block-size parameter for gtest matrices.
///
/// @namespace torsion::test

namespace torsion::test
{

    /// Parameter for gtest host-rate / block-size matrices.
    struct HostConfig
    {
        double hostSR;
        int hostBlock;
    };

} // namespace torsion::test
