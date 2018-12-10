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
        uint32_t ID;
        uint32_t Duration;
        uint16_t _;
        uint16_t Charges;
    };

    class CGPlayer;

#pragma pack(push, 1)
    struct CGItemData
    {
        ObjectGuid Owner;
        ObjectGuid Contained;
        ObjectGuid Creator;
        ObjectGuid GiftCreator;
        uint32_t StackCount;
        uint32_t Duration;
        std::array<uint32_t, 5> SpellCharges;
        uint32_t Flags;
        std::array<ItemEnchantmentInfo, 15> Enchantments;
        uint32_t PropertySeed;
        uint32_t RandomPropertiesID;
        uint32_t Durability;
        uint32_t MaxDurability;
        uint32_t CreatePlayedTime;
    };
#pragma pack(pop)

    static_assert(sizeof(CGItemData) == sizeof(uint32_t) * 66);

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
