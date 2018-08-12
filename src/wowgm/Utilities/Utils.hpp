#pragma once

#include <ctime>
#include <string>
#include <cstdint>
#include <type_traits>

namespace wowgm::utilities
{
    struct tm* localtime_r(const time_t* time, struct tm *result);

    std::string ByteArrayToHexStr(std::uint8_t const* bytes, std::uint32_t arrayLen, bool reverse = false);

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
}