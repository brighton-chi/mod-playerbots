#ifndef _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H

#include "RaidKarazhanTriggers.h"
#include "AiObjectContext.h"

class RaidKarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidKarazhanTriggerContext()
    {
        creators["spectral retainer need target priority"] = &RaidKarazhanTriggerContext::spectral_retainer_need_target_priority;
        creators["mana warp is about to explode"] = &RaidKarazhanTriggerContext::mana_warp_is_about_to_explode;

        creators["attumen the huntsman need target priority"] = &RaidKarazhanTriggerContext::attumen_the_huntsman_need_target_priority;
        creators["attumen the huntsman attumen spawned"] = &RaidKarazhanTriggerContext::attumen_the_huntsman_attumen_spawned;
        creators["attumen the huntsman attumen mounted"] = &RaidKarazhanTriggerContext::attumen_the_huntsman_attumen_mounted;
        creators["attumen the huntsman manage timer"] = &RaidKarazhanTriggerContext::attumen_the_huntsman_manage_timer;

        creators["moroes need target priority"] = &RaidKarazhanTriggerContext::moroes_need_target_priority;

        creators["maiden of virtue boss engaged by main tank"] = &RaidKarazhanTriggerContext::maiden_of_virtue_boss_engaged_by_main_tank;
        creators["maiden of virtue boss engaged by ranged"] = &RaidKarazhanTriggerContext::maiden_of_virtue_boss_engaged_by_ranged;

        creators["big bad wolf boss engaged by main tank"] = &RaidKarazhanTriggerContext::big_bad_wolf_boss_engaged_by_main_tank;
        creators["big bad wolf boss is chasing little red riding hood"] = &RaidKarazhanTriggerContext::big_bad_wolf_boss_is_chasing_little_red_riding_hood;

        creators["romulo and julianne both bosses revived"] = &RaidKarazhanTriggerContext::romulo_and_julianne_both_bosses_revived;

        creators["wizard of oz need target priority"] = &RaidKarazhanTriggerContext::wizard_of_oz_need_target_priority;
        creators["wizard of oz strawman is vulnerable to fire"] = &RaidKarazhanTriggerContext::wizard_of_oz_strawman_is_vulnerable_to_fire;

        creators["the curator astral flare spawned"] = &RaidKarazhanTriggerContext::the_curator_astral_flare_spawned;
        creators["the curator boss engaged by tanks"] = &RaidKarazhanTriggerContext::the_curator_boss_engaged_by_tanks;
        creators["the curator boss engaged by ranged"] = &RaidKarazhanTriggerContext::the_curator_boss_engaged_by_ranged;

        creators["terestian illhoof need target priority"] = &RaidKarazhanTriggerContext::terestian_illhoof_need_target_priority;
        
        creators["shade of aran arcane explosion is casting"] = &RaidKarazhanTriggerContext::shade_of_aran_arcane_explosion_is_casting;
        creators["shade of aran flame wreath is casting"] = &RaidKarazhanTriggerContext::shade_of_aran_flame_wreath_is_casting;
        creators["shade of aran conjured elementals summoned"] = &RaidKarazhanTriggerContext::shade_of_aran_conjured_elementals_summoned;
        creators["shade of aran boss engaged by ranged"] = &RaidKarazhanTriggerContext::shade_of_aran_boss_engaged_by_ranged;
        
        creators["netherspite red beam is active"] = &RaidKarazhanTriggerContext::netherspite_red_beam_is_active;
        creators["netherspite blue beam is active"] = &RaidKarazhanTriggerContext::netherspite_blue_beam_is_active;
        creators["netherspite green beam is active"] = &RaidKarazhanTriggerContext::netherspite_green_beam_is_active;
        creators["netherspite bot is not beam blocker"] = &RaidKarazhanTriggerContext::netherspite_bot_is_not_beam_blocker;
        creators["netherspite boss is banished"] = &RaidKarazhanTriggerContext::netherspite_boss_is_banished;
        creators["netherspite need to manage timers"] = &RaidKarazhanTriggerContext::netherspite_need_to_manage_timers;

        creators["prince malchezaar bot is enfeebled"] = &RaidKarazhanTriggerContext::prince_malchezaar_bot_is_enfeebled;
        creators["prince malchezaar infernals are spawned"] = &RaidKarazhanTriggerContext::prince_malchezaar_infernals_are_spawned;
        creators["prince malchezaar boss engaged by main tank"] = &RaidKarazhanTriggerContext::prince_malchezaar_boss_engaged_by_main_tank;

        creators["nightbane boss engaged by main tank"] = &RaidKarazhanTriggerContext::nightbane_boss_engaged_by_main_tank;
        creators["nightbane ranged prepare for charred earth"] = &RaidKarazhanTriggerContext::nightbane_ranged_prepare_for_charred_earth;
        creators["nightbane main tank is susceptible to fear"] = &RaidKarazhanTriggerContext::nightbane_main_tank_is_susceptible_to_fear;
        creators["nightbane pets ignore collision to chase flying boss"] = &RaidKarazhanTriggerContext::nightbane_pets_ignore_collision_to_chase_flying_boss;
        creators["nightbane boss is flying"] = &RaidKarazhanTriggerContext::nightbane_boss_is_flying;
        creators["nightbane need to manage timers and trackers"] = &RaidKarazhanTriggerContext::nightbane_need_to_manage_timers_and_trackers;
    }

