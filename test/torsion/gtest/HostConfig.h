#pragma once

#include <ostream>

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

    inline void PrintTo(const HostConfig &c, std::ostream *os)
    {
        *os << static_cast<int>(c.hostSR) << "_" << c.hostBlock;
    }

} // namespace torsion::test
