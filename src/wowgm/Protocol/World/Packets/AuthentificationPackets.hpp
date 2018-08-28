#pragma once

#include "Packet.hpp"

#include <boost/optional.hpp>
#include <cstdint>
#include <array>
#include <openssl/sha.h>  // for SHA_DIGEST_LENGTH
#include <cstdint>        // for std::uint32_t, std::uint8_t, std::uint16_t
#include <string>         // for string

#include "ResponseCodes.hpp"

namespace wowgm::protocol::world::packets
{
    struct ClientConnectionAuthChallenge final : public ServerPacket
    {
        ClientConnectionAuthChallenge(WorldPacket&& packet);

        void Read() override;

        std::uint32_t AuthSeed;
        std::array<std::uint32_t, 8> Seeds = {};
        std::uint8_t UnkByte;
    };

    struct AccountInfo
    {
        std::uint32_t BillingTimeRemaining;
        std::uint32_t UnkAccountInfo;
        std::uint32_t BillingTimeRested;
        std::uint8_t PlayerExpansion;
        std::uint8_t AccountExpansion;
        std::uint8_t BillingFlags;
    };

    struct QueueInfo
    {
        std::uint32_t Position;
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

        std::uint32_t ServerID = 0;
        std::uint32_t BattlegroupID = 0;
        std::uint8_t LoginServerType = 0;
        std::uint32_t RealmID = 0;
        std::uint16_t Build = 0;
        std::array<std::uint8_t, SHA_DIGEST_LENGTH> Digest = { };
        std::uint32_t RegionID;
        std::string AccountName;
        std::uint32_t ClientSeed;
        std::uint64_t DosResponse;
        bool UseIPv6 = false;
    };
}