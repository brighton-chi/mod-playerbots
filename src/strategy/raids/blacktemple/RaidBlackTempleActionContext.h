#ifndef _PLAYERBOT_RAIDBLACKTEMPLEACTIONCONTEXT_H
#define _PLAYERBOT_RAIDBLACKTEMPLEACTIONCONTEXT_H

#include "RaidBlackTempleActions.h"
#include "NamedObjectContext.h"

class RaidBlackTempleActionContext : public NamedObjectContext<Action>
{
public:
    RaidBlackTempleActionContext()
    {
        // High Warlord Naj'entus

        creators["high warlord naj'entus misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_misdirect_boss_to_main_tank;

        creators["high warlord naj'entus main tank position boss"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_main_tank_position_boss;

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

        // Shade of Akama
        // N/A

        // Teron Gorefiend

        creators["teron gorefiend misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::teron_gorefiend_misdirect_boss_to_main_tank;

        creators["teron gorefiend main tank position boss"] =
            &RaidBlackTempleActionContext::teron_gorefiend_main_tank_position_boss;

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

        creators["gurtogg bloodboil disperse ranged"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_disperse_ranged;

        creators["gurtogg bloodboil ranged move to absorb bloodboil position"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_ranged_move_to_absorb_bloodboil_position;

        creators["gurtogg bloodboil fel raged bot move to tank position"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_fel_raged_bot_move_to_tank_position;

        creators["gurtogg bloodboil manage phase timer"] =
            &RaidBlackTempleActionContext::gurtogg_bloodboil_manage_phase_timer;

        // Reliquary of Souls

        creators["reliquary of souls misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_misdirect_boss_to_main_tank;

        creators["reliquary of souls melee dps stay at max range from essence of suffering"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_melee_dps_stay_at_max_range_from_essence_of_suffering;

        creators["reliquary of souls healers dps essence of suffering"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_healers_dps_essence_of_suffering;

        creators["reliquary of souls spellsteal rune shield"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_spellsteal_rune_shield;

        creators["reliquary of souls spell reflect deaden"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_spell_reflect_deaden;

        creators["reliquary of souls manage dps timer"] =
            &RaidBlackTempleActionContext::reliquary_of_souls_manage_dps_timer;

        // Mother Shahraz

        creators["mother shahraz misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::mother_shahraz_misdirect_boss_to_main_tank;

        creators["mother shahraz tanks position boss"] =
            &RaidBlackTempleActionContext::mother_shahraz_tanks_position_boss;

        creators["mother shahraz position ranged under statue"] =
            &RaidBlackTempleActionContext::mother_shahraz_position_ranged_under_statue;

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

        creators["illidari council assign dps targets"] =
            &RaidBlackTempleActionContext::illidari_council_assign_dps_targets;

        creators["illidari council manage dps timer"] =
            &RaidBlackTempleActionContext::illidari_council_manage_dps_timer;

        // Illidan Stormrage <The Betrayer>

        creators["illidan stormrage misdirect to tank"] =
            &RaidBlackTempleActionContext::illidan_stormrage_misdirect_to_tank;

        creators["illidan stormrage main tank move away from flame crash"] =
            &RaidBlackTempleActionContext::illidan_stormrage_main_tank_move_away_from_flame_crash;

        creators["illidan stormrage assist tanks handle flames of azzinoth"] =
            &RaidBlackTempleActionContext::illidan_stormrage_assist_tanks_handle_flames_of_azzinoth;

        creators["illidan stormrage ranged spread above grate"] =
            &RaidBlackTempleActionContext::illidan_stormrage_ranged_spread_above_grate;

        creators["illidan stormrage disperse ranged"] =
            &RaidBlackTempleActionContext::illidan_stormrage_disperse_ranged;

        creators["illidan stormrage position melee"] =
            &RaidBlackTempleActionContext::illidan_stormrage_position_melee;

        creators["illidan stormrage warlock tank handle demon boss"] =
            &RaidBlackTempleActionContext::illidan_stormrage_warlock_tank_handle_demon_boss;

        creators["illidan stormrage dps prioritize adds"] =
            &RaidBlackTempleActionContext::illidan_stormrage_dps_prioritize_adds;
    }

private:
    // High Warlord Naj'entus

    static Action* high_warlord_najentus_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new HighWarlordNajentusMisdirectBossToMainTankAction(botAI); }

    static Action* high_warlord_najentus_main_tank_position_boss(
        PlayerbotAI* botAI) { return new HighWarlordNajentusMainTankPositionBossAction(botAI); }

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

    // Shade of Akama
    // N/A

    // Teron Gorefiend

    static Action* teron_gorefiend_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new TeronGorefiendMisdirectBossToMainTankAction(botAI); }

    static Action* teron_gorefiend_main_tank_position_boss(
        PlayerbotAI* botAI) { return new TeronGorefiendMainTankPositionBossAction(botAI); }

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

    static Action* gurtogg_bloodboil_disperse_ranged(
        PlayerbotAI* botAI) { return new GurtoggBloodboilDisperseRangedAction(botAI); }

    static Action* gurtogg_bloodboil_ranged_move_to_absorb_bloodboil_position(
        PlayerbotAI* botAI) { return new GurtoggBloodboilRangedMoveToAbsorbBloodboilPositionAction(botAI); }

    static Action* gurtogg_bloodboil_fel_raged_bot_move_to_tank_position(
        PlayerbotAI* botAI) { return new GurtoggBloodboilFelRagedBotMoveToTankPositionAction(botAI); }

    static Action* gurtogg_bloodboil_manage_phase_timer(
        PlayerbotAI* botAI) { return new GurtoggBloodboilManagePhaseTimerAction(botAI); }

    // Reliquary of Souls

    static Action* reliquary_of_souls_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsMisdirectBossToMainTankAction(botAI); }

    static Action* reliquary_of_souls_melee_dps_stay_at_max_range_from_essence_of_suffering(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsMeleeDpsStayAtMaxRangeFromEssenceOfSufferingAction(botAI); }

    static Action* reliquary_of_souls_healers_dps_essence_of_suffering(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsHealersDpsEssenceOfSufferingAction(botAI); }

    static Action* reliquary_of_souls_spellsteal_rune_shield(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsSpellstealRuneShieldAction(botAI); }

    static Action* reliquary_of_souls_spell_reflect_deaden(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsSpellReflectDeadenAction(botAI); }

    static Action* reliquary_of_souls_manage_dps_timer(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsManageDpsTimerAction(botAI); }

    // Mother Shahraz

    static Action* mother_shahraz_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new MotherShahrazMisdirectBossToMainTankAction(botAI); }

    static Action* mother_shahraz_tanks_position_boss(
        PlayerbotAI* botAI) { return new MotherShahrazTanksPositionBossAction(botAI); }

    static Action* mother_shahraz_position_ranged_under_statue(
        PlayerbotAI* botAI) { return new MotherShahrazPositionRangedUnderStatueAction(botAI); }

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

    static Action* illidari_council_assign_dps_targets(
        PlayerbotAI* botAI) { return new IllidariCouncilAssignDpsTargetsAction(botAI); }

    static Action* illidari_council_manage_dps_timer(
        PlayerbotAI* botAI) { return new IllidariCouncilManageDpsTimerAction(botAI); }

    // Illidan Stormrage <The Betrayer>

    static Action* illidan_stormrage_misdirect_to_tank(
        PlayerbotAI* botAI) { return new IllidanStormrageMisdirectToTankAction(botAI); }

    static Action* illidan_stormrage_main_tank_move_away_from_flame_crash(
        PlayerbotAI* botAI) { return new IllidanStormrageMainTankMoveAwayFromFlameCrashAction(botAI); }

    static Action* illidan_stormrage_assist_tanks_handle_flames_of_azzinoth(
        PlayerbotAI* botAI) { return new IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction(botAI); }

    static Action* illidan_stormrage_ranged_spread_above_grate(
        PlayerbotAI* botAI) { return new IllidanStormrageRangedSpreadAboveGrateAction(botAI); }

    static Action* illidan_stormrage_disperse_ranged(
        PlayerbotAI* botAI) { return new IllidanStormrageDisperseRangedAction(botAI); }

    static Action* illidan_stormrage_position_melee(
        PlayerbotAI* botAI) { return new IllidanStormragePositionMeleeAction(botAI); }

    static Action* illidan_stormrage_warlock_tank_handle_demon_boss(
        PlayerbotAI* botAI) { return new IllidanStormrageWarlockTankHandleDemonBossAction(botAI); }

    static Action* illidan_stormrage_dps_prioritize_adds(
        PlayerbotAI* botAI) { return new IllidanStormrageDpsPrioritizeAddsAction(botAI); }
};

#endif
