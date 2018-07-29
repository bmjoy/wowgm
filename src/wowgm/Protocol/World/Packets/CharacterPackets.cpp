#include "CharacterPackets.hpp"

namespace wowgm::protocol::world::packets
{
    ClientEnumCharactersResult::ClientEnumCharactersResult(WorldPacket&& packet) : ServerPacket(std::move(packet))
    {

    }

    void ClientEnumCharactersResult::Read()
    {
        std::uint32_t unkCounter = _worldPacket.ReadBits(23);
        bool unkBit = _worldPacket.ReadBit();

        Characters.resize(_worldPacket.ReadBits(17));
        std::vector<uint8_t> nameLengths(Characters.size());

        std::uint8_t i = 0;
        for (CharacterInfo& charInfo : Characters)
        {
            charInfo.GUID[3] = _worldPacket.ReadBit();
            charInfo.GuildGUID[1] = _worldPacket.ReadBit();
            charInfo.GuildGUID[7] = _worldPacket.ReadBit();
            charInfo.GuildGUID[2] = _worldPacket.ReadBit();
            nameLengths[i] = _worldPacket.ReadBits(7);
            charInfo.GUID[4] = _worldPacket.ReadBit();
            charInfo.GUID[7] = _worldPacket.ReadBit();
            charInfo.GuildGUID[3] = _worldPacket.ReadBit();
            charInfo.GUID[5] = _worldPacket.ReadBit();
            charInfo.GuildGUID[6] = _worldPacket.ReadBit();
            charInfo.GUID[1] = _worldPacket.ReadBit();
            charInfo.GuildGUID[5] = _worldPacket.ReadBit();
            charInfo.GuildGUID[4] = _worldPacket.ReadBit();
            bool firstLogin = _worldPacket.ReadBit();
            charInfo.GUID[0] = _worldPacket.ReadBit();
            charInfo.GUID[2] = _worldPacket.ReadBit();
            charInfo.GUID[6] = _worldPacket.ReadBit();
            charInfo.GuildGUID[0] = _worldPacket.ReadBit();

            ++i;
        }

        i = 0;
        for (CharacterInfo& charInfo : Characters)
        {
            _worldPacket >> charInfo.Class;

            _worldPacket.read_skip(19 * 9 + 4 * 9); // bag and inventory

            _worldPacket >> charInfo.Pet.Family;
            _worldPacket.ReadByteSeq(charInfo.GuildGUID[2]);
            _worldPacket >> charInfo.Order;
            _worldPacket >> charInfo.HairStyleID;
            _worldPacket.ReadByteSeq(charInfo.GuildGUID[3]);
            _worldPacket >> charInfo.Pet.DisplayID;
            _worldPacket >> charInfo.Flags;
            _worldPacket >> charInfo.HairColorID;
            _worldPacket.ReadByteSeq(charInfo.GUID[4]);
            _worldPacket >> charInfo.MapID;
            _worldPacket.ReadByteSeq(charInfo.GuildGUID[5]);
            _worldPacket >> charInfo.Z;
            _worldPacket.ReadByteSeq(charInfo.GuildGUID[6]);
            _worldPacket >> charInfo.Pet.Level;
            _worldPacket.ReadByteSeq(charInfo.GUID[3]);
            _worldPacket >> charInfo.Y;
            _worldPacket >> charInfo.CustomizationFlags;
            _worldPacket >> charInfo.FacialHairID;
            _worldPacket.ReadByteSeq(charInfo.GUID[7]);
            _worldPacket >> charInfo.Gender;
            charInfo.Name = _worldPacket.ReadString(nameLengths[i]);
            _worldPacket >> charInfo.FaceID;
            _worldPacket.ReadByteSeq(charInfo.GUID[0]);
            _worldPacket.ReadByteSeq(charInfo.GUID[2]);
            _worldPacket.ReadByteSeq(charInfo.GuildGUID[1]);
            _worldPacket.ReadByteSeq(charInfo.GuildGUID[7]);
            _worldPacket >> charInfo.X;
            _worldPacket >> charInfo.SkinID;
            _worldPacket >> charInfo.Race;
            _worldPacket >> charInfo.Level;
            _worldPacket.ReadByteSeq(charInfo.GUID[6]);
            _worldPacket.ReadByteSeq(charInfo.GuildGUID[4]);
            _worldPacket.ReadByteSeq(charInfo.GuildGUID[0]);
            _worldPacket.ReadByteSeq(charInfo.GUID[5]);
            _worldPacket.ReadByteSeq(charInfo.GUID[1]);
            _worldPacket >> charInfo.ZoneID;

            ++i;
        }
    }

    bool CharacterInfo::HasPet() const
    {
        return Pet.Level != 0;
    }

    bool CharacterInfo::IsInGuild() const
    {
        return !GuildGUID.IsEmpty();
    }
}
