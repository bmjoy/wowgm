#pragma once

#include "PacketCrypt.hpp"

namespace wowgm::protocol::world
{
    class WorldPacketCrypt : public PacketCrypt
    {
        public:
            WorldPacketCrypt();

            void Init(BigNumber* K) override;
    };
}
