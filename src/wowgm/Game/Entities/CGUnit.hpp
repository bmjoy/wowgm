#pragma once

#include "UpdateField.hpp"
#include "ObjectGuid.hpp"
#include "CGObject.hpp"

#include <cstdint>

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    struct CGUnitData
    {
        struct UnitBytes0
        {
            std::uint8_t Race;
            std::uint8_t Class;
            std::uint8_t Gender;
            std::uint8_t PowerType;
        };

        struct UnitBytes1
        {
            std::uint8_t StandState;
            std::uint8_t FreePetTalentPoints;
            std::uint8_t StandFlags;
            std::uint8_t Flags;
        };

        struct UnitBytes2
        {
            std::uint8_t SheathState;
            std::uint8_t PvpFlags;
            std::uint8_t RenameFlags;
            std::uint8_t ShapeshiftForm;
        };

        struct DamageInfo
        {
            std::uint32_t Minimum;
            std::uint32_t Maximum;
        };

        struct AttackPowerInfo
        {
            std::uint32_t Value;
            std::uint32_t PositiveModifier;
            std::uint32_t NegativeModifier;
            float Multiplier;
        };

        CGUnitData() { }
        virtual ~CGUnitData() { }

        Descriptor<ObjectGuid> Charm;
        Descriptor<ObjectGuid> Summon;
        Descriptor<ObjectGuid> Critter;
        Descriptor<ObjectGuid> CharmedBy;
        Descriptor<ObjectGuid> SummonedBy;
        Descriptor<ObjectGuid> CreatedBy;
        Descriptor<ObjectGuid> Target;
        Descriptor<ObjectGuid> ChannelTarget;
        Descriptor<std::uint32_t> ChannelSpellID;
        Descriptor<UnitBytes0> Bytes0;
        Descriptor<std::uint32_t, 6> Powers;
        Descriptor<std::uint32_t, 6> MaxPowers;
        Descriptor<float, 5> PowerRegenModifier;
        Descriptor<float, 5> InterruptedPowerRegenModifier;
        Descriptor<std::uint32_t> Level;
        Descriptor<std::uint32_t> FactionID;
        Descriptor<std::uint32_t, 3> VirtualItems;
        Descriptor<std::uint32_t, 2> Flags;
        Descriptor<std::uint32_t> AuraState;
        Descriptor<std::uint32_t, 3> AttackTimes;
        Descriptor<float> BoundingRadius;
        Descriptor<float> CombatReach;
        Descriptor<std::uint32_t> DisplayID;
        Descriptor<std::uint32_t> NativeDisplayID;
        Descriptor<std::uint32_t> MountDisplayID;
        Descriptor<DamageInfo, 2> Damage;
        Descriptor<UnitBytes1> Bytes1;
        Descriptor<std::uint32_t> PetNumber;
        Descriptor<std::uint32_t> PetNameTimestamp;
        Descriptor<std::uint32_t> PetExperience;
        Descriptor<std::uint32_t> PetNextLevelXP;
        Descriptor<std::uint32_t> DynamicFlags;
        Descriptor<float> ModCastSpeed;
        Descriptor<float> ModCastHaste;
        Descriptor<std::uint32_t> CreatedBySpell;
        Descriptor<std::uint32_t> NpcFlags;
        Descriptor<std::uint32_t> NpcEmoteState;
        Descriptor<std::uint32_t, 5> Stats;
        Descriptor<std::uint32_t, 5> PosStats;
        Descriptor<std::uint32_t, 5> NegStats;
        Descriptor<std::uint32_t, 7> Resistances;
        Descriptor<std::uint32_t, 7> ResistancesBuffModPositive;
        Descriptor<std::uint32_t, 7> ResistancesBuffModNegative;
        Descriptor<std::uint32_t> BaseMana;
        Descriptor<std::uint32_t> BaseHealth;
        Descriptor<UnitBytes2> Bytes2;
        Descriptor<AttackPowerInfo, 3> AttackPower;
        Descriptor<DamageInfo> RangedDamage;
        Descriptor<std::uint32_t, 7> PowerCostModifier;
        Descriptor<float, 7> PowerCostMultiplier;
        Descriptor<float> MaxHealthModifier;
        Descriptor<float> HoverHeight;
        Descriptor<std::uint32_t> MaxItemLevel;
        Descriptor<std::uint32_t> _;
    };

    class CGUnit : public CGUnitData, public CGObject
    {
    public:
        explicit CGUnit(TypeMask typeMask);
        virtual ~CGUnit();

        CGUnitData const& GetUnitData() const;

        CGUnit* ToUnit() override;
        CGUnit const* ToUnit() const override;

    private:
        TypeMask _typeMask;
    };
}
