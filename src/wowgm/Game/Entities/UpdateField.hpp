#pragma once

#include <type_traits>
#include <cstdint>
#include <array>

namespace wowgm::game::entities
{
    template <typename T, size_t N = 1>
    class Descriptor
    {
    private:
        using member_type = typename std::conditional<N == 1, T, std::array<T, N>>::type;
        using element_type = T;

        constexpr static const size_t arity = N;

        static_assert(N >= 1 && N <= 1024, "Descriptor with N out of bounds");

        member_type _value;
    public:

        Descriptor() : _value() { }

        inline T& operator [] (size_t index)
        {
            static_assert(N != 1);
            return _value[index];
        }

        inline const T& operator[] (size_t index) const
        {
            static_assert(N != 1);
            return _value[index];
        }

        inline operator member_type() const
        {
            return _value;
        }

        Descriptor(Descriptor<T, N>&& other) = delete;
        Descriptor(Descriptor<T, N> const& other) = delete;

        Descriptor<T, N>& operator = (Descriptor<T, N> const&) = delete;
        Descriptor<T, N>& operator = (Descriptor<T, N>&&) = delete;

        Descriptor<T, N>& operator = (member_type const& other) = delete;
        Descriptor<T, N>& operator = (member_type&& other) = delete;
    };

    static_assert(sizeof(std::array<float, 2>) == 8);
    static_assert(sizeof(std::array<float, 2>) == sizeof(Descriptor<float, 2>));

    static_assert(sizeof(Descriptor<float>) == sizeof(float), "Descriptor<T> must be of same size than T.");
    static_assert(sizeof(Descriptor<float, 2>) == sizeof(float) * 2, "Descriptor<T, N> must be of same size than T, N.");
}
