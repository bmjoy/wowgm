#pragma once

#include "UpdateField.hpp"
#include "ObjectGuid.hpp"
#include "CGObject.hpp"
#include "CClientObjCreate.hpp"

#include <cstdint>

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
        Descriptor<ObjectGuid> Owner;
        Descriptor<ObjectGuid> Contained;
        Descriptor<ObjectGuid> Creator;
        Descriptor<ObjectGuid> GiftCreator;
        Descriptor<std::uint32_t> StackCount;
        Descriptor<std::uint32_t> Duration;
        Descriptor<std::uint32_t, 5> SpellCharges;
        Descriptor<std::uint32_t> Flags;
        Descriptor<ItemEnchantmentInfo, 15> Enchantments;
        Descriptor<std::uint32_t> PropertySeed;
        Descriptor<std::uint32_t> RandomPropertiesID;
        Descriptor<std::uint32_t> Durability;
        Descriptor<std::uint32_t> MaxDurability;
        Descriptor<std::uint32_t> CreatePlayedTime;
    };
#pragma pack(pop)

    static_assert(sizeof(CGItemData) == sizeof(std::uint32_t) * 66);

    class CGItem : public CGItemData, public CGObject
    {
    public:
        explicit CGItem(CClientObjCreate const& objCreate);
        virtual ~CGItem();

        CGItemData const& GetItemData() const;

        CGPlayer* GetOwner() const;
        CGPlayer* GetCreator() const;

        CGItem* ToItem() override;
        CGItem const* ToItem() const override;
    };
}
