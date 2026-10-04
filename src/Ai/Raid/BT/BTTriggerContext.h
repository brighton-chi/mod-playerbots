/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTTRIGGERCONTEXT_H
#define PLAYERBOTS_BTTRIGGERCONTEXT_H

#include "BTTriggers.h"
#include "NamedObjectContext.h"

class RaidBlackTempleTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidBlackTempleTriggerContext()
    {
        // General
        creators["black temple no encounter in progress"] =
            &RaidBlackTempleTriggerContext::black_temple_no_encounter_in_progress;

        // High Warlord Naj'entus
        creators["high warlord naj'entus hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_hunter_should_misdirect;

        creators["high warlord naj'entus should be tanked"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_should_be_tanked;

        creators["high warlord naj'entus casts needle spines"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_casts_needle_spines;

        creators["high warlord naj'entus player is impaled"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_player_is_impaled;

        creators["high warlord naj'entus has tidal shield"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_has_tidal_shield;

        // Supremus
        creators["supremus hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::supremus_hunter_should_misdirect;

        creators["supremus ranged should spread"] =
            &RaidBlackTempleTriggerContext::supremus_ranged_should_spread;

        creators["supremus fixates on bot"] =
            &RaidBlackTempleTriggerContext::supremus_fixates_on_bot;

        creators["supremus near volcano"] =
            &RaidBlackTempleTriggerContext::supremus_near_volcano;

        creators["supremus should manage phase timer"] =
            &RaidBlackTempleTriggerContext::supremus_should_manage_phase_timer;

        // Shade of Akama
        creators["shade of akama killing channelers starts phase 2"] =
            &RaidBlackTempleTriggerContext::shade_of_akama_killing_channelers_starts_phase_2;

        // Teron Gorefiend
        creators["teron gorefiend hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_hunter_should_misdirect;

        creators["teron gorefiend should be tanked"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_should_be_tanked;

        creators["teron gorefiend ranged should position on balcony"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_ranged_should_position_on_balcony;

        creators["teron gorefiend casts shadow of death"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_casts_shadow_of_death;

        creators["teron gorefiend shadow of death"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_shadow_of_death;

        creators["teron gorefiend transformed into vengeful spirit"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_transformed_into_vengeful_spirit;

        // Gurtogg Bloodboil
        creators["gurtogg bloodboil hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_hunter_should_misdirect;

        creators["gurtogg bloodboil should be tanked"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_should_be_tanked;

        creators["gurtogg bloodboil casts bloodboil"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_casts_bloodboil;

        creators["gurtogg bloodboil fel rage on group member"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_fel_rage_on_group_member;

        creators["gurtogg bloodboil should manage phase timer"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_should_manage_phase_timer;

        // Reliquary of Souls
        creators["reliquary of souls hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_hunter_should_misdirect;

        creators["reliquary of souls essence of suffering fixates on closest target"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_suffering_fixates_on_closest_target;

        creators["reliquary of souls essence of suffering disables healing"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_suffering_disables_healing;

        creators["reliquary of souls essence of desire has rune shield"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_has_rune_shield;

        creators["reliquary of souls essence of desire casts deaden"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_casts_deaden;

        // Mother Shahraz
        creators["mother shahraz hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_hunter_should_misdirect;

        creators["mother shahraz should be tanked"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_should_be_tanked;

        creators["mother shahraz tanks are positioning boss"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_tanks_are_positioning_boss;

        creators["mother shahraz ranged should position under pillar"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_ranged_should_position_under_pillar;

        creators["mother shahraz fatal attraction"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_fatal_attraction;

        // Illidari Council
        creators["illidari council hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::illidari_council_hunter_should_misdirect;

        creators["illidari council gathios should be tanked"] =
            &RaidBlackTempleTriggerContext::illidari_council_gathios_should_be_tanked;

        creators["illidari council gathios casts judgement of command"] =
            &RaidBlackTempleTriggerContext::illidari_council_gathios_casts_judgement_of_command;

        creators["illidari council malande should be tanked"] =
            &RaidBlackTempleTriggerContext::illidari_council_malande_should_be_tanked;

        creators["illidari council darkshadow should be tanked"] =
            &RaidBlackTempleTriggerContext::illidari_council_darkshadow_should_be_tanked;

        creators["illidari council zerevor should be tanked by mage"] =
            &RaidBlackTempleTriggerContext::illidari_council_zerevor_should_be_tanked_by_mage;

        creators["illidari council mage tank needs dedicated healer"] =
            &RaidBlackTempleTriggerContext::illidari_council_mage_tank_needs_dedicated_healer;

        creators["illidari council ranged should spread"] =
            &RaidBlackTempleTriggerContext::illidari_council_ranged_should_spread;

        creators["illidari council pets screw up the pull"] =
            &RaidBlackTempleTriggerContext::illidari_council_pets_screw_up_the_pull;

        creators["illidari council should assign dps priority"] =
            &RaidBlackTempleTriggerContext::illidari_council_should_assign_dps_priority;

        creators["illidari council should manage dps timer"] =
            &RaidBlackTempleTriggerContext::illidari_council_should_manage_dps_timer;

        // Illidan Stormrage <The Betrayer>
        creators["illidan stormrage hunter should misdirect"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_hunter_should_misdirect;

        creators["illidan stormrage casts flame crash"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_casts_flame_crash;

        creators["illidan stormrage parasitic shadowfiend on group member"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_parasitic_shadowfiend_on_group_member;

        creators["illidan stormrage parasitic shadowfiends run wild"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_parasitic_shadowfiends_run_wild;

        creators["illidan stormrage flames of azzinoth should be tanked"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_flames_of_azzinoth_should_be_tanked;

        creators["illidan stormrage pets die to fire"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_pets_die_to_fire;

        creators["illidan stormrage grate is safe from flames"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_grate_is_safe_from_flames;

        creators["illidan stormrage dark barrage on immunity class"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_dark_barrage_on_immunity_class;

        creators["illidan stormrage prepares to land"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_prepares_to_land;

        creators["illidan stormrage ranged should spread"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_ranged_should_spread;

        creators["illidan stormrage this expansion hates melee"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_this_expansion_hates_melee;

        creators["illidan stormrage warlock should tank demon form"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_warlock_should_tank_demon_form;

        creators["illidan stormrage should assign dps priority"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_assign_dps_priority;

        creators["illidan stormrage maiev placed shadow trap"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_maiev_placed_shadow_trap;

        creators["illidan stormrage should manage dps timer and rti"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_manage_dps_timer_and_rti;

        creators["illidan stormrage should clear hazards between phases"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_should_clear_hazards_between_phases;

        creators["illidan stormrage cheat"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_cheat;
    }

private:
    // General
    static Trigger* black_temple_no_encounter_in_progress(PlayerbotAI* botAI)
    {
        return new BlackTempleNoEncounterInProgressTrigger(botAI);
    }

    // High Warlord Naj'entus
    static Trigger* high_warlord_najentus_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "high warlord naj'entus hunter should misdirect", "high warlord naj'entus");
    }
    static Trigger* high_warlord_najentus_should_be_tanked(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusShouldBeTankedTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_casts_needle_spines(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusCastsNeedleSpinesTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_player_is_impaled(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusPlayerIsImpaledTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_has_tidal_shield(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusHasTidalShieldTrigger(botAI);
    }

    // Supremus
    static Trigger* supremus_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new SupremusHunterShouldMisdirectTrigger(botAI);
    }
    static Trigger* supremus_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new SupremusRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* supremus_fixates_on_bot(PlayerbotAI* botAI)
    {
        return new SupremusFixatesOnBotTrigger(botAI);
    }
    static Trigger* supremus_near_volcano(PlayerbotAI* botAI)
    {
        return new SupremusNearVolcanoTrigger(botAI);
    }
    static Trigger* supremus_should_manage_phase_timer(PlayerbotAI* botAI)
    {
        return new SupremusShouldManagePhaseTimerTrigger(botAI);
    }

    // Shade of Akama
    static Trigger* shade_of_akama_killing_channelers_starts_phase_2(PlayerbotAI* botAI)
    {
        return new ShadeOfAkamaKillingChannelersStartsPhase2Trigger(botAI);
    }

    // Teron Gorefiend
    static Trigger* teron_gorefiend_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "teron gorefiend hunter should misdirect", "teron gorefiend");
    }
    static Trigger* teron_gorefiend_should_be_tanked(PlayerbotAI* botAI)
    {
        return new TeronGorefiendShouldBeTankedTrigger(botAI);
    }
    static Trigger* teron_gorefiend_ranged_should_position_on_balcony(PlayerbotAI* botAI)
    {
        return new TeronGorefiendRangedShouldPositionOnBalconyTrigger(botAI);
    }
    static Trigger* teron_gorefiend_casts_shadow_of_death(PlayerbotAI* botAI)
    {
        return new TeronGorefiendCastsShadowOfDeathTrigger(botAI);
    }
    static Trigger* teron_gorefiend_shadow_of_death(PlayerbotAI* botAI)
    {
        return new TeronGorefiendShadowOfDeathTrigger(botAI);
    }
    static Trigger* teron_gorefiend_transformed_into_vengeful_spirit(PlayerbotAI* botAI)
    {
        return new TeronGorefiendTransformedIntoVengefulSpiritTrigger(botAI);
    }

    // Gurtogg Bloodboil
    static Trigger* gurtogg_bloodboil_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilHunterShouldMisdirectTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_should_be_tanked(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilShouldBeTankedTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_casts_bloodboil(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilCastsBloodboilTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_fel_rage_on_group_member(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilFelRageOnGroupMemberTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_should_manage_phase_timer(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilShouldManagePhaseTimerTrigger(botAI);
    }

    // Reliquary of Souls
    static Trigger* reliquary_of_souls_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsHunterShouldMisdirectTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_suffering_fixates_on_closest_target(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_suffering_disables_healing(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_desire_has_rune_shield(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_desire_casts_deaden(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfDesireCastsDeadenTrigger(botAI);
    }

    // Mother Shahraz
    static Trigger* mother_shahraz_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "mother shahraz hunter should misdirect", "mother shahraz");
    }
    static Trigger* mother_shahraz_should_be_tanked(PlayerbotAI* botAI)
    {
        return new MotherShahrazShouldBeTankedTrigger(botAI);
    }
    static Trigger* mother_shahraz_tanks_are_positioning_boss(PlayerbotAI* botAI)
    {
        return new MotherShahrazTanksArePositioningBossTrigger(botAI);
    }
    static Trigger* mother_shahraz_ranged_should_position_under_pillar(PlayerbotAI* botAI)
    {
        return new MotherShahrazRangedShouldPositionUnderPillarTrigger(botAI);
    }
    static Trigger* mother_shahraz_fatal_attraction(PlayerbotAI* botAI)
    {
        return new MotherShahrazFatalAttractionTrigger(botAI);
    }

    // Illidari Council
    static Trigger* illidari_council_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new BlackTempleHunterShouldMisdirectTrigger(
            botAI, "illidari council hunter should misdirect", "gathios the shatterer");
    }
    static Trigger* illidari_council_gathios_should_be_tanked(PlayerbotAI* botAI)
    {
        return new IllidariCouncilGathiosShouldBeTankedTrigger(botAI);
    }
    static Trigger* illidari_council_gathios_casts_judgement_of_command(PlayerbotAI* botAI)
    {
        return new IllidariCouncilGathiosCastsJudgementOfCommandTrigger(botAI);
    }
    static Trigger* illidari_council_malande_should_be_tanked(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMalandeShouldBeTankedTrigger(botAI);
    }
    static Trigger* illidari_council_darkshadow_should_be_tanked(PlayerbotAI* botAI)
    {
        return new IllidariCouncilDarkshadowShouldBeTankedTrigger(botAI);
    }
    static Trigger* illidari_council_zerevor_should_be_tanked_by_mage(PlayerbotAI* botAI)
    {
        return new IllidariCouncilZerevorShouldBeTankedByMageTrigger(botAI);
    }
    static Trigger* illidari_council_mage_tank_needs_dedicated_healer(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMageTankNeedsDedicatedHealerTrigger(botAI);
    }
    static Trigger* illidari_council_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new IllidariCouncilRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* illidari_council_pets_screw_up_the_pull(PlayerbotAI* botAI)
    {
        return new IllidariCouncilPetsScrewUpThePullTrigger(botAI);
    }
    static Trigger* illidari_council_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new IllidariCouncilShouldAssignDpsPriorityTrigger(botAI);
    }
    static Trigger* illidari_council_should_manage_dps_timer(PlayerbotAI* botAI)
    {
        return new IllidariCouncilShouldManageDpsTimerTrigger(botAI);
    }

    // Illidan Stormrage <The Betrayer>
    static Trigger* illidan_stormrage_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new IllidanStormrageHunterShouldMisdirectTrigger(botAI);
    }
    static Trigger* illidan_stormrage_casts_flame_crash(PlayerbotAI* botAI)
    {
        return new IllidanStormrageCastsFlameCrashTrigger(botAI);
    }
    static Trigger* illidan_stormrage_parasitic_shadowfiend_on_group_member(PlayerbotAI* botAI)
    {
        return new IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger(botAI);
    }
    static Trigger* illidan_stormrage_parasitic_shadowfiends_run_wild(PlayerbotAI* botAI)
    {
        return new IllidanStormrageParasiticShadowfiendsRunWildTrigger(botAI);
    }
    static Trigger* illidan_stormrage_flames_of_azzinoth_should_be_tanked(PlayerbotAI* botAI)
    {
        return new IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger(botAI);
    }
    static Trigger* illidan_stormrage_pets_die_to_fire(PlayerbotAI* botAI)
    {
        return new IllidanStormragePetsDieToFireTrigger(botAI);
    }
    static Trigger* illidan_stormrage_grate_is_safe_from_flames(PlayerbotAI* botAI)
    {
        return new IllidanStormrageGrateIsSafeFromFlamesTrigger(botAI);
    }
    static Trigger* illidan_stormrage_dark_barrage_on_immunity_class(PlayerbotAI* botAI)
    {
        return new IllidanStormrageDarkBarrageOnImmunityClassTrigger(botAI);
    }
    static Trigger* illidan_stormrage_prepares_to_land(PlayerbotAI* botAI)
    {
        return new IllidanStormragePreparesToLandTrigger(botAI);
    }
    static Trigger* illidan_stormrage_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new IllidanStormrageRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* illidan_stormrage_this_expansion_hates_melee(PlayerbotAI* botAI)
    {
        return new IllidanStormrageThisExpansionHatesMeleeTrigger(botAI);
    }
    static Trigger* illidan_stormrage_warlock_should_tank_demon_form(PlayerbotAI* botAI)
    {
        return new IllidanStormrageWarlockShouldTankDemonFormTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldAssignDpsPriorityTrigger(botAI);
    }
    static Trigger* illidan_stormrage_maiev_placed_shadow_trap(PlayerbotAI* botAI)
    {
        return new IllidanStormrageMaievPlacedShadowTrapTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_manage_dps_timer_and_rti(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldManageDpsTimerAndRtiTrigger(botAI);
    }
    static Trigger* illidan_stormrage_should_clear_hazards_between_phases(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShouldClearHazardsBetweenPhasesTrigger(botAI);
    }
    static Trigger* illidan_stormrage_cheat(PlayerbotAI* botAI)
    {
        return new IllidanStormrageCheatTrigger(botAI);
    }
};

#endif
