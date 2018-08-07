#include "WorldStateMgr.hpp"

#include <type_traits>

namespace wowgm::game::entities
{
    template <typename T, size_t N = 1>
    struct Descriptor
    {
        static_assert(N >= 1 && N <= 1024, "Descriptor with N out of bounds");

    private:
        T Value[N];

        std::enable_if<N != 1, T>::type& operator [] (int index)
        {
            return Value[index];
        }

        std::enable_if<N != 1, T>::type const& operator[] (int index) const
        {
            return Value[index];
        }

    public:
        std::enable_if<N == 1, T>::type const& GetValue() const { return Value[0]; }
        std::enable_if<N != 1, T>::type* GetValue() { return Value; }
    };

}
