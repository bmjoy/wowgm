#pragma once

#include <cstdint>
#include <cmath>

namespace wowgm::game::structures
{
    struct C3Vector
    {
        C3Vector() { }

        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;

        inline C3Vector& operator = (uint32_t packedValue)
        {
            X = ((packedValue & 0x7FF) << 21 >> 21) * 0.25f;
            Y = ((((packedValue >> 11) & 0x7FF) << 21) >> 21) * 0.25f;
            Z = ((packedValue >> 22 << 22) >> 22) * 0.25f;
            return *this;
        }

        inline float& operator [] (size_t index)
        {
            return reinterpret_cast<float*>(&X)[index];
        }

        inline float operator [] (size_t index) const
        {
            return reinterpret_cast<float const*>(&X)[index];
        }

        float length() const
        {
            return std::sqrt(X * X + Y * Y + Z * Z);
        }

        float lengthSquared() const
        {
            return X * X + Y * Y + Z * Z;
        }

        float distance(C3Vector const& other) const
        {
            return std::abs(distanceSquared(other));
        }

        float distanceSquared(C3Vector const& other) const
        {
            float distance = 0.0f;
            for (uint32_t i = 0; i < 3; ++i)
                distance += (other[i] - (*this)[i]) * (other[i] - (*this)[i]);
            return distance;
        }

        void normalize()
        {
            auto length = std::sqrt(X * X + Y * Y + Z * Z);
            X /= length;
            Y /= length;
            Z /= length;
        }
    };
}