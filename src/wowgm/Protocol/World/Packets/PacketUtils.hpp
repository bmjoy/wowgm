#pragma once

#include "Utils.hpp"
#include "WorldPacket.hpp"

#include <type_traits>

namespace wowgm::protocol::world::packets
{
    using namespace wowgm::utilities;

    template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
    inline WorldPacket& operator >> (WorldPacket& packet, T& value)
    {
        typedef typename std::underlying_type<T>::type type;

        value = packet.read<T>();
        return packet;
    }
}