#pragma once

#include "Packet.hpp"
#include "ObjectGuid.hpp"

#include <string>
#include <vector>
#include <boost/optional.hpp>

namespace wowgm::protocol::world::packets
{
    using namespace wowgm::utilities;

    struct PetInfo
    {
        std::uint32_t DisplayID;
        std::uint32_t Family;
        std::uint32_t Level;
    };

    struct CharacterInfo
    {
        ObjectGuid GUID;
        ObjectGuid GuildGUID;
        std::string Name;

        PetInfo Pet;

        std::uint8_t Order;
        std::int32_t MapID;
        float X, Y, Z; // C3Vector

        std::uint8_t Gender;

        std::uint8_t Race;
        std::uint8_t Class;
        std::uint32_t ZoneID;

        std::uint32_t Flags;
        std::uint32_t CustomizationFlags;

        std::uint8_t SkinID;
        std::uint8_t FacialHairID;
        std::uint8_t HairColorID;
        std::uint8_t HairStyleID;
        std::uint8_t FaceID;

        std::uint32_t Level;

        bool HasPet() const;
        bool IsInGuild() const;
    };

    struct ClientEnumCharactersResult : public ServerPacket
    {
        ClientEnumCharactersResult(WorldPacket&& packet);

        void Read() override;

        std::vector<CharacterInfo> Characters;
    };
}
