#pragma once

#include "Packet.hpp"

#include <cstdint>
#include <array>
#include <openssl/sha.h>  // for SHA_DIGEST_LENGTH
#include "stdint.h"       // for uint32_t, uint8_t, uint16_t
#include "xstring"        // for string

namespace wowgm::protocol::world::packets
{
    struct ClientConnectionAuthChallenge final : public ServerPacket
    {
        ClientConnectionAuthChallenge(WorldPacket&& packet);

        void Read() override;

        std::array<std::uint8_t, 16> Seeds[2] = { };
        std::uint32_t AuthSeed;
    };

    struct UserRouterClientAuthSession final : public ClientPacket
    {
        UserRouterClientAuthSession();

        WorldPacket const* Write() override;

        std::uint32_t ServerID = 0;
        std::uint32_t BattlegroupID = 0;
        std::uint8_t LoginServerType = 0;
        std::uint32_t RealmID = 0;
        std::uint16_t Build = 0;
        std::array<std::uint8_t, SHA_DIGEST_LENGTH> Digest = { };
        std::uint32_t RegionID;
        std::string AccountName;
        std::uint32_t ClientSeed;
        bool UseIPv6 = false;
    };
}