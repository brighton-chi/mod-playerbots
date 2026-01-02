#ifndef _PLAYERBOT_RAIDBLACKTEMPLETRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDBLACKTEMPLETRIGGERCONTEXT_H

#include "RaidBlackTempleTriggers.h"
#include "AiObjectContext.h"

class RaidBlackTempleTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidBlackTempleTriggerContext()
    {
        // High Warlord Naj'entus

        creators["high warlord naj'entus pulling boss"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_pulling_boss;

        creators["high warlord naj'entus boss engaged by main tank"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_boss_engaged_by_main_tank;

        creators["high warlord naj'entus casts needle spines"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_casts_needle_spines;

        creators["high warlord naj'entus player is impaled"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_player_is_impaled;

        creators["high warlord naj'entus boss has tidal shield"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_boss_has_tidal_shield;

        // Supremus

        creators["supremus pulling boss or changing phase"] =
            &RaidBlackTempleTriggerContext::supremus_pulling_boss_or_changing_phase;

        creators["supremus boss engaged by ranged"] =
            &RaidBlackTempleTriggerContext::supremus_boss_engaged_by_ranged;

        creators["supremus boss is fixated on bot"] =
            &RaidBlackTempleTriggerContext::supremus_boss_is_fixated_on_bot;

        creators["supremus volcano is nearby"] =
            &RaidBlackTempleTriggerContext::supremus_volcano_is_nearby;

        creators["supremus need to manage phase timer"] =
            &RaidBlackTempleTriggerContext::supremus_need_to_manage_phase_timer;

        // Shade of Akama
        // N/A

        // Teron Gorefiend

        creators["teron gorefiend pulling boss"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_pulling_boss;

        creators["teron gorefiend boss engaged"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_boss_engaged;

        creators["teron gorefiend boss is casting shadow of death"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_boss_is_casting_shadow_of_death;

        creators["teron gorefiend bot has shadow of death"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_bot_has_shadow_of_death;

        creators["teron gorefiend bot transformed into vengeful spirit"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_bot_transformed_into_vengeful_spirit;

        // Gurtogg Bloodboil

        creators["gurtogg bloodboil pulling boss"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_pulling_boss;

        creators["gurtogg bloodboil boss engaged by tanks"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_boss_engaged_by_tanks;

        creators["gurtogg bloodboil boss casts aoe spells"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_boss_casts_aoe_spells;

        creators["gurtogg bloodboil boss casts bloodboil on five farthest players"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_boss_casts_bloodboil_on_five_farthest_players;

        creators["gurtogg bloodboil bot has fel rage"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_bot_has_fel_rage;

        creators["gurtogg bloodboil need to manage phase timer"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_need_to_manage_phase_timer;

        // Reliquary of Souls

        creators["reliquary of souls aggro resets upon phase change"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_aggro_resets_upon_phase_change;

        creators["reliquary of souls essence of suffering fixates on closest target"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_suffering_fixates_on_closest_target;

        creators["reliquary of souls essence of suffering disables healing"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_suffering_disables_healing;

        creators["reliquary of souls essence of desire has rune shield"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_has_rune_shield;

        creators["reliquary of souls essence of desire casting deaden"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_casting_deaden;

        creators["reliquary of souls need to manage dps timer"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_need_to_manage_dps_timer;

        // Mother Shahraz

        creators["mother shahraz pulling boss"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_pulling_boss;

        creators["mother shahraz boss engaged by tanks"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_boss_engaged_by_tanks;

        creators["mother shahraz sinister beam knocks back players"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_sinister_beam_knocks_back_players;

        creators["mother shahraz bots are linked by fatal attraction"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_bots_are_linked_by_fatal_attraction;

        // Illidari Council

        creators["illidari council pulling bosses"] =
            &RaidBlackTempleTriggerContext::illidari_council_pulling_bosses;

        // Illidan Stormrage <The Betrayer>
    }

private:
    // High Warlord Naj'entus

    static Trigger* high_warlord_najentus_pulling_boss(
        PlayerbotAI* botAI) { return new HighWarlordNajentusPullingBossTrigger(botAI); }

    static Trigger* high_warlord_najentus_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new HighWarlordNajentusBossEngagedByMainTankTrigger(botAI); }

    static Trigger* high_warlord_najentus_casts_needle_spines(
        PlayerbotAI* botAI) { return new HighWarlordNajentusCastsNeedleSpinesTrigger(botAI); }

    static Trigger* high_warlord_najentus_player_is_impaled(
        PlayerbotAI* botAI) { return new HighWarlordNajentusPlayerIsImpaledTrigger(botAI); }

    static Trigger* high_warlord_najentus_boss_has_tidal_shield(
        PlayerbotAI* botAI) { return new HighWarlordNajentusBossHasTidalShieldTrigger(botAI); }

    // Supremus

    static Trigger* supremus_pulling_boss_or_changing_phase(
        PlayerbotAI* botAI) { return new SupremusPullingBossOrChangingPhaseTrigger(botAI); }

    static Trigger* supremus_boss_engaged_by_ranged(
        PlayerbotAI* botAI) { return new SupremusBossEngagedByRangedTrigger(botAI); }

    static Trigger* supremus_boss_is_fixated_on_bot(
        PlayerbotAI* botAI) { return new SupremusBossIsFixatedOnBotTrigger(botAI); }

    static Trigger* supremus_volcano_is_nearby(
        PlayerbotAI* botAI) { return new SupremusVolcanoIsNearbyTrigger(botAI); }

    static Trigger* supremus_need_to_manage_phase_timer(
        PlayerbotAI* botAI) { return new SupremusNeedToManagePhaseTimerTrigger(botAI); }

    // Shade of Akama
    // N/A

    // Teron Gorefiend

    static Trigger* teron_gorefiend_pulling_boss(
        PlayerbotAI* botAI) { return new TeronGorefiendPullingBossTrigger(botAI); }

    static Trigger* teron_gorefiend_boss_engaged(
        PlayerbotAI* botAI) { return new TeronGorefiendBossEngagedTrigger(botAI); }

    static Trigger* teron_gorefiend_boss_is_casting_shadow_of_death(
        PlayerbotAI* botAI) { return new TeronGorefiendBossIsCastingShadowOfDeathTrigger(botAI); }

    static Trigger* teron_gorefiend_bot_has_shadow_of_death(
        PlayerbotAI* botAI) { return new TeronGorefiendBotHasShadowOfDeathTrigger(botAI); }

    static Trigger* teron_gorefiend_bot_transformed_into_vengeful_spirit(
        PlayerbotAI* botAI) { return new TeronGorefiendBotTransformedIntoVengefulSpiritTrigger(botAI); }

    // Gurtogg Bloodboil

    static Trigger* gurtogg_bloodboil_pulling_boss(
        PlayerbotAI* botAI) { return new GurtoggBloodboilPullingBossTrigger(botAI); }

    static Trigger* gurtogg_bloodboil_boss_engaged_by_tanks(
        PlayerbotAI* botAI) { return new GurtoggBloodboilBossEngagedByTanksTrigger(botAI); }

    static Trigger* gurtogg_bloodboil_boss_casts_aoe_spells(
        PlayerbotAI* botAI) { return new GurtoggBloodboilBossCastsAoeSpellsTrigger(botAI); }

    static Trigger* gurtogg_bloodboil_boss_casts_bloodboil_on_five_farthest_players(
        PlayerbotAI* botAI) { return new GurtoggBloodboilBossCastsBloodboilOnFiveFarthestPlayersTrigger(botAI); }

    static Trigger* gurtogg_bloodboil_bot_has_fel_rage(
        PlayerbotAI* botAI) { return new GurtoggBloodboilBotHasFelRageTrigger(botAI); }

    static Trigger* gurtogg_bloodboil_need_to_manage_phase_timer(
        PlayerbotAI* botAI) { return new GurtoggBloodboilNeedToManagePhaseTimerTrigger(botAI); }

    // Reliquary of Souls

    static Trigger* reliquary_of_souls_aggro_resets_upon_phase_change(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger(botAI); }

    static Trigger* reliquary_of_souls_essence_of_suffering_fixates_on_closest_target(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger(botAI); }

    static Trigger* reliquary_of_souls_essence_of_suffering_disables_healing(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger(botAI); }

    static Trigger* reliquary_of_souls_essence_of_desire_has_rune_shield(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(botAI); }

    static Trigger* reliquary_of_souls_essence_of_desire_casting_deaden(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger(botAI); }

    static Trigger* reliquary_of_souls_need_to_manage_dps_timer(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsNeedToManageDpsTimerTrigger(botAI); }

    // Mother Shahraz

    static Trigger* mother_shahraz_pulling_boss(
        PlayerbotAI* botAI) { return new MotherShahrazPullingBossTrigger(botAI); }

    static Trigger* mother_shahraz_boss_engaged_by_tanks(
        PlayerbotAI* botAI) { return new MotherShahrazBossEngagedByTanksTrigger(botAI); }

    static Trigger* mother_shahraz_sinister_beam_knocks_back_players(
        PlayerbotAI* botAI) { return new MotherShahrazSinisterBeamKnocksBackPlayersTrigger(botAI); }

    static Trigger* mother_shahraz_bots_are_linked_by_fatal_attraction(
        PlayerbotAI* botAI) { return new MotherShahrazBotsAreLinkedByFatalAttractionTrigger(botAI); }

    // Illidari Council

    static Trigger* illidari_council_pulling_bosses(
        PlayerbotAI* botAI) { return new IllidariCouncilPullingBossesTrigger(botAI); }

    // Illidan Stormrage <The Betrayer>
};

#endif
