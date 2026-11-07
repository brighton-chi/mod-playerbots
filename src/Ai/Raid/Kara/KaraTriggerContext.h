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

        creators["attumen the huntsman should hold dps"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_should_hold_dps;

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

        creators["shade of aran ranged is in blizzard"] =
            &RaidKarazhanTriggerContext::shade_of_aran_ranged_is_in_blizzard;

        creators["shade of aran flame wreath is active"] =
            &RaidKarazhanTriggerContext::shade_of_aran_flame_wreath_is_active;

        creators["shade of aran conjured elementals summoned"] =
            &RaidKarazhanTriggerContext::shade_of_aran_conjured_elementals_summoned;

        creators["shade of aran ranged should maintain distance"] =
            &RaidKarazhanTriggerContext::shade_of_aran_ranged_should_maintain_distance;

        // Netherspite
        creators["netherspite should block red beam"] =
            &RaidKarazhanTriggerContext::netherspite_should_block_red_beam;

        creators["netherspite should block beam"] =
            &RaidKarazhanTriggerContext::netherspite_should_block_beam;

        creators["netherspite next beam blocker should prepare"] =
            &RaidKarazhanTriggerContext::netherspite_next_beam_blocker_should_prepare;

        creators["netherspite should position around beams"] =
            &RaidKarazhanTriggerContext::netherspite_should_position_around_beams;

        creators["netherspite should spread for netherbreath"] =
            &RaidKarazhanTriggerContext::netherspite_should_spread_for_netherbreath;

        creators["netherspite should hold dps"] =
            &RaidKarazhanTriggerContext::netherspite_should_hold_dps;

        creators["netherspite should manage timers and trackers"] =
            &RaidKarazhanTriggerContext::netherspite_need_to_manage_timers_and_trackers;

        // Prince Malchezaar
        creators["prince malchezaar bot is enfeebled"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_bot_is_enfeebled;

        creators["prince malchezaar bot in hazard"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_bot_in_hazard;

        creators["prince malchezaar should be tanked"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_should_be_tanked;

        // Nightbane
        creators["nightbane should be tanked"] =
            &RaidKarazhanTriggerContext::nightbane_should_be_tanked;

        creators["nightbane ranged should stack"] =
            &RaidKarazhanTriggerContext::nightbane_ranged_should_stack;

        creators["nightbane pets should be held"] =
            &RaidKarazhanTriggerContext::nightbane_pets_should_be_held;

        creators["nightbane should hold dps"] =
            &RaidKarazhanTriggerContext::nightbane_should_hold_dps;

        creators["nightbane should stack in flight"] =
            &RaidKarazhanTriggerContext::nightbane_should_stack_in_flight;

        creators["nightbane should stand down"] =
            &RaidKarazhanTriggerContext::nightbane_should_stand_down;

        creators["nightbane should manage timers and trackers"] =
            &RaidKarazhanTriggerContext::nightbane_should_manage_timers_and_trackers;
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
    static Trigger* attumen_the_huntsman_should_hold_dps(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanShouldHoldDpsTrigger(botAI);
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
    static Trigger* shade_of_aran_ranged_is_in_blizzard(PlayerbotAI* botAI)
    {
        return new ShadeOfAranRangedIsInBlizzardTrigger(botAI);
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
    static Trigger* netherspite_should_block_red_beam(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldBlockRedBeamTrigger(botAI);
    }
    static Trigger* netherspite_should_block_beam(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldBlockBeamTrigger(botAI);
    }
    static Trigger* netherspite_next_beam_blocker_should_prepare(PlayerbotAI* botAI)
    {
        return new NetherspiteNextBeamBlockerShouldPrepareTrigger(botAI);
    }
    static Trigger* netherspite_should_position_around_beams(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldPositionAroundBeamsTrigger(botAI);
    }
    static Trigger* netherspite_should_spread_for_netherbreath(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldSpreadForNetherbreathTrigger(botAI);
    }
    static Trigger* netherspite_should_hold_dps(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldHoldDpsTrigger(botAI);
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
    static Trigger* prince_malchezaar_bot_in_hazard(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarBotInHazardTrigger(botAI);
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
    static Trigger* nightbane_ranged_should_stack(PlayerbotAI* botAI)
    {
        return new NightbaneRangedShouldStackTrigger(botAI);
    }
    static Trigger* nightbane_pets_should_be_held(PlayerbotAI* botAI)
    {
        return new NightbanePetsShouldBeHeldTrigger(botAI);
    }
    static Trigger* nightbane_should_hold_dps(PlayerbotAI* botAI)
    {
        return new NightbaneShouldHoldDpsTrigger(botAI);
    }
    static Trigger* nightbane_should_stack_in_flight(PlayerbotAI* botAI)
    {
        return new NightbaneShouldStackInFlightTrigger(botAI);
    }
    static Trigger* nightbane_should_stand_down(PlayerbotAI* botAI)
    {
        return new NightbaneShouldStandDownTrigger(botAI);
    }
    static Trigger* nightbane_should_manage_timers_and_trackers(PlayerbotAI* botAI)
    {
        return new NightbaneShouldManageTimersAndTrackersTrigger(botAI);
    }
};

#endif
