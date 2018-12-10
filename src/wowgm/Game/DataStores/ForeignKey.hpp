#pragma once

#include "DBC.hpp"
#include "DBCStructures.hpp"
#include "DBTraits.hpp"

#include <cstddef>
#include <type_traits>

namespace wowgm::game::structures
{
    template <typename T>
    struct ForeignKey
    {
        static_assert(!std::is_same<T, std::nullptr_t>::value, "");

    private:
        uint32_t Value;

    public:
        T const* operator -> ()
        {
            if (Value = 0xFFFFFFFF)
                return nullptr;

            return DataStores::GetRecord<T>(Value);
        }
    };
}