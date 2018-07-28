#include "WorldPacket.hpp"
#include "AuthentificationPackets.hpp"
#include "Opcodes.hpp"      // for Opcode, Opcode::CMSG_AUTH_SESSION

#include <type_traits>      // for move
#include <boost/utility/in_place_factory.hpp>

namespace wowgm::protocol::world::packets
{
    template <typename T>
    inline WorldPacket& operator >> (WorldPacket& worldPacket, boost::optional<T>& optional)
    {
        optional = boost::in_place();
        worldPacket >> *optional;

        return worldPacket;
    }

    inline WorldPacket& operator >> (WorldPacket& worldPacket, ResponseCodes& responseCode)
    {
        responseCode = worldPacket.read<ResponseCodes>();
        return worldPacket;
    }

    inline WorldPacket& operator >> (WorldPacket& worldPacket, AccountInfo& accInfo)
    {
        worldPacket >> accInfo.BillingTimeRemaining;
        worldPacket >> accInfo.PlayerExpansion;
        worldPacket >> accInfo.UnkAccountInfo;
        worldPacket >> accInfo.AccountExpansion;
        worldPacket >> accInfo.BillingTimeRested;
        worldPacket >> accInfo.BillingFlags;

        return worldPacket;
    }

    ClientConnectionAuthChallenge::ClientConnectionAuthChallenge(WorldPacket&& worldPacket) : ServerPacket(std::move(worldPacket))
    {

    }

    void ClientConnectionAuthChallenge::Read()
    {
        for (std::uint32_t i = 0; i < 2; ++i)
        {
            for (std::uint32_t j = 0; j < Seeds[i].size(); ++j)
                _worldPacket >> Seeds[i][j];
        }

        _worldPacket >> AuthSeed;
        _worldPacket.read_skip<std::uint8_t>();
    }

    ClientConnectionAuthResponse::ClientConnectionAuthResponse(WorldPacket&& worldPacket) : ServerPacket(std::move(worldPacket))
    {

    }

    void ClientConnectionAuthResponse::Read()
    {
        bool hasQueueInfo = _worldPacket.ReadBit();

        if (hasQueueInfo)
        {
            QueueInfo = boost::in_place();
            QueueInfo->Bit = _worldPacket.ReadBit();
        }

        bool hasAccountInfo = _worldPacket.ReadBit();

        if (hasAccountInfo)
            _worldPacket >> AccountInfo;

        _worldPacket >> AuthResult;

        if (hasQueueInfo)
            _worldPacket >> QueueInfo->Position;
    }

    UserRouterClientAuthSession::UserRouterClientAuthSession() : ClientPacket(Opcode::CMSG_AUTH_SESSION)
    {

    }

    WorldPacket const* UserRouterClientAuthSession::Write()
    {
        _worldPacket << std::uint32_t(ServerID);
        _worldPacket << std::uint32_t(BattlegroupID);
        _worldPacket << std::uint8_t(LoginServerType);
        _worldPacket << std::uint8_t(Digest[10]);
        _worldPacket << std::uint8_t(Digest[18]);
        _worldPacket << std::uint8_t(Digest[12]);
        _worldPacket << std::uint8_t(Digest[5]);
        _worldPacket << std::uint64_t(0); // Unk
        _worldPacket << std::uint8_t(Digest[15]);
        _worldPacket << std::uint8_t(Digest[9]);
        _worldPacket << std::uint8_t(Digest[19]);
        _worldPacket << std::uint8_t(Digest[4]);
        _worldPacket << std::uint8_t(Digest[7]);
        _worldPacket << std::uint8_t(Digest[16]);
        _worldPacket << std::uint8_t(Digest[3]);
        _worldPacket << std::uint16_t(Build);
        _worldPacket << std::uint8_t(Digest[8]);
        _worldPacket << std::uint32_t(RealmID);
        _worldPacket << std::uint8_t(0);
        _worldPacket << std::uint8_t(Digest[17]);
        _worldPacket << std::uint8_t(Digest[6]);
        _worldPacket << std::uint8_t(Digest[0]);
        _worldPacket << std::uint8_t(Digest[1]);
        _worldPacket << std::uint8_t(Digest[11]);
        _worldPacket << std::uint32_t(ClientSeed); // LocalChallenge
        _worldPacket << std::uint8_t(Digest[2]);
        _worldPacket << std::uint32_t(RegionID);
        _worldPacket << std::uint8_t(Digest[14]);
        _worldPacket << std::uint8_t(Digest[13]);

        _worldPacket << std::uint32_t(0); // Addon size
        // Addon block

        _worldPacket.WriteBit(UseIPv6);
        _worldPacket.WriteBits(AccountName.length(), 12);
        _worldPacket.FlushBits();

        _worldPacket.WriteString(AccountName);

        return &_worldPacket;
    }
}
