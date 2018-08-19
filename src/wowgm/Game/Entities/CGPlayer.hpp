#pragma once

#include "ObjectGuid.hpp"
#include "CGObject.hpp"
#include "CGUnit.hpp"
#include "JamCliValuesUpdate.hpp"
#include "CClientObjCreate.hpp"

#include <array>
#include <cstdint>
#include <bitset>

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

#pragma pack(push, 1)
    struct CGPlayerData
    {
        struct QuestLogEntry
        {
            std::uint32_t ID;
            std::uint32_t State;
            std::array<std::uint16_t, 4> Objectives;
            std::uint32_t Time;
        };

        struct VisibleItem
        {
            std::uint32_t ID;
            std::uint16_t PermanentEnchantmentID;
            std::uint16_t TemporaryEnchantmentID;
        };

        ObjectGuid DuelArbiter;
        std::uint32_t Flags;
        std::uint32_t GuildRank;
        std::uint32_t GuildDeleteDate;
        std::uint32_t GuildLevel;
        std::array<std::uint8_t, 12> PlayerBytes;
        std::uint32_t DuelTeam;
        std::uint32_t GuildTimestamp;
        std::array<QuestLogEntry, 50> QuestLog;
        std::array<VisibleItem, 19> VisibleItems;
        std::uint32_t ChosenTitle;
        std::uint32_t FakeInebriation;
        std::uint32_t _;
        std::array<ObjectGuid, 23> Gear;
        std::array<ObjectGuid, 16> Backpack;
        std::array<ObjectGuid, 28> Bank;
        std::array<ObjectGuid, 7> BankBags;
        std::array<ObjectGuid, 12> BuyBack;
        ObjectGuid FarSight;
        std::bitset<64 * 4> KnownTitles;
        std::uint32_t XP;
        std::uint32_t NextLevelXP;
        std::array<std::uint16_t, 128> SkillLineID;
        std::array<std::uint16_t, 128> SkillStep;
        std::array<std::uint16_t, 128> SkillRank;
        std::array<std::uint16_t, 128> SkillMaxRank;
        std::array<std::uint16_t, 128> SkillModifier;
        std::array<std::uint16_t, 128> SkillTalent;
        std::uint32_t CharacterPoints;
        std::uint32_t TrackCreatures;
        std::uint32_t TrackResources;
        std::array<std::uint32_t, 2> Expertise;
        float BlockChance;
        float DodgeChance;
        float ParryChance;
        std::array<float, 3> CritChance;
        std::array<float, 7> SpellCritChance;
        std::uint32_t ShieldBlock;
        float ShieldBlockCritChance;
        float Mastery;
        std::bitset<8 * (4 * 156)> ExploredZones;
        std::uint32_t RestStateExperience;
        std::uint64_t Coinage;
        struct {
            std::array<std::uint32_t, 7> PositiveModifier;
            std::array<std::uint32_t, 7> NegativeModifier;
            std::array<std::uint32_t, 7> PercentageModifier;
        } Damage;
        std::uint32_t ModHealingDonePositiveModifier;
        float ModHealingPct;
        float ModHealingDonePct;
        std::array<float, 3> WeaponDamageMultipliers;
        float ModSpellPowerPercentage;
        float OverrideSpellPowerByApPct;
        std::uint32_t ModTargetResistance;
        std::uint32_t ModTargetPhysicalResistance;
        std::array<std::uint8_t, 4> PlayerFieldBytes;
        std::uint32_t SelfResurrectionSpellID;
        std::uint32_t PvpMedals;
        std::array<std::uint32_t, 12> BuybackPrice;
        std::array<std::uint32_t, 12> BuybackTimestamp;
        std::array<std::uint16_t, 2> Kills;
        std::uint32_t HonorableLifetimeKills;
        std::array<std::uint8_t, 4> Bytes2;
        std::uint32_t WatchedFactionID;
        std::array<std::uint32_t, 26> CombatRating;
        std::array<std::uint32_t, 21> ArenaTeamInfo;
        std::uint32_t BattlegroundRating;
        std::uint32_t MaxLevel;
        std::array<std::uint32_t, 25> DailyQuests;
        std::array<float, 4> RuneRegen;
        std::array<std::uint32_t, 3> NoReagentCost;
        std::array<std::uint32_t, 9> GlyphSlots;
        std::array<std::uint32_t, 9> Glyphs;
        std::uint32_t GlyphsEnabled;
        std::uint32_t PetSpellPower;
        std::array<std::uint16_t, 16> FieldResearching;
        std::array<std::uint16_t, 16> FieldResearchSite;
        std::array<std::uint32_t, 2> ProfessionSkillLine;
        float UIHitModifier;
        float UISpellHitModifier;
        std::uint32_t RealmTimeOffset;
        float ModHaste;
        float ModRangedHaste;
        float ModPetHaste;
        float ModHasteRegen;
    };
#pragma pack(pop)

    static_assert(sizeof(CGPlayerData) == sizeof(std::uint32_t) * 0x04D6);

    class CGPlayer : public CGUnit, public CGPlayerData
    {
    public:
        explicit CGPlayer(CClientObjCreate const& createBlock);
        virtual ~CGPlayer();

        CGPlayerData const& GetPlayerData() const;
        CGPlayerData& GetPlayerData();

        CGPlayer* ToPlayer() override;
        CGPlayer const* ToPlayer() const override;

        void UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate) override;
    };
}