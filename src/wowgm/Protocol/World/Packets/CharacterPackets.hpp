#pragma once

#include "Packet.hpp"
#include "ObjectGuid.hpp"

#include <string>
#include <vector>
#include <boost/optional.hpp>

namespace wowgm::protocol::world::packets
{
    using namespace wowgm::game::structures;

    struct PetInfo
    {
        uint32_t DisplayID;
        uint32_t Family;
        uint32_t Level;
    };

    struct CharacterInfo
    {
        ObjectGuid GUID;
        ObjectGuid GuildGUID;
        std::string Name;

        PetInfo Pet;

        uint8_t Order;
        int32_t MapID;
        float X, Y, Z; // C3Vector

        uint8_t Gender;

        uint8_t Race;
        uint8_t Class;
        uint32_t ZoneID;

        uint32_t Flags;
        uint32_t CustomizationFlags;

        uint8_t SkinID;
        uint8_t FacialHairID;
        uint8_t HairColorID;
        uint8_t HairStyleID;
        uint8_t FaceID;

        uint8_t Level;

        bool HasPet() const;
        bool IsInGuild() const;
    };

    struct ClientEnumCharactersResult : public ServerPacket
    {
        ClientEnumCharactersResult(WorldPacket&& packet);

        void Read() override;

        std::vector<CharacterInfo> Characters;
    };

    struct UserClientPlayerLogin : public ClientPacket
    {
        UserClientPlayerLogin();

        WorldPacket const* Write() override;

        ObjectGuid GUID;
    };
}
