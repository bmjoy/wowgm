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
        for (std::uint32_t i = 0; i < 8; ++i)
            _worldPacket >> Seeds[i];

        _worldPacket >> AuthSeed;
        _worldPacket >> UnkByte;
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
        _worldPacket << std::uint64_t(DosResponse);
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
        _worldPacket << std::uint8_t(1); // Hardcoded
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

        static std::uint8_t AddonData[] = {
            36, 4, 0, 0, 120, 156, 117, 210, 75, 110, 131, 64, 12, 0, 80, 122, 143, 238, 122, 146, 124, 26,
            148, 54, 72, 81, 32, 217, 86, 102, 198, 1, 139, 193, 70, 102, 160, 77, 212, 227, 244, 160, 37,
            187, 86, 242, 172, 159, 199, 227, 223, 75, 150, 101, 235, 64, 247, 59, 168, 255, 88, 185, 150,
            112, 198, 30, 57, 158, 247, 217, 83, 255, 249, 124, 200, 254, 185, 186, 22, 80, 130, 52, 183,
            132, 35, 131, 45, 147, 139, 36, 108, 218, 26, 180, 70, 29, 91, 25, 18, 28, 99, 192, 43, 97, 240,
            5, 49, 245, 48, 88, 65, 196, 158, 184, 49, 19, 108, 32, 32, 123, 80, 139, 2, 45, 189, 150, 48,
            163, 191, 128, 18, 212, 1, 71, 43, 76, 250, 26, 226, 65, 154, 164, 85, 248, 21, 109, 28, 192,
            197, 19, 144, 223, 41, 244, 118, 242, 243, 238, 168, 114, 37, 251, 235, 45, 214, 83, 83, 137,
            4, 11, 95, 217, 201, 196, 17, 245, 77, 38, 101, 8, 70, 72, 30, 110, 67, 107, 142, 37, 47, 54,
            45, 216, 123, 206, 139, 114, 210, 25, 237, 37, 231, 19, 5, 191, 6, 238, 210, 186, 17, 142, 42,
            33, 29, 96, 202, 158, 199, 1, 157, 93, 209, 62, 98, 191, 10, 75, 167, 144, 60, 163, 71, 72, 41,
            174, 195, 152, 58, 132, 131, 72, 183, 216, 78, 52, 93, 68, 1, 78, 197, 22, 153, 241, 8, 222, 144,
            199, 122, 205, 39, 39, 188, 138, 54, 169, 114, 170, 199, 93, 218, 237, 86, 212, 99, 1, 12, 13, 90,
            87, 91, 73, 135, 246, 12, 42, 5, 143, 101, 71, 193, 158, 253, 194, 196, 168, 166, 93, 132, 124, 25,
            69, 151, 63, 255, 250, 247, 207, 246, 253, 23, 190, 137, 67, 50
        };

        _worldPacket << std::uint32_t(sizeof(AddonData)); // Addon size
        _worldPacket.append(AddonData, sizeof(AddonData));

        _worldPacket.WriteBit(UseIPv6);
        _worldPacket.WriteBits(AccountName.length(), 12);
        _worldPacket.FlushBits();

        _worldPacket.WriteString(AccountName);

        return &_worldPacket;
    }
}