private:
    static Trigger* spectral_retainer_need_target_priority(PlayerbotAI* botAI) { return new SpectralRetainerNeedTargetPriorityTrigger(botAI); }
    static Trigger* mana_warp_is_about_to_explode(PlayerbotAI* botAI) { return new ManaWarpIsAboutToExplodeTrigger(botAI); }

    static Trigger* attumen_the_huntsman_need_target_priority(PlayerbotAI* botAI) { return new AttumenTheHuntsmanNeedTargetPriorityTrigger(botAI); }
    static Trigger* attumen_the_huntsman_attumen_spawned(PlayerbotAI* botAI) { return new AttumenTheHuntsmanAttumenSpawnedTrigger(botAI); }
    static Trigger* attumen_the_huntsman_attumen_mounted(PlayerbotAI* botAI) { return new AttumenTheHuntsmanAttumenMountedTrigger(botAI); }
    static Trigger* attumen_the_huntsman_manage_timer(PlayerbotAI* botAI) { return new AttumenTheHuntsmanManageTimerTrigger(botAI); }
    
    static Trigger* moroes_need_target_priority(PlayerbotAI* botAI) { return new MoroesNeedTargetPriorityTrigger(botAI); }
   
    static Trigger* maiden_of_virtue_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new MaidenOfVirtueBossEngagedByMainTankTrigger(botAI); }
    static Trigger* maiden_of_virtue_boss_engaged_by_ranged(PlayerbotAI* botAI) { return new MaidenOfVirtueBossEngagedByRangedTrigger(botAI); }

    static Trigger* big_bad_wolf_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new BigBadWolfBossEngagedByMainTankTrigger(botAI); }
    static Trigger* big_bad_wolf_boss_is_chasing_little_red_riding_hood(PlayerbotAI* botAI) { return new BigBadWolfBossIsChasingLittleRedRidingHoodTrigger(botAI); }

    static Trigger* romulo_and_julianne_both_bosses_revived(PlayerbotAI* botAI) { return new RomuloAndJulianneBothBossesRevivedTrigger(botAI); }

    static Trigger* wizard_of_oz_need_target_priority(PlayerbotAI* botAI) { return new WizardOfOzNeedTargetPriorityTrigger(botAI); }
    static Trigger* wizard_of_oz_strawman_is_vulnerable_to_fire(PlayerbotAI* botAI) { return new WizardOfOzStrawmanIsVulnerableToFireTrigger(botAI); }

    static Trigger* the_curator_astral_flare_spawned(PlayerbotAI* botAI) { return new TheCuratorAstralFlareSpawnedTrigger(botAI); }
    static Trigger* the_curator_boss_engaged_by_tanks(PlayerbotAI* botAI) { return new TheCuratorBossEngagedByTanksTrigger(botAI); }
    static Trigger* the_curator_boss_engaged_by_ranged(PlayerbotAI* botAI) { return new TheCuratorBossEngagedByRangedTrigger(botAI); }

    static Trigger* terestian_illhoof_need_target_priority(PlayerbotAI* botAI) { return new TerestianIllhoofNeedTargetPriorityTrigger(botAI); }
    
    static Trigger* shade_of_aran_arcane_explosion_is_casting(PlayerbotAI* botAI) { return new ShadeOfAranArcaneExplosionIsCastingTrigger(botAI); }
    static Trigger* shade_of_aran_flame_wreath_is_casting(PlayerbotAI* botAI) { return new ShadeOfAranFlameWreathIsCastingTrigger(botAI); }
    static Trigger* shade_of_aran_conjured_elementals_summoned(PlayerbotAI* botAI) { return new ShadeOfAranConjuredElementalsSummonedTrigger(botAI); }
    static Trigger* shade_of_aran_boss_engaged_by_ranged(PlayerbotAI* botAI) { return new ShadeOfAranBossEngagedByRangedTrigger(botAI); }
    
    static Trigger* netherspite_red_beam_is_active(PlayerbotAI* botAI) { return new NetherspiteRedBeamIsActiveTrigger(botAI); }
    static Trigger* netherspite_blue_beam_is_active(PlayerbotAI* botAI) { return new NetherspiteBlueBeamIsActiveTrigger(botAI); }
    static Trigger* netherspite_green_beam_is_active(PlayerbotAI* botAI) { return new NetherspiteGreenBeamIsActiveTrigger(botAI); }
    static Trigger* netherspite_bot_is_not_beam_blocker(PlayerbotAI* botAI) { return new NetherspiteBotIsNotBeamBlockerTrigger(botAI); }
    static Trigger* netherspite_boss_is_banished(PlayerbotAI* botAI) { return new NetherspiteBossIsBanishedTrigger(botAI); }
    static Trigger* netherspite_need_to_manage_timers(PlayerbotAI* botAI) { return new NetherspiteNeedToManageTimersTrigger(botAI); }

    static Trigger* prince_malchezaar_bot_is_enfeebled(PlayerbotAI* botAI) { return new PrinceMalchezaarBotIsEnfeebledTrigger(botAI); }
    static Trigger* prince_malchezaar_infernals_are_spawned(PlayerbotAI* botAI) { return new PrinceMalchezaarInfernalsAreSpawnedTrigger(botAI); }
    static Trigger* prince_malchezaar_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new PrinceMalchezaarBossEngagedByMainTankTrigger(botAI); }

    static Trigger* nightbane_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new NightbaneBossEngagedByMainTankTrigger(botAI); }
    static Trigger* nightbane_ranged_prepare_for_charred_earth(PlayerbotAI* botAI) { return new NightbaneRangedPrepareForCharredEarthTrigger(botAI); }
    static Trigger* nightbane_main_tank_is_susceptible_to_fear(PlayerbotAI* botAI) { return new NightbaneMainTankIsSusceptibleToFearTrigger(botAI); }
    static Trigger* nightbane_pets_ignore_collision_to_chase_flying_boss(PlayerbotAI* botAI) { return new NightbanePetsIgnoreColllisionToChaseFlyingBossTrigger(botAI); }
    static Trigger* nightbane_boss_is_flying(PlayerbotAI* botAI) { return new NightbaneBossIsFlyingTrigger(botAI); }
    static Trigger* nightbane_need_to_manage_timers_and_trackers(PlayerbotAI* botAI) { return new NightbaneNeedToManageTimersAndTrackersTrigger(botAI); }
};

#endif