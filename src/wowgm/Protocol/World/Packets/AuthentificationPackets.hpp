#pragma once

#include "Packet.hpp"

#include <boost/optional.hpp>
#include <cstdint>
#include <array>
#include <openssl/sha.h>  // for SHA_DIGEST_LENGTH
#include <cstdint>        // for uint32_t, uint8_t, uint16_t
#include <string>         // for string

#include "ResponseCodes.hpp"

namespace wowgm::protocol::world::packets
{
    struct ClientConnectionAuthChallenge final : public ServerPacket
    {
        ClientConnectionAuthChallenge(WorldPacket&& packet);

        void Read() override;

        uint32_t AuthSeed;
        std::array<uint32_t, 8> Seeds = {};
        uint8_t UnkByte;
    };

    struct AccountInfo
    {
        uint32_t BillingTimeRemaining;
        uint32_t UnkAccountInfo;
        uint32_t BillingTimeRested;
        uint8_t PlayerExpansion;
        uint8_t AccountExpansion;
        uint8_t BillingFlags;
    };

    struct QueueInfo
    {
        uint32_t Position;
        bool Bit;
    };

    struct ClientConnectionAuthResponse final : public ServerPacket
    {
        ClientConnectionAuthResponse(WorldPacket&& packet);

        void Read() override;

        boost::optional<QueueInfo> QueueInfo;
        boost::optional<AccountInfo> AccountInfo;
        ResponseCodes AuthResult;
    };

    struct UserRouterClientAuthSession final : public ClientPacket
    {
        UserRouterClientAuthSession();

        WorldPacket const* Write() override;

        uint32_t ServerID = 0;
        uint32_t BattlegroupID = 0;
        uint8_t LoginServerType = 0;
        uint32_t RealmID = 0;
        uint16_t Build = 0;
        std::array<uint8_t, SHA_DIGEST_LENGTH> Digest = { };
        uint32_t RegionID;
        std::string AccountName;
        uint32_t ClientSeed;
        uint64_t DosResponse;
        bool UseIPv6 = false;
    };
}