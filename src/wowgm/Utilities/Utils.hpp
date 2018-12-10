#pragma once

#include <ctime>
#include <string>
#include <cstdint>
#include <type_traits>

struct tm* localtime_r(const time_t* time, struct tm *result);

namespace wowgm::utilities
{
    std::string ByteArrayToHexStr(uint8_t const* bytes, uint32_t arrayLen, bool reverse = false);

    // https://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
    // http://talesofcpp.fusionfenix.com/post-22/true-story-efficient-packing
    template <std::size_t I, typename T>
    struct indexed { using type = T; };

    template <typename Is, typename ...Ts>
    struct indexer;

    template <std::size_t ...Is, typename ...Ts>
    struct indexer<std::index_sequence<Is...>, Ts...> : indexed<Is, Ts>... { };

    template <std::size_t I, typename T>
    static indexed<I, T> select(indexed<I, T>);

    template <std::size_t I, typename ...Ts>
    using nth_element = typename decltype(select<I>(indexer<std::index_sequence_for<Ts...>, Ts...>{}))::type;

    template <typename E>
    using is_scoped_enum = std::integral_constant<bool, std::is_enum<E>::value && !std::is_convertible<E, int>::value>;

    template <typename E>
    inline typename std::enable_if<std::is_enum<E>::value, E>::type operator & (E enumeration, typename std::underlying_type<E>::type mask)
    {
        typedef typename std::underlying_type<E>::type enum_underlying_type;

        return static_cast<E>(static_cast<enum_underlying_type>(enumeration) & mask);
    }

    template <typename E>
    inline typename std::enable_if<std::is_enum<E>::value, E>::type operator | (E enumeration, typename std::underlying_type<E>::type mask)
    {
        typedef typename std::underlying_type<E>::type enum_underlying_type;

        return static_cast<E>(static_cast<enum_underlying_type>(enumeration) | mask);
    }

    template <typename E>
    inline typename std::enable_if<std::is_enum<E>::value, E>::type operator ^ (E enumeration, typename std::underlying_type<E>::type mask)
    {
        typedef typename std::underlying_type<E>::type enum_underlying_type;

        return static_cast<E>(static_cast<enum_underlying_type>(enumeration) ^ mask);
    }
}
