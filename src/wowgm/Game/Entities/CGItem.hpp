#pragma once

#include "ObjectGuid.hpp"
#include "CGObject.hpp"
#include "CClientObjCreate.hpp"
#include "JamCliValuesUpdate.hpp"

#include <cstdint>
#include <array>


namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    struct ItemEnchantmentInfo
    {
        std::uint32_t ID;
        std::uint32_t Duration;
        std::uint16_t _;
        std::uint16_t Charges;
    };

    class CGPlayer;

#pragma pack(push, 1)
    struct CGItemData
    {
        ObjectGuid Owner;
        ObjectGuid Contained;
        ObjectGuid Creator;
        ObjectGuid GiftCreator;
        std::uint32_t StackCount;
        std::uint32_t Duration;
        std::array<std::uint32_t, 5> SpellCharges;
        std::uint32_t Flags;
        std::array<ItemEnchantmentInfo, 15> Enchantments;
        std::uint32_t PropertySeed;
        std::uint32_t RandomPropertiesID;
        std::uint32_t Durability;
        std::uint32_t MaxDurability;
        std::uint32_t CreatePlayedTime;
    };
#pragma pack(pop)

    static_assert(sizeof(CGItemData) == sizeof(std::uint32_t) * 66);

    class CGItem : public CGItemData, public CGObject
    {
    public:
        explicit CGItem(CClientObjCreate const& objCreate);
        virtual ~CGItem();

        CGItemData const& GetItemData() const;
        CGItemData& GetItemData();

        CGPlayer* GetOwner() const;
        CGPlayer* GetCreator() const;

        CGItem* ToItem() override;
        CGItem const* ToItem() const override;

        void UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate) override;
    };
}
