#pragma once

#include "DBCStructures.hpp"
#include "DBCMeta.hpp"
#include "DBTraits.hpp"
#include "DBStorage.hpp"
#include "Logger.hpp"

#include <cstdint>
#include <cstddef>
#include <chrono>

// AUTOGENERATED FILE - DO NOT EDIT
// See contrib/dbmeta.py
namespace wowgm::game::datastores
{
    namespace DataStores
    {
        void Initialize()
        {
            using hrc = std::chrono::high_resolution_clock;

            auto start = hrc::now();

            // Storage<Startup_StringsEntry>::Initialize();
            Storage<ItemSparseEntry>::Initialize();
            // Storage<ItemEntry>::Initialize();
            // Storage<SpellVisualKitEntry>::Initialize();
            // Storage<SpellVisualEffectNameEntry>::Initialize();
            // Storage<SpellEffectEntry>::Initialize();
            // Storage<ParticleColorEntry>::Initialize();
            // Storage<ObjectEffectPackageElemEntry>::Initialize();
            // Storage<ObjectEffectPackageEntry>::Initialize();
            // Storage<ObjectEffectModifierEntry>::Initialize();
            // Storage<ObjectEffectGroupEntry>::Initialize();
            // Storage<ObjectEffectEntry>::Initialize();
            // Storage<NameGenEntry>::Initialize();
            // Storage<LoadingScreensEntry>::Initialize();
            // Storage<ItemVisualEffectsEntry>::Initialize();
            // Storage<ItemVisualsEntry>::Initialize();
            // Storage<ItemDisplayInfoEntry>::Initialize();
            // Storage<HelmetGeosetVisDataEntry>::Initialize();
            // Storage<GuildColorEmblemEntry>::Initialize();
            // Storage<GuildColorBorderEntry>::Initialize();
            // Storage<GuildColorBackgroundEntry>::Initialize();
            // Storage<GlueScreenEmoteEntry>::Initialize();
            // Storage<GameTipsEntry>::Initialize();
            // Storage<CreatureModelDataEntry>::Initialize();
            // Storage<CreatureFamilyEntry>::Initialize();
            // Storage<CreatureDisplayInfoExtraEntry>::Initialize();
            // Storage<CreatureDisplayInfoEntry>::Initialize();
            // Storage<CharStartOutfitEntry>::Initialize();
            // Storage<AnimReplacementSetEntry>::Initialize();
            // Storage<AnimReplacementEntry>::Initialize();
            // Storage<AnimKitSegmentEntry>::Initialize();
            // Storage<AnimKitPriorityEntry>::Initialize();
            // Storage<SoundProviderPreferencesEntry>::Initialize();
            // Storage<SpamMessagesEntry>::Initialize();
            // Storage<SoundFilterElemEntry>::Initialize();
            // Storage<SoundFilterEntry>::Initialize();
            // Storage<ResistancesEntry>::Initialize();
            // Storage<NamesReservedEntry>::Initialize();
            // Storage<NamesProfanityEntry>::Initialize();
            // Storage<MovieVariationEntry>::Initialize();
            // Storage<MovieFileDataEntry>::Initialize();
            // Storage<MovieEntry>::Initialize();
            // Storage<ItemSubClassEntry>::Initialize();
            // Storage<ItemClassEntry>::Initialize();
            // Storage<FileDataEntry>::Initialize();
            // Storage<FactionTemplateEntry>::Initialize();
            // Storage<FactionGroupEntry>::Initialize();
            // Storage<ChrRacesEntry>::Initialize();
            // Storage<ChrClassesEntry>::Initialize();
            // Storage<ChatProfanityEntry>::Initialize();
            // Storage<CharacterFacialHairStylesEntry>::Initialize();
            // Storage<CharSectionsEntry>::Initialize();
            // Storage<CharHairGeosetsEntry>::Initialize();
            // Storage<CharBaseInfoEntry>::Initialize();
            // Storage<Cfg_ConfigsEntry>::Initialize();
            // Storage<Cfg_CategoriesEntry>::Initialize();
            // Storage<BannedAddOnsEntry>::Initialize();
            // Storage<AnimKitConfigBoneSetEntry>::Initialize();
            // Storage<AnimKitConfigEntry>::Initialize();
            // Storage<AnimKitEntry>::Initialize();
            // Storage<AnimKitBoneSetEntry>::Initialize();
            // Storage<AnimKitBoneSetAliasEntry>::Initialize();
            // Storage<PhaseShiftZoneSoundsEntry>::Initialize();
            // Storage<WorldChunkSoundsEntry>::Initialize();
            // Storage<WorldStateZoneSoundsEntry>::Initialize();
            // Storage<ZoneMusicEntry>::Initialize();
            // Storage<ZoneIntroMusicTableEntry>::Initialize();
            // Storage<WorldStateUIEntry>::Initialize();
            // Storage<WorldSafeLocsEntry>::Initialize();
            // Storage<WorldMapTransformsEntry>::Initialize();
            // Storage<WorldMapOverlayEntry>::Initialize();
            // Storage<WorldMapContinentEntry>::Initialize();
            // Storage<WorldMapAreaEntry>::Initialize();
            // Storage<WeaponSwingSounds2Entry>::Initialize();
            // Storage<WeaponImpactSoundsEntry>::Initialize();
            // Storage<World_PVP_AreaEntry>::Initialize();
            // Storage<VocalUISoundsEntry>::Initialize();
            // Storage<VehicleUIIndSeatEntry>::Initialize();
            // Storage<VehicleUIIndicatorEntry>::Initialize();
            // Storage<VehicleSeatEntry>::Initialize();
            // Storage<VehicleEntry>::Initialize();
            // Storage<UnitPowerBarEntry>::Initialize();
            // Storage<UnitBloodEntry>::Initialize();
            // Storage<UnitBloodLevelsEntry>::Initialize();
            // Storage<TransportRotationEntry>::Initialize();
            // Storage<TransportPhysicsEntry>::Initialize();
            // Storage<TransportAnimationEntry>::Initialize();
            // Storage<TotemCategoryEntry>::Initialize();
            // Storage<TerrainTypeSoundsEntry>::Initialize();
            // Storage<TaxiPathEntry>::Initialize();
            // Storage<TaxiPathNodeEntry>::Initialize();
            // Storage<TaxiNodesEntry>::Initialize();
            // Storage<TalentTreePrimarySpellsEntry>::Initialize();
            // Storage<TalentTabEntry>::Initialize();
            // Storage<TalentEntry>::Initialize();
            // Storage<SummonPropertiesEntry>::Initialize();
            // Storage<StringLookupsEntry>::Initialize();
            // Storage<StationeryEntry>::Initialize();
            // Storage<SpellVisualKitModelAttachEntry>::Initialize();
            // Storage<SpellVisualKitAreaModelEntry>::Initialize();
            // Storage<SpellVisualEntry>::Initialize();
            // Storage<SpellTotemsEntry>::Initialize();
            // Storage<SpellTargetRestrictionsEntry>::Initialize();
            // Storage<SpellSpecialUnitEffectEntry>::Initialize();
            // Storage<SpellShapeshiftFormEntry>::Initialize();
            // Storage<SpellShapeshiftEntry>::Initialize();
            // Storage<SpellScalingEntry>::Initialize();
            // Storage<SpellRuneCostEntry>::Initialize();
            // Storage<SpellReagentsEntry>::Initialize();
            // Storage<SpellPowerEntry>::Initialize();
            // Storage<SpellRangeEntry>::Initialize();
            // Storage<SpellRadiusEntry>::Initialize();
            // Storage<SpellMissileMotionEntry>::Initialize();
            // Storage<SpellMissileEntry>::Initialize();
            // Storage<SpellMechanicEntry>::Initialize();
            // Storage<SpellLevelsEntry>::Initialize();
            // Storage<SpellItemEnchantmentConditionEntry>::Initialize();
            // Storage<SpellItemEnchantmentEntry>::Initialize();
            // Storage<SpellInterruptsEntry>::Initialize();
            // Storage<SpellIconEntry>::Initialize();
            // Storage<SpellFocusObjectEntry>::Initialize();
            // Storage<SpellFlyoutItemEntry>::Initialize();
            // Storage<SpellFlyoutEntry>::Initialize();
            // Storage<SpellEquippedItemsEntry>::Initialize();
            // Storage<SpellEffectCameraShakesEntry>::Initialize();
            // Storage<SpellDurationEntry>::Initialize();
            // Storage<SpellDispelTypeEntry>::Initialize();
            // Storage<SpellDifficultyEntry>::Initialize();
            // Storage<SpellDescriptionVariablesEntry>::Initialize();
            Storage<SpellEntry>::Initialize();
            // Storage<SpellCooldownsEntry>::Initialize();
            // Storage<SpellClassOptionsEntry>::Initialize();
            // Storage<SpellChainEffectsEntry>::Initialize();
            // Storage<SpellCategoryEntry>::Initialize();
            // Storage<SpellCategoriesEntry>::Initialize();
            // Storage<SpellCastTimesEntry>::Initialize();
            // Storage<SpellCastingRequirementsEntry>::Initialize();
            // Storage<SpellAuraVisXTalentTabEntry>::Initialize();
            // Storage<SpellAuraVisibilityEntry>::Initialize();
            // Storage<SpellAuraRestrictionsEntry>::Initialize();
            // Storage<SpellAuraOptionsEntry>::Initialize();
            // Storage<SpellActivationOverlayEntry>::Initialize();
            // Storage<SoundAmbienceFlavorEntry>::Initialize();
            // Storage<SoundAmbienceEntry>::Initialize();
            // Storage<SkillTiersEntry>::Initialize();
            // Storage<SkillRaceClassInfoEntry>::Initialize();
            // Storage<SkillLineEntry>::Initialize();
            // Storage<SkillLineCategoryEntry>::Initialize();
            // Storage<SkillLineAbilitySortedSpellEntry>::Initialize();
            // Storage<SkillLineAbilityEntry>::Initialize();
            // Storage<ServerMessagesEntry>::Initialize();
            // Storage<ScreenLocationEntry>::Initialize();
            // Storage<ScreenEffectEntry>::Initialize();
            // Storage<ScalingStatValuesEntry>::Initialize();
            // Storage<ScalingStatDistributionEntry>::Initialize();
            // Storage<RandPropPointsEntry>::Initialize();
            // Storage<ResearchSiteEntry>::Initialize();
            // Storage<ResearchProjectEntry>::Initialize();
            // Storage<ResearchFieldEntry>::Initialize();
            // Storage<ResearchBranchEntry>::Initialize();
            // Storage<QuestXPEntry>::Initialize();
            // Storage<QuestSortEntry>::Initialize();
            // Storage<QuestPOIPointEntry>::Initialize();
            // Storage<QuestPOIBlobEntry>::Initialize();
            // Storage<QuestInfoEntry>::Initialize();
            // Storage<QuestFactionRewardEntry>::Initialize();
            // Storage<PvpDifficultyEntry>::Initialize();
            // Storage<PowerDisplayEntry>::Initialize();
            // Storage<PlayerConditionEntry>::Initialize();
            // Storage<PhaseXPhaseGroupEntry>::Initialize();
            // Storage<PhaseEntry>::Initialize();
            // Storage<PaperDollItemFrameEntry>::Initialize();
            // Storage<PageTextMaterialEntry>::Initialize();
            // Storage<PackageEntry>::Initialize();
            // Storage<OverrideSpellDataEntry>::Initialize();
            // Storage<NumTalentsAtLevelEntry>::Initialize();
            // Storage<NPCSoundsEntry>::Initialize();
            // Storage<MountTypeEntry>::Initialize();
            // Storage<MountCapabilityEntry>::Initialize();
            // Storage<MaterialEntry>::Initialize();
            // Storage<MapDifficultyEntry>::Initialize();
            // Storage<MailTemplateEntry>::Initialize();
            // Storage<LockTypeEntry>::Initialize();
            // Storage<LockEntry>::Initialize();
            // Storage<LoadingScreenTaxiSplinesEntry>::Initialize();
            // Storage<LfgDungeonsEntry>::Initialize();
            // Storage<LfgDungeonsGroupingMapEntry>::Initialize();
            // Storage<LfgDungeonGroupEntry>::Initialize();
            // Storage<LfgDungeonExpansionEntry>::Initialize();
            // Storage<LanguagesEntry>::Initialize();
            // Storage<LanguageWordsEntry>::Initialize();
            // Storage<JournalInstanceEntry>::Initialize();
            // Storage<JournalEncounterSectionEntry>::Initialize();
            // Storage<JournalEncounterEntry>::Initialize();
            // Storage<JournalEncounterItemEntry>::Initialize();
            // Storage<JournalEncounterCreatureEntry>::Initialize();
            // Storage<ItemSubClassMaskEntry>::Initialize();
            // Storage<ItemSetEntry>::Initialize();
            // Storage<ItemReforgeEntry>::Initialize();
            // Storage<ItemRandomSuffixEntry>::Initialize();
            // Storage<ItemRandomPropertiesEntry>::Initialize();
            // Storage<ItemPurchaseGroupEntry>::Initialize();
            // Storage<ItemPriceBaseEntry>::Initialize();
            // Storage<ItemPetFoodEntry>::Initialize();
            // Storage<ItemNameDescriptionEntry>::Initialize();
            // Storage<ItemLimitCategoryEntry>::Initialize();
            // Storage<ItemGroupSoundsEntry>::Initialize();
            // Storage<ItemDisenchantLootEntry>::Initialize();
            // Storage<ItemDamageWandEntry>::Initialize();
            // Storage<ItemDamageTwoHandCasterEntry>::Initialize();
            // Storage<ItemDamageTwoHandEntry>::Initialize();
            // Storage<ItemDamageThrownEntry>::Initialize();
            // Storage<ItemDamageRangedEntry>::Initialize();
            // Storage<ItemDamageOneHandCasterEntry>::Initialize();
            // Storage<ItemDamageOneHandEntry>::Initialize();
            // Storage<ItemDamageAmmoEntry>::Initialize();
            // Storage<ItemBagFamilyEntry>::Initialize();
            // Storage<ItemArmorShieldEntry>::Initialize();
            // Storage<ItemArmorTotalEntry>::Initialize();
            // Storage<ItemArmorQualityEntry>::Initialize();
            // Storage<ImportPriceWeaponEntry>::Initialize();
            // Storage<ImportPriceShieldEntry>::Initialize();
            // Storage<ImportPriceQualityEntry>::Initialize();
            // Storage<ImportPriceArmorEntry>::Initialize();
            // Storage<HolidaysEntry>::Initialize();
            // Storage<HolidayNamesEntry>::Initialize();
            // Storage<HolidayDescriptionsEntry>::Initialize();
            // Storage<GuildPerkSpellsEntry>::Initialize();
            // Storage<gtSpellScalingEntry>::Initialize();
            // Storage<gtRegenMPPerSptEntry>::Initialize();
            // Storage<gtOCTRegenMPEntry>::Initialize();
            // Storage<gtOCTHpPerStaminaEntry>::Initialize();
            // Storage<gtOCTClassCombatRatingScalarEntry>::Initialize();
            // Storage<gtOCTBaseMPByClassEntry>::Initialize();
            // Storage<gtOCTBaseHPByClassEntry>::Initialize();
            // Storage<gtNPCManaCostScalerEntry>::Initialize();
            // Storage<gtChanceToSpellCritBaseEntry>::Initialize();
            // Storage<gtChanceToSpellCritEntry>::Initialize();
            // Storage<gtChanceToMeleeCritBaseEntry>::Initialize();
            // Storage<gtChanceToMeleeCritEntry>::Initialize();
            // Storage<gtCombatRatingsEntry>::Initialize();
            // Storage<gtBarberShopCostBaseEntry>::Initialize();
            // Storage<GMTicketCategoryEntry>::Initialize();
            // Storage<GMSurveySurveysEntry>::Initialize();
            // Storage<GMSurveyQuestionsEntry>::Initialize();
            // Storage<GMSurveyCurrentSurveyEntry>::Initialize();
            // Storage<GMSurveyAnswersEntry>::Initialize();
            // Storage<GlyphSlotEntry>::Initialize();
            // Storage<GlyphPropertiesEntry>::Initialize();
            // Storage<GemPropertiesEntry>::Initialize();
            // Storage<GameTablesEntry>::Initialize();
            // Storage<GameObjectDisplayInfoEntry>::Initialize();
            // Storage<GameObjectArtKitEntry>::Initialize();
            // Storage<FootstepTerrainLookupEntry>::Initialize();
            // Storage<FactionEntry>::Initialize();
            // Storage<ExhaustionEntry>::Initialize();
            // Storage<EnvironmentalDamageEntry>::Initialize();
            // Storage<EmotesTextEntry>::Initialize();
            // Storage<EmotesTextSoundEntry>::Initialize();
            // Storage<EmotesTextDataEntry>::Initialize();
            // Storage<EmotesEntry>::Initialize();
            // Storage<DurabilityQualityEntry>::Initialize();
            // Storage<DurabilityCostsEntry>::Initialize();
            // Storage<DungeonMapChunkEntry>::Initialize();
            // Storage<DungeonMapEntry>::Initialize();
            // Storage<DungeonEncounterEntry>::Initialize();
            // Storage<DestructibleModelDataEntry>::Initialize();
            // Storage<DeathThudLookupsEntry>::Initialize();
            // Storage<DanceMovesEntry>::Initialize();
            // Storage<CurrencyCategoryEntry>::Initialize();
            // Storage<CurrencyTypesEntry>::Initialize();
            // Storage<CreatureTypeEntry>::Initialize();
            // Storage<CreatureSpellDataEntry>::Initialize();
            // Storage<CreatureSoundDataEntry>::Initialize();
            // Storage<CreatureMovementInfoEntry>::Initialize();
            // Storage<CreatureImmunitiesEntry>::Initialize();
            // Storage<CinematicSequencesEntry>::Initialize();
            // Storage<CinematicCameraEntry>::Initialize();
            // Storage<ChrClassesXPowerTypesEntry>::Initialize();
            // Storage<ChatChannelsEntry>::Initialize();
            // Storage<CharTitlesEntry>::Initialize();
            // Storage<CastableRaidBuffsEntry>::Initialize();
            // Storage<CameraShakesEntry>::Initialize();
            // Storage<CameraModeEntry>::Initialize();
            // Storage<BattlemasterListEntry>::Initialize();
            // Storage<BarberShopStyleEntry>::Initialize();
            // Storage<BankBagSlotPricesEntry>::Initialize();
            // Storage<AuctionHouseEntry>::Initialize();
            // Storage<ArmorLocationEntry>::Initialize();
            // Storage<AreaTriggerEntry>::Initialize();
            // Storage<AreaAssignmentEntry>::Initialize();
            // Storage<AreaPOISortedWorldStateEntry>::Initialize();
            // Storage<AreaPOIEntry>::Initialize();
            // Storage<AreaGroupEntry>::Initialize();
            // Storage<Achievement_CategoryEntry>::Initialize();
            // Storage<Achievement_CriteriaEntry>::Initialize();
            // Storage<AchievementEntry>::Initialize();
            // Storage<ItemCurrencyCostEntry>::Initialize();
            // Storage<ItemExtendedCostEntry>::Initialize();
            // Storage<KeyChainEntry>::Initialize();
            // Storage<DeclinedWordCasesEntry>::Initialize();
            // Storage<DeclinedWordEntry>::Initialize();
            // Storage<ZoneLightPointEntry>::Initialize();
            // Storage<ZoneLightEntry>::Initialize();
            // Storage<WMOAreaTableEntry>::Initialize();
            // Storage<WeatherEntry>::Initialize();
            // Storage<TerrainTypeEntry>::Initialize();
            // Storage<TerrainMaterialEntry>::Initialize();
            // Storage<SoundEntriesFallbacksEntry>::Initialize();
            // Storage<SoundEmittersEntry>::Initialize();
            // Storage<SoundEmitterPillPointsEntry>::Initialize();
            // Storage<LiquidTypeEntry>::Initialize();
            // Storage<LiquidObjectEntry>::Initialize();
            // Storage<LiquidMaterialEntry>::Initialize();
            // Storage<LightSkyboxEntry>::Initialize();
            // Storage<LightFloatBandEntry>::Initialize();
            // Storage<LightEntry>::Initialize();
            // Storage<GroundEffectTextureEntry>::Initialize();
            // Storage<GroundEffectDoodadEntry>::Initialize();
            // Storage<FootprintTexturesEntry>::Initialize();
            Storage<MapEntry>::Initialize();
            // Storage<LightParamsEntry>::Initialize();
            // Storage<LightIntBandEntry>::Initialize();
            // Storage<AreaTableEntry>::Initialize();
            // Storage<AnimationDataEntry>::Initialize();
            // Storage<SoundEntriesEntry>::Initialize();
            // Storage<SoundEntriesAdvancedEntry>::Initialize();
            // Storage<VideoHardwareEntry>::Initialize();

            auto end = hrc::now();

            LOG_PERFORMANCE << "DBCs loaded in " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0f << " milliseconds";
        }

        template <typename T> T const* GetRecord(std::uint32_t index)
        {
            return Storage<T>::GetRecord(index);
        }
    }
}