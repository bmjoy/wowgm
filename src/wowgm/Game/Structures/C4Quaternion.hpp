#pragma once

#include <cstdint>
#include <cmath>

namespace wowgm::game::structures
{
    struct C4Quaternion
    {
        C4Quaternion() { }

        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
        float W = 0.0f;

        inline C4Quaternion& operator = (std::uint64_t packedValue)
        {
            X = (packedValue >> 42) * (1.0f / 2097152.0f);
            Y = (((packedValue << 22) >> 32) >> 11) * (1.0f / 1048576.0f);
            Z = (packedValue << 43 >> 43) * (1.0f / 1048576.0f);

            W = X * X + Y * Y + Z * Z;
            if (std::abs(W - 1.0f) >= (1 / 1048576.0f))
                W = std::sqrt(1.0f - W);
            else
                W = 0.0f;

            return *this;
        }
    };
}