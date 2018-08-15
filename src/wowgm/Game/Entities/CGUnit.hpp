#pragma once

#include "ObjectGuid.hpp"
#include "CGObject.hpp"
#include "CClientObjCreate.hpp"

#include <cstdint>
#include <array>


namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

#pragma pack(push, 1)
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

        ObjectGuid Charm;
        ObjectGuid Summon;
        ObjectGuid Critter;
        ObjectGuid CharmedBy;
        ObjectGuid SummonedBy;
        ObjectGuid CreatedBy;
        ObjectGuid Target;
        ObjectGuid ChannelTarget;
        std::uint32_t ChannelSpellID;
        UnitBytes0 Bytes0;
        std::array<std::uint32_t, 6> Powers;
        std::array<std::uint32_t, 6> MaxPowers;
        std::array<float, 5> PowerRegenModifier;
        std::array<float, 5> InterruptedPowerRegenModifier;
        std::uint32_t Level;
        std::uint32_t FactionID;
        std::array<std::uint32_t, 3> VirtualItems;
        std::array<std::uint32_t, 2> Flags;
        std::uint32_t AuraState;
        std::array<std::uint32_t, 3> AttackTimes;
        float BoundingRadius;
        float CombatReach;
        std::uint32_t DisplayID;
        std::uint32_t NativeDisplayID;
        std::uint32_t MountDisplayID;
        std::array<DamageInfo, 2> Damage;
        UnitBytes1 Bytes1;
        std::uint32_t PetNumber;
        std::uint32_t PetNameTimestamp;
        std::uint32_t PetExperience;
        std::uint32_t PetNextLevelXP;
        std::uint32_t DynamicFlags;
        float ModCastSpeed;
        float ModCastHaste;
        std::uint32_t CreatedBySpell;
        std::uint32_t NpcFlags;
        std::uint32_t NpcEmoteState;
        std::array<std::uint32_t, 5> Stats;
        std::array<std::uint32_t, 5> PosStats;
        std::array<std::uint32_t, 5> NegStats;
        std::array<std::uint32_t, 7> Resistances;
        std::array<std::uint32_t, 7> ResistancesBuffModPositive;
        std::array<std::uint32_t, 7> ResistancesBuffModNegative;
        std::uint32_t BaseMana;
        std::uint32_t BaseHealth;
        UnitBytes2 Bytes2;
        std::array<AttackPowerInfo, 3> AttackPower;
        DamageInfo RangedDamage;
        std::array<std::uint32_t, 7> PowerCostModifier;
        std::array<float, 7> PowerCostMultiplier;
        float MaxHealthModifier;
        float HoverHeight;
        std::uint32_t MaxItemLevel;
        std::uint32_t _;
    };
#pragma pack(pop)

    static_assert(sizeof(CGUnitData) == sizeof(std::uint32_t) * 142);

    class CGUnit : public CGUnitData, public CGObject
    {
    public:
        explicit CGUnit(CClientObjCreate const& typeMask);
        virtual ~CGUnit();

        CGUnitData const& GetUnitData() const;
        CGUnitData& GetUnitData();

        CGUnit* ToUnit() override;
        CGUnit const* ToUnit() const override;

        void UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate) override;
    };
}
