#include "WorldStateMgr.hpp"

#include <type_traits>

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

        Descriptor() : _value()
        {

        }

        Descriptor(Descriptor<T, N>&& other) = delete;
        Descriptor(Descriptor<T, N> const& other) = delete;

        inline Descriptor<T, N>& operator = (member_type const& other)
        {
            _value = other;
            return *this;
        }

        inline Descriptor<T, N>& operator = (member_type&& other)
        {
            _value = std::move(other);
            return *this;
        }
    };

}
