/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARATRIGGERCONTEXT_H
#define PLAYERBOTS_KARATRIGGERCONTEXT_H

#include "KaraTriggers.h"
#include "NamedObjectContext.h"

class RaidKarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidKarazhanTriggerContext()
    {
        // Shared
        creators["karazhan no encounter in progress"] =
            &RaidKarazhanTriggerContext::karazhan_no_encounter_in_progress;

        creators["karazhan enemies cast fear"] =
            &RaidKarazhanTriggerContext::karazhan_enemies_cast_fear;

        // Trash
        creators["mana warp casts warp breach"] =
            &RaidKarazhanTriggerContext::mana_warp_casts_warp_breach;

        creators["mana warp stunner should hold"] =
            &RaidKarazhanTriggerContext::mana_warp_stunner_should_hold;

        // Attumen the Huntsman
        creators["attumen the huntsman should be tanked"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_should_be_tanked;

        creators["attumen the huntsman midnight should be focused"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_midnight_should_be_focused;

        creators["attumen the huntsman mounted should be tanked"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_mounted_should_be_tanked;

        creators["attumen the huntsman should stack behind"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_should_stack_behind;

        creators["attumen the huntsman should manage dps timer"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_should_manage_dps_timer;

        // Moroes
        creators["moroes should prioritize adds"] =
            &RaidKarazhanTriggerContext::moroes_should_prioritize_adds;

        // Maiden of Virtue
        creators["maiden of virtue should be tanked"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_should_be_tanked;

        creators["maiden of virtue should break repentance"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_should_break_repentance;

        creators["maiden of virtue ranged should spread"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_ranged_should_spread;

        creators["maiden of virtue should ground holy fire"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_should_ground_holy_fire;

        // The Big Bad Wolf
        creators["big bad wolf should be tanked"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_should_be_tanked;

        creators["big bad wolf chasing red hood"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_chasing_red_hood;

        creators["big bad wolf paladin should protect red hood"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_paladin_should_protect_red_hood;

        // Romulo and Julianne
        creators["romulo and julianne should be killed together"] =
            &RaidKarazhanTriggerContext::romulo_and_julianne_should_be_killed_together;

        // The Wizard of Oz
        creators["wizard of oz should prioritize targets"] =
            &RaidKarazhanTriggerContext::wizard_of_oz_should_prioritize_targets;

        creators["wizard of oz mage should burn strawman"] =
            &RaidKarazhanTriggerContext::wizard_of_oz_mage_should_burn_strawman;

        // The Curator
        creators["the curator should prioritize astral flares"] =
            &RaidKarazhanTriggerContext::the_curator_should_prioritize_astral_flares;

        creators["the curator should be tanked"] =
            &RaidKarazhanTriggerContext::the_curator_should_be_tanked;

        creators["the curator ranged should spread"] =
            &RaidKarazhanTriggerContext::the_curator_ranged_should_spread;

        // Terestian Illhoof
        creators["terestian illhoof should prioritize chains"] =
            &RaidKarazhanTriggerContext::terestian_illhoof_should_prioritize_chains;

        // Shade of Aran
        creators["shade of aran arcane explosion is casting"] =
            &RaidKarazhanTriggerContext::shade_of_aran_arcane_explosion_is_casting;

        creators["shade of aran flame wreath is active"] =
            &RaidKarazhanTriggerContext::shade_of_aran_flame_wreath_is_active;

        creators["shade of aran conjured elementals summoned"] =
            &RaidKarazhanTriggerContext::shade_of_aran_conjured_elementals_summoned;

        creators["shade of aran ranged should maintain distance"] =
            &RaidKarazhanTriggerContext::shade_of_aran_ranged_should_maintain_distance;

        // Netherspite
        creators["netherspite red beam is active"] =
            &RaidKarazhanTriggerContext::netherspite_red_beam_is_active;

        creators["netherspite blue beam is active"] =
            &RaidKarazhanTriggerContext::netherspite_blue_beam_is_active;

        creators["netherspite green beam is active"] =
            &RaidKarazhanTriggerContext::netherspite_green_beam_is_active;

        creators["netherspite bot is not beam blocker"] =
            &RaidKarazhanTriggerContext::netherspite_bot_is_not_beam_blocker;

        creators["netherspite in banish phase"] =
            &RaidKarazhanTriggerContext::netherspite_in_banish_phase;

        creators["netherspite should manage timers and trackers"] =
            &RaidKarazhanTriggerContext::netherspite_need_to_manage_timers_and_trackers;

        // Prince Malchezaar
        creators["prince malchezaar bot is enfeebled"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_bot_is_enfeebled;

        creators["prince malchezaar engaged by non-tanks"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_engaged_by_non_tanks;

        creators["prince malchezaar should be tanked"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_should_be_tanked;

        // Nightbane
        creators["nightbane should be tanked"] =
            &RaidKarazhanTriggerContext::nightbane_should_be_tanked;

        creators["nightbane ground phase engaged by ranged"] =
            &RaidKarazhanTriggerContext::nightbane_ground_phase_engaged_by_ranged;

        creators["nightbane pets chase flying boss out of bounds"] =
            &RaidKarazhanTriggerContext::nightbane_pets_chase_flying_boss_out_of_bounds;

        creators["nightbane in flight phase"] =
            &RaidKarazhanTriggerContext::nightbane_in_flight_phase;

        creators["nightbane bot went out of bounds"] =
            &RaidKarazhanTriggerContext::nightbane_bot_went_out_of_bounds;

        creators["nightbane should manage timers and trackers"] =
            &RaidKarazhanTriggerContext::nightbane_need_to_manage_timers_and_trackers;
    }

private:
    // Shared
    static Trigger* karazhan_no_encounter_in_progress(PlayerbotAI* botAI)
    {
        return new KarazhanNoEncounterInProgressTrigger(botAI);
    }
    static Trigger* karazhan_enemies_cast_fear(PlayerbotAI* botAI)
    {
        return new KarazhanEnemiesCastFearTrigger(botAI);
    }

    // Trash
    static Trigger* mana_warp_casts_warp_breach(PlayerbotAI* botAI)
    {
        return new ManaWarpCastsWarpBreachTrigger(botAI);
    }
    static Trigger* mana_warp_stunner_should_hold(PlayerbotAI* botAI)
    {
        return new ManaWarpStunnerShouldHoldTrigger(botAI);
    }

    // Attumen the Huntsman
    static Trigger* attumen_the_huntsman_should_be_tanked(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanShouldBeTankedTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_midnight_should_be_focused(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanMidnightShouldBeFocusedTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_mounted_should_be_tanked(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanMountedShouldBeTankedTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_should_stack_behind(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanShouldStackBehindTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_should_manage_dps_timer(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanShouldManageDpsTimerTrigger(botAI);
    }

    // Moroes
    static Trigger* moroes_should_prioritize_adds(PlayerbotAI* botAI)
    {
        return new MoroesShouldPrioritizeAddsTrigger(botAI);
    }

    // Maiden of Virtue
    static Trigger* maiden_of_virtue_should_be_tanked(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueShouldBeTankedTrigger(botAI);
    }
    static Trigger* maiden_of_virtue_should_break_repentance(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueShouldBreakRepentanceTrigger(botAI);
    }
    static Trigger* maiden_of_virtue_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* maiden_of_virtue_should_ground_holy_fire(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueShouldGroundHolyFireTrigger(botAI);
    }

    // The Big Bad Wolf
    static Trigger* big_bad_wolf_should_be_tanked(PlayerbotAI* botAI)
    {
        return new BigBadWolfShouldBeTankedTrigger(botAI);
    }
    static Trigger* big_bad_wolf_chasing_red_hood(PlayerbotAI* botAI)
    {
        return new BigBadWolfChasingRedHoodTrigger(botAI);
    }
    static Trigger* big_bad_wolf_paladin_should_protect_red_hood(PlayerbotAI* botAI)
    {
        return new BigBadWolfPaladinShouldProtectRedHoodTrigger(botAI);
    }

    // Romulo and Julianne
    static Trigger* romulo_and_julianne_should_be_killed_together(PlayerbotAI* botAI)
    {
        return new RomuloAndJulianneShouldBeKilledTogetherTrigger(botAI);
    }

    // The Wizard of Oz
    static Trigger* wizard_of_oz_should_prioritize_targets(PlayerbotAI* botAI)
    {
        return new WizardOfOzShouldPrioritizeTargetsTrigger(botAI);
    }
    static Trigger* wizard_of_oz_mage_should_burn_strawman(PlayerbotAI* botAI)
    {
        return new WizardOfOzMageShouldBurnStrawmanTrigger(botAI);
    }

    // The Curator
    static Trigger* the_curator_should_prioritize_astral_flares(PlayerbotAI* botAI)
    {
        return new TheCuratorShouldPrioritizeAstralFlaresTrigger(botAI);
    }
    static Trigger* the_curator_should_be_tanked(PlayerbotAI* botAI)
    {
        return new TheCuratorShouldBeTankedTrigger(botAI);
    }
    static Trigger* the_curator_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new TheCuratorRangedShouldSpreadTrigger(botAI);
    }

    // Terestian Illhoof
    static Trigger* terestian_illhoof_should_prioritize_chains(PlayerbotAI* botAI)
    {
        return new TerestianIllhoofShouldPrioritizeChainsTrigger(botAI);
    }

    // Shade of Aran
    static Trigger* shade_of_aran_arcane_explosion_is_casting(PlayerbotAI* botAI)
    {
        return new ShadeOfAranArcaneExplosionIsCastingTrigger(botAI);
    }
    static Trigger* shade_of_aran_flame_wreath_is_active(PlayerbotAI* botAI)
    {
        return new ShadeOfAranFlameWreathIsActiveTrigger(botAI);
    }
    static Trigger* shade_of_aran_conjured_elementals_summoned(PlayerbotAI* botAI)
    {
        return new ShadeOfAranConjuredElementalsSummonedTrigger(botAI);
    }
    static Trigger* shade_of_aran_ranged_should_maintain_distance(PlayerbotAI* botAI)
    {
        return new ShadeOfAranRangedShouldMaintainDistanceTrigger(botAI);
    }

    // Netherspite
    static Trigger* netherspite_red_beam_is_active(PlayerbotAI* botAI)
    {
        return new NetherspiteRedBeamIsActiveTrigger(botAI);
    }
    static Trigger* netherspite_blue_beam_is_active(PlayerbotAI* botAI)
    {
        return new NetherspiteBlueBeamIsActiveTrigger(botAI);
    }
    static Trigger* netherspite_green_beam_is_active(PlayerbotAI* botAI)
    {
        return new NetherspiteGreenBeamIsActiveTrigger(botAI);
    }
    static Trigger* netherspite_bot_is_not_beam_blocker(PlayerbotAI* botAI)
    {
        return new NetherspiteBotIsNotBeamBlockerTrigger(botAI);
    }
    static Trigger* netherspite_in_banish_phase(PlayerbotAI* botAI)
    {
        return new NetherspiteInBanishPhaseTrigger(botAI);
    }
    static Trigger* netherspite_need_to_manage_timers_and_trackers(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldManageTimersAndTrackersTrigger(botAI);
    }

    // Prince Malchezaar
    static Trigger* prince_malchezaar_bot_is_enfeebled(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarBotIsEnfeebledTrigger(botAI);
    }
    static Trigger* prince_malchezaar_engaged_by_non_tanks(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarEngagedByNonTanksTrigger(botAI);
    }
    static Trigger* prince_malchezaar_should_be_tanked(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarShouldBeTankedTrigger(botAI);
    }

    // Nightbane
    static Trigger* nightbane_should_be_tanked(PlayerbotAI* botAI)
    {
        return new NightbaneShouldBeTankedTrigger(botAI);
    }
    static Trigger* nightbane_ground_phase_engaged_by_ranged(PlayerbotAI* botAI)
    {
        return new NightbaneGroundPhaseEngagedByRangedTrigger(botAI);
    }
    static Trigger* nightbane_pets_chase_flying_boss_out_of_bounds(PlayerbotAI* botAI)
    {
        return new NightbanePetsChaseFlyingBossOutOfBoundsTrigger(botAI);
    }
    static Trigger* nightbane_in_flight_phase(PlayerbotAI* botAI)
    {
        return new NightbaneInFlightPhaseTrigger(botAI);
    }
    static Trigger* nightbane_bot_went_out_of_bounds(PlayerbotAI* botAI)
    {
        return new NightbaneBotWentOutOfBoundsTrigger(botAI);
    }
    static Trigger* nightbane_need_to_manage_timers_and_trackers(PlayerbotAI* botAI)
    {
        return new NightbaneShouldManageTimersAndTrackersTrigger(botAI);
    }
};

#endif
