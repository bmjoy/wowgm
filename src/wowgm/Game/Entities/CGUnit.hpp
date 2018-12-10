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
            uint8_t Race;
            uint8_t Class;
            uint8_t Gender;
            uint8_t PowerType;
        };

        struct UnitBytes1
        {
            uint8_t StandState;
            uint8_t FreePetTalentPoints;
            uint8_t StandFlags;
            uint8_t Flags;
        };

        struct UnitBytes2
        {
            uint8_t SheathState;
            uint8_t PvpFlags;
            uint8_t RenameFlags;
            uint8_t ShapeshiftForm;
        };

        struct DamageInfo
        {
            float Minimum;
            float Maximum;
        };

        struct AttackPowerInfo
        {
            uint32_t Value;
            uint32_t PositiveModifier;
            uint32_t NegativeModifier;
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
        uint32_t ChannelSpellID;
        UnitBytes0 UnitBytes0;
        std::array<uint32_t, 6> Powers;
        std::array<uint32_t, 6> MaxPowers;
        std::array<float, 5> PowerRegenModifier;
        std::array<float, 5> InterruptedPowerRegenModifier;
        uint32_t Level;
        uint32_t FactionID;
        std::array<uint32_t, 3> VirtualItems;
        std::array<uint32_t, 2> UnitFlags;
        uint32_t AuraState;
        std::array<uint32_t, 3> AttackTimes;
        float BoundingRadius;
        float CombatReach;
        uint32_t DisplayID;
        uint32_t NativeDisplayID;
        uint32_t MountDisplayID;
        std::array<DamageInfo, 2> Damage;
        UnitBytes1 UnitBytes1;
        uint32_t PetNumber;
        uint32_t PetNameTimestamp;
        uint32_t PetExperience;
        uint32_t PetNextLevelXP;
        uint32_t DynamicFlags;
        float ModCastSpeed;
        float ModCastHaste;
        uint32_t CreatedBySpell;
        uint32_t NpcFlags;
        uint32_t NpcEmoteState;
        std::array<uint32_t, 5> Stats;
        std::array<uint32_t, 5> PosStats;
        std::array<uint32_t, 5> NegStats;
        std::array<uint32_t, 7> Resistances;
        std::array<uint32_t, 7> ResistancesBuffModPositive;
        std::array<uint32_t, 7> ResistancesBuffModNegative;
        uint32_t BaseMana;
        uint32_t BaseHealth;
        UnitBytes2 UnitBytes2;
        std::array<AttackPowerInfo, 2> AttackPower;
        DamageInfo RangedDamage;
        std::array<uint32_t, 7> PowerCostModifier;
        std::array<float, 7> PowerCostMultiplier;
        float MaxHealthModifier;
        float HoverHeight;
        uint32_t MaxItemLevel;
        uint32_t _;
    };
#pragma pack(pop)

    static_assert(sizeof(CGUnitData) == sizeof(uint32_t) * 0x008A);

    class CGUnit : public CGObject, public CGUnitData
    {
    public:
        explicit CGUnit(CClientObjCreate const& createBlock);
        virtual ~CGUnit();

        CGUnitData const& GetUnitData() const;
        CGUnitData& GetUnitData();

        CGUnit* ToUnit() override;
        CGUnit const* ToUnit() const override;

        void UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate) override;
    };
}
