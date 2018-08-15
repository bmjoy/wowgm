#pragma once

namespace wowgm::game::structures
{
    struct C3Vector
    {
        C3Vector() { }

        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;

        inline C3Vector& operator = (std::uint32_t packedValue)
        {
            X = ((packedValue & 0x7FF) << 21 >> 21) * 0.25f;
            Y = ((((packedValue >> 11) & 0x7FF) << 21) >> 21) * 0.25f;
            Z = ((packedValue >> 22 << 22) >> 22) * 0.25f;
            return *this;
        }
    };
}