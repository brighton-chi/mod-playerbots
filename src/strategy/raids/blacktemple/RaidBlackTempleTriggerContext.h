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

        creators["supremus changes phase every 60 seconds"] =
            &RaidBlackTempleTriggerContext::supremus_changes_phase_every_60_seconds;

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

        // Reliquary of Souls

        creators["reliquary of souls essence of desire has rune shield"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_has_rune_shield;

        creators["reliquary of souls essence of desire casting deaden"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_casting_deaden;

        // Mother Shahraz

        // Illidari Council

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

    static Trigger* supremus_changes_phase_every_60_seconds(
        PlayerbotAI* botAI) { return new SupremusChangesPhaseEvery60SecondsTrigger(botAI); }

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

    // Reliquary of Souls

    static Trigger* reliquary_of_souls_essence_of_desire_has_rune_shield(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(botAI); }

    static Trigger* reliquary_of_souls_essence_of_desire_casting_deaden(
        PlayerbotAI* botAI) { return new ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger(botAI); }

    // Mother Shahraz

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
};

#endif
