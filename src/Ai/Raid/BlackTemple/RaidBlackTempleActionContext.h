#ifndef _PLAYERBOT_RAIDBLACKTEMPLEACTIONCONTEXT_H
#define _PLAYERBOT_RAIDBLACKTEMPLEACTIONCONTEXT_H

#include "RaidBlackTempleActions.h"
#include "NamedObjectContext.h"

class RaidBlackTempleActionContext : public NamedObjectContext<Action>
{
public:
    RaidBlackTempleActionContext()
    {
        // General
        creators["black temple erase timers and trackers"] =
            &RaidBlackTempleActionContext::black_temple_erase_timers_and_trackers;

        // High Warlord Naj'entus
        creators["high warlord naj'entus misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_misdirect_boss_to_main_tank;

        creators["high warlord naj'entus tanks position boss"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_tanks_position_boss;

        creators["high warlord naj'entus disperse ranged"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_disperse_ranged;

        creators["high warlord naj'entus remove impaling spine"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_remove_impaling_spine;

        creators["high warlord naj'entus throw impaling spine"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_throw_impaling_spine;

        // Supremus
        creators["supremus misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::supremus_misdirect_boss_to_main_tank;

        creators["supremus disperse ranged"] =
            &RaidBlackTempleActionContext::supremus_disperse_ranged;

        creators["supremus kite boss"] =
            &RaidBlackTempleActionContext::supremus_kite_boss;

        creators["supremus move away from volcanos"] =
            &RaidBlackTempleActionContext::supremus_move_away_from_volcanos;

        creators["supremus manage phase timer"] =
            &RaidBlackTempleActionContext::supremus_manage_phase_timer;

        // Teron Gorefiend
        creators["teron gorefiend misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::teron_gorefiend_misdirect_boss_to_main_tank;

        creators["teron gorefiend tanks position boss"] =
            &RaidBlackTempleActionContext::teron_gorefiend_tanks_position_boss;

        creators["teron gorefiend position ranged on balcony"] =
            &RaidBlackTempleActionContext::teron_gorefiend_position_ranged_on_balcony;

        creators["teron gorefiend avoid shadow of death"] =
            &RaidBlackTempleActionContext::teron_gorefiend_avoid_shadow_of_death;

        creators["teron gorefiend move to corner to die"] =
            &RaidBlackTempleActionContext::teron_gorefiend_move_to_corner_to_die;

        creators["teron gorefiend control and destroy shadowy constructs"] =
            &RaidBlackTempleActionContext::teron_gorefiend_control_and_destroy_shadowy_constructs;

        // Gurtogg Bloodboil
        creators["gurtogg bloodboil misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_misdirect_boss_to_main_tank;

        creators["gurtogg bloodboil tanks position boss"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_tanks_position_boss;

        creators["gurtogg bloodboil rotate ranged groups"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_rotate_ranged_groups;

        creators["gurtogg bloodboil ranged move away from enraged player"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_ranged_move_away_from_enraged_player;

        creators["gurtogg bloodboil manage phase timer"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_manage_phase_timer;

        // Reliquary of Souls
        creators["reliquary of souls misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_misdirect_boss_to_main_tank;

        creators["reliquary of souls adjust distance from essence of suffering"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_adjust_distance_from_essence_of_suffering;

        creators["reliquary of souls healers dps essence of suffering"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_healers_dps_essence_of_suffering;

        creators["reliquary of souls spellsteal rune shield"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_spellsteal_rune_shield;

        creators["reliquary of souls spell reflect deaden"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_spell_reflect_deaden;

        // Mother Shahraz
        creators["mother shahraz misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::mother_shahraz_misdirect_boss_to_main_tank;

        creators["mother shahraz tanks position boss under pillar"] =
            &RaidBlackTempleActionContext::mother_shahraz_tanks_position_boss_under_pillar;

        creators["mother shahraz position ranged under pillar"] =
            &RaidBlackTempleActionContext::mother_shahraz_position_ranged_under_pillar;

        creators["mother shahraz run away to break fatal attraction"] =
            &RaidBlackTempleActionContext::mother_shahraz_run_away_to_break_fatal_attraction;

        // Illidari Council
        creators["illidari council misdirect bosses to tanks"] =
            &RaidBlackTempleActionContext::illidari_council_misdirect_bosses_to_tanks;

        creators["illidari council main tank position gathios"] =
            &RaidBlackTempleActionContext::illidari_council_main_tank_position_gathios;

        creators["illidari council main tank reflect judgement of command"] =
            &RaidBlackTempleActionContext::illidari_council_main_tank_reflect_judgement_of_command;

        creators["illidari council first assist tank position malande"] =
            &RaidBlackTempleActionContext::illidari_council_first_assist_tank_position_malande;

        creators["illidari council second assist tank position darkshadow"] =
            &RaidBlackTempleActionContext::illidari_council_second_assist_tank_position_darkshadow;

        creators["illidari council mage tank position zerevor"] =
            &RaidBlackTempleActionContext::illidari_council_mage_tank_position_zerevor;

        creators["illidari council position mage tank healer"] =
            &RaidBlackTempleActionContext::illidari_council_position_mage_tank_healer;

        creators["illidari council assign dps targets"] =
            &RaidBlackTempleActionContext::illidari_council_assign_dps_targets;

        creators["illidari council command pets to attack gathios"] =
            &RaidBlackTempleActionContext::illidari_council_command_pets_to_attack_gathios;

        creators["illidari council manage dps timer"] =
            &RaidBlackTempleActionContext::illidari_council_manage_dps_timer;

        // Illidan Stormrage <The Betrayer>
        creators["illidan stormrage misdirect to tank"] =
            &RaidBlackTempleActionContext::illidan_stormrage_misdirect_to_tank;

        creators["illidan stormrage main tank move away from flame crash"] =
            &RaidBlackTempleActionContext::illidan_stormrage_main_tank_move_away_from_flame_crash;

        creators["illidan stormrage isolate bot with parasite"] =
            &RaidBlackTempleActionContext::illidan_stormrage_isolate_bot_with_parasite;

        creators["illidan stormrage assist tanks handle flames of azzinoth"] =
            &RaidBlackTempleActionContext::illidan_stormrage_assist_tanks_handle_flames_of_azzinoth;

        creators["illidan stormrage control pet aggression"] =
            &RaidBlackTempleActionContext::illidan_stormrage_control_pet_aggression;

        creators["illidan stormrage position above grate"] =
            &RaidBlackTempleActionContext::illidan_stormrage_position_above_grate;

        creators["illidan stormrage remove dark barrage"] =
            &RaidBlackTempleActionContext::illidan_stormrage_remove_dark_barrage;

        creators["illidan stormrage move away from landing point"] =
            &RaidBlackTempleActionContext::illidan_stormrage_move_away_from_landing_point;

        creators["illidan stormrage disperse ranged"] =
            &RaidBlackTempleActionContext::illidan_stormrage_disperse_ranged;

        creators["illidan stormrage melee go somewhere to not die"] =
            &RaidBlackTempleActionContext::illidan_stormrage_melee_go_somewhere_to_not_die;

        creators["illidan stormrage warlock tank handle demon boss"] =
            &RaidBlackTempleActionContext::illidan_stormrage_warlock_tank_handle_demon_boss;

        creators["illidan stormrage dps prioritize adds"] =
            &RaidBlackTempleActionContext::illidan_stormrage_dps_prioritize_adds;

        creators["illidan stormrage manage dps timer"] =
            &RaidBlackTempleActionContext::illidan_stormrage_manage_dps_timer;

        creators["illidan stormrage destroy hazards cheat"] =
            &RaidBlackTempleActionContext::illidan_stormrage_destroy_hazards_cheat;
    }

private:
    // General
    static Action* black_temple_erase_timers_and_trackers(
        PlayerbotAI* botAI) { return new BlackTempleEraseTimersAndTrackersAction(botAI); }

    // High Warlord Naj'entus
    static Action* high_warlord_najentus_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new HighWarlordNajentusMisdirectBossToMainTankAction(botAI); }

    static Action* high_warlord_najentus_tanks_position_boss(
        PlayerbotAI* botAI) { return new HighWarlordNajentusTanksPositionBossAction(botAI); }

    static Action* high_warlord_najentus_disperse_ranged(
        PlayerbotAI* botAI) { return new HighWarlordNajentusDisperseRangedAction(botAI); }

    static Action* high_warlord_najentus_remove_impaling_spine(
        PlayerbotAI* botAI) { return new HighWarlordNajentusRemoveImpalingSpineAction(botAI); }

    static Action* high_warlord_najentus_throw_impaling_spine(
        PlayerbotAI* botAI) { return new HighWarlordNajentusThrowImpalingSpineAction(botAI); }

    // Supremus
    static Action* supremus_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new SupremusMisdirectBossToMainTankAction(botAI); }

    static Action* supremus_disperse_ranged(
        PlayerbotAI* botAI) { return new SupremusDisperseRangedAction(botAI); }

    static Action* supremus_kite_boss(
        PlayerbotAI* botAI) { return new SupremusKiteBossAction(botAI); }

    static Action* supremus_move_away_from_volcanos(
        PlayerbotAI* botAI) { return new SupremusMoveAwayFromVolcanosAction(botAI); }

    static Action* supremus_manage_phase_timer(
        PlayerbotAI* botAI) { return new SupremusManagePhaseTimerAction(botAI); }

    // Teron Gorefiend
    static Action* teron_gorefiend_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new TeronGorefiendMisdirectBossToMainTankAction(botAI); }

    static Action* teron_gorefiend_tanks_position_boss(
        PlayerbotAI* botAI) { return new TeronGorefiendTanksPositionBossAction(botAI); }

    static Action* teron_gorefiend_position_ranged_on_balcony(
        PlayerbotAI* botAI) { return new TeronGorefiendPositionRangedOnBalconyAction(botAI); }

    static Action* teron_gorefiend_avoid_shadow_of_death(
        PlayerbotAI* botAI) { return new TeronGorefiendAvoidShadowOfDeathAction(botAI); }

    static Action* teron_gorefiend_move_to_corner_to_die(
        PlayerbotAI* botAI) { return new TeronGorefiendMoveToCornerToDieAction(botAI); }

    static Action* teron_gorefiend_control_and_destroy_shadowy_constructs(
        PlayerbotAI* botAI) { return new TeronGorefiendControlAndDestroyShadowyConstructsAction(botAI); }

    // Gurtogg Bloodboil
    static Action* gurtogg_bloodboil_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new GurtoggBloodboilMisdirectBossToMainTankAction(botAI); }

    static Action* gurtogg_bloodboil_tanks_position_boss(
        PlayerbotAI* botAI) { return new GurtoggBloodboilTanksPositionBossAction(botAI); }

    static Action* gurtogg_bloodboil_rotate_ranged_groups(
        PlayerbotAI* botAI) { return new GurtoggBloodboilRotateRangedGroupsAction(botAI); }

    static Action* gurtogg_bloodboil_ranged_move_away_from_enraged_player(
        PlayerbotAI* botAI) { return new GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction(botAI); }

    static Action* gurtogg_bloodboil_manage_phase_timer(
        PlayerbotAI* botAI) { return new GurtoggBloodboilManagePhaseTimerAction(botAI); }

    // Reliquary of Souls
    static Action* reliquary_of_souls_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsMisdirectBossToMainTankAction(botAI); }

    static Action* reliquary_of_souls_adjust_distance_from_essence_of_suffering(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsAdjustDistanceFromEssenceOfSufferingAction(botAI); }

    static Action* reliquary_of_souls_healers_dps_essence_of_suffering(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsHealersDpsEssenceOfSufferingAction(botAI); }

    static Action* reliquary_of_souls_spellsteal_rune_shield(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsSpellstealRuneShieldAction(botAI); }

    static Action* reliquary_of_souls_spell_reflect_deaden(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsSpellReflectDeadenAction(botAI); }

    // Mother Shahraz
    static Action* mother_shahraz_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new MotherShahrazMisdirectBossToMainTankAction(botAI); }

    static Action* mother_shahraz_tanks_position_boss_under_pillar(
        PlayerbotAI* botAI) { return new MotherShahrazTanksPositionBossUnderPillarAction(botAI); }

    static Action* mother_shahraz_position_ranged_under_pillar(
        PlayerbotAI* botAI) { return new MotherShahrazPositionRangedUnderPillarAction(botAI); }

    static Action* mother_shahraz_run_away_to_break_fatal_attraction(
        PlayerbotAI* botAI) { return new MotherShahrazRunAwayToBreakFatalAttractionAction(botAI); }

    // Illidari Council
    static Action* illidari_council_misdirect_bosses_to_tanks(
        PlayerbotAI* botAI) { return new IllidariCouncilMisdirectBossesToTanksAction(botAI); }

    static Action* illidari_council_main_tank_position_gathios(
        PlayerbotAI* botAI) { return new IllidariCouncilMainTankPositionGathiosAction(botAI); }

    static Action* illidari_council_main_tank_reflect_judgement_of_command(
        PlayerbotAI* botAI) { return new IllidariCouncilMainTankReflectJudgementOfCommandAction(botAI); }

    static Action* illidari_council_first_assist_tank_position_malande(
        PlayerbotAI* botAI) { return new IllidariCouncilFirstAssistTankPositionMalandeAction(botAI); }

    static Action* illidari_council_second_assist_tank_position_darkshadow(
        PlayerbotAI* botAI) { return new IllidariCouncilSecondAssistTankPositionDarkshadowAction(botAI); }

    static Action* illidari_council_mage_tank_position_zerevor(
        PlayerbotAI* botAI) { return new IllidariCouncilMageTankPositionZerevorAction(botAI); }

    static Action* illidari_council_position_mage_tank_healer(
        PlayerbotAI* botAI) { return new IllidariCouncilPositionMageTankHealerAction(botAI); }

    static Action* illidari_council_assign_dps_targets(
        PlayerbotAI* botAI) { return new IllidariCouncilAssignDpsTargetsAction(botAI); }

    static Action* illidari_council_command_pets_to_attack_gathios(
        PlayerbotAI* botAI) { return new IllidariCouncilCommandPetsToAttackGathiosAction(botAI); }

    static Action* illidari_council_manage_dps_timer(
        PlayerbotAI* botAI) { return new IllidariCouncilManageDpsTimerAction(botAI); }

    // Illidan Stormrage <The Betrayer>
    static Action* illidan_stormrage_misdirect_to_tank(
        PlayerbotAI* botAI) { return new IllidanStormrageMisdirectToTankAction(botAI); }

    static Action* illidan_stormrage_main_tank_move_away_from_flame_crash(
        PlayerbotAI* botAI) { return new IllidanStormrageMainTankMoveAwayFromFlameCrashAction(botAI); }

    static Action* illidan_stormrage_isolate_bot_with_parasite(
        PlayerbotAI* botAI) { return new IllidanStormrageIsolateBotWithParasiteAction(botAI); }

    static Action* illidan_stormrage_assist_tanks_handle_flames_of_azzinoth(
        PlayerbotAI* botAI) { return new IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction(botAI); }

    static Action* illidan_stormrage_control_pet_aggression(
        PlayerbotAI* botAI) { return new IllidanStormrageControlPetAggressionAction(botAI); }

    static Action* illidan_stormrage_position_above_grate(
        PlayerbotAI* botAI) { return new IllidanStormragePositionAboveGrateAction(botAI); }

    static Action* illidan_stormrage_remove_dark_barrage(
        PlayerbotAI* botAI) { return new IllidanStormrageRemoveDarkBarrageAction(botAI); }

    static Action* illidan_stormrage_move_away_from_landing_point(
        PlayerbotAI* botAI) { return new IllidanStormrageMoveAwayFromLandingPointAction(botAI); }

    static Action* illidan_stormrage_disperse_ranged(
        PlayerbotAI* botAI) { return new IllidanStormrageDisperseRangedAction(botAI); }

    static Action* illidan_stormrage_melee_go_somewhere_to_not_die(
        PlayerbotAI* botAI) { return new IllidanStormrageMeleeGoSomewhereToNotDieAction(botAI); }

    static Action* illidan_stormrage_warlock_tank_handle_demon_boss(
        PlayerbotAI* botAI) { return new IllidanStormrageWarlockTankHandleDemonBossAction(botAI); }

    static Action* illidan_stormrage_dps_prioritize_adds(
        PlayerbotAI* botAI) { return new IllidanStormrageDpsPrioritizeAddsAction(botAI); }

    static Action* illidan_stormrage_manage_dps_timer(
        PlayerbotAI* botAI) { return new IllidanStormrageManageDpsTimerAction(botAI); }

    static Action* illidan_stormrage_destroy_hazards_cheat(
        PlayerbotAI* botAI) { return new IllidanStormrageDestroyHazardsCheatAction(botAI); }
};

#endif
