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
            uint32_t ID;
            uint32_t State;
            std::array<uint16_t, 4> Objectives;
            uint32_t Time;
        };

        struct VisibleItem
        {
            uint32_t ID;
            uint16_t PermanentEnchantmentID;
            uint16_t TemporaryEnchantmentID;
        };

        ObjectGuid DuelArbiter;
        uint32_t Flags;
        uint32_t GuildRank;
        uint32_t GuildDeleteDate;
        uint32_t GuildLevel;
        std::array<uint8_t, 12> PlayerBytes;
        uint32_t DuelTeam;
        uint32_t GuildTimestamp;
        std::array<QuestLogEntry, 50> QuestLog;
        std::array<VisibleItem, 19> VisibleItems;
        uint32_t ChosenTitle;
        uint32_t FakeInebriation;
        uint32_t _;
        std::array<ObjectGuid, 23> Gear;
        std::array<ObjectGuid, 16> Backpack;
        std::array<ObjectGuid, 28> Bank;
        std::array<ObjectGuid, 7> BankBags;
        std::array<ObjectGuid, 12> BuyBack;
        ObjectGuid FarSight;
        std::bitset<64 * 4> KnownTitles;
        uint32_t XP;
        uint32_t NextLevelXP;
        std::array<uint16_t, 128> SkillLineID;
        std::array<uint16_t, 128> SkillStep;
        std::array<uint16_t, 128> SkillRank;
        std::array<uint16_t, 128> SkillMaxRank;
        std::array<uint16_t, 128> SkillModifier;
        std::array<uint16_t, 128> SkillTalent;
        uint32_t CharacterPoints;
        uint32_t TrackCreatures;
        uint32_t TrackResources;
        std::array<uint32_t, 2> Expertise;
        float BlockChance;
        float DodgeChance;
        float ParryChance;
        std::array<float, 3> CritChance;
        std::array<float, 7> SpellCritChance;
        uint32_t ShieldBlock;
        float ShieldBlockCritChance;
        float Mastery;
        std::bitset<8 * (4 * 156)> ExploredZones;
        uint32_t RestStateExperience;
        uint64_t Coinage;
        struct {
            std::array<uint32_t, 7> PositiveModifier;
            std::array<uint32_t, 7> NegativeModifier;
            std::array<uint32_t, 7> PercentageModifier;
        } Damage;
        uint32_t ModHealingDonePositiveModifier;
        float ModHealingPct;
        float ModHealingDonePct;
        std::array<float, 3> WeaponDamageMultipliers;
        float ModSpellPowerPercentage;
        float OverrideSpellPowerByApPct;
        uint32_t ModTargetResistance;
        uint32_t ModTargetPhysicalResistance;
        std::array<uint8_t, 4> PlayerFieldBytes;
        uint32_t SelfResurrectionSpellID;
        uint32_t PvpMedals;
        std::array<uint32_t, 12> BuybackPrice;
        std::array<uint32_t, 12> BuybackTimestamp;
        std::array<uint16_t, 2> Kills;
        uint32_t HonorableLifetimeKills;
        std::array<uint8_t, 4> Bytes2;
        uint32_t WatchedFactionID;
        std::array<uint32_t, 26> CombatRating;
        std::array<uint32_t, 21> ArenaTeamInfo;
        uint32_t BattlegroundRating;
        uint32_t MaxLevel;
        std::array<uint32_t, 25> DailyQuests;
        std::array<float, 4> RuneRegen;
        std::array<uint32_t, 3> NoReagentCost;
        std::array<uint32_t, 9> GlyphSlots;
        std::array<uint32_t, 9> Glyphs;
        uint32_t GlyphsEnabled;
        uint32_t PetSpellPower;
        std::array<uint16_t, 16> FieldResearching;
        std::array<uint16_t, 16> FieldResearchSite;
        std::array<uint32_t, 2> ProfessionSkillLine;
        float UIHitModifier;
        float UISpellHitModifier;
        uint32_t RealmTimeOffset;
        float ModHaste;
        float ModRangedHaste;
        float ModPetHaste;
        float ModHasteRegen;
    };
#pragma pack(pop)

    static_assert(sizeof(CGPlayerData) == sizeof(uint32_t) * 0x04D6);

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