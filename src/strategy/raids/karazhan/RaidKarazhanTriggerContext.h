#ifndef _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H

#include "RaidKarazhanTriggers.h"
#include "AiObjectContext.h"

class RaidKarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidKarazhanTriggerContext()
    {
        creators["attumen the huntsman main tank"] = &RaidKarazhanTriggerContext::attumen_the_huntsman_main_tank;
        creators["attumen the huntsman split bosses"] = &RaidKarazhanTriggerContext::attumen_the_huntsman_split_bosses;
        creators["attumen the huntsman stack behind"] = &RaidKarazhanTriggerContext::attumen_the_huntsman_stack_behind;
        creators["attumen the huntsman manage timer"] = &RaidKarazhanTriggerContext::attumen_the_huntsman_manage_timer;
        
        creators["moroes mark target"] = &RaidKarazhanTriggerContext::moroes_mark_target;
        
        creators["maiden of virtue main tank movement"] = &RaidKarazhanTriggerContext::maiden_of_virtue_main_tank_movement;
        creators["maiden of virtue position ranged"] = &RaidKarazhanTriggerContext::maiden_of_virtue_position_ranged;
        
        creators["big bad wolf run away"] = &RaidKarazhanTriggerContext::big_bad_wolf_run_away;
        creators["big bad wolf position boss"] = &RaidKarazhanTriggerContext::big_bad_wolf_position_boss;
        
        creators["romulo and julianne mark target"] = &RaidKarazhanTriggerContext::romulo_and_julianne_mark_target;
        
        creators["wizard of oz mark target"] = &RaidKarazhanTriggerContext::wizard_of_oz_mark_target;
        creators["wizard of oz scorch strawman"] = &RaidKarazhanTriggerContext::wizard_of_oz_scorch_strawman;
        
        creators["the curator mark astral flare"] = &RaidKarazhanTriggerContext::the_curator_mark_astral_flare;
        creators["the curator position boss"] = &RaidKarazhanTriggerContext::the_curator_position_boss;
        creators["the curator spread ranged"] = &RaidKarazhanTriggerContext::the_curator_spread_ranged;
       
        creators["terestian illhoof mark target"] = &RaidKarazhanTriggerContext::terestian_illhoof_mark_target;
        
        creators["shade of aran arcane explosion run away"] = &RaidKarazhanTriggerContext::shade_of_aran_arcane_explosion_run_away;
        creators["shade of aran flame wreath stop movement"] = &RaidKarazhanTriggerContext::shade_of_aran_flame_wreath_stop_movement;
        creators["shade of aran mark conjured elemental"] = &RaidKarazhanTriggerContext::shade_of_aran_mark_conjured_elemental;
        creators["shade of aran spread ranged"] = &RaidKarazhanTriggerContext::shade_of_aran_spread_ranged;
        
        creators["netherspite block red beam"] = &RaidKarazhanTriggerContext::netherspite_block_red_beam;
        creators["netherspite block blue beam"] = &RaidKarazhanTriggerContext::netherspite_block_blue_beam;
        creators["netherspite block green beam"] = &RaidKarazhanTriggerContext::netherspite_block_green_beam;
        creators["netherspite avoid beam and void zone"] = &RaidKarazhanTriggerContext::netherspite_avoid_beam_and_void_zone;
        creators["netherspite banish phase avoid void zone"] = &RaidKarazhanTriggerContext::netherspite_banish_phase_avoid_void_zone;
        creators["netherspite manage timers"] = &RaidKarazhanTriggerContext::netherspite_manage_timers;
       
        creators["prince malchezaar enfeebled avoid hazard"] = &RaidKarazhanTriggerContext::prince_malchezaar_enfeebled_avoid_hazard;
        creators["prince malchezaar non tank avoid infernal"] = &RaidKarazhanTriggerContext::prince_malchezaar_non_tank_avoid_infernal;
        // creators["prince malchezaar non tank avoid hazard"] = &RaidKarazhanTriggerContext::prince_malchezaar_non_tank_avoid_hazard;
        creators["prince malchezaar main tank movement"] = &RaidKarazhanTriggerContext::prince_malchezaar_main_tank_movement;
        
        creators["nightbane ground phase position boss"] = &RaidKarazhanTriggerContext::nightbane_ground_phase_position_boss;
        creators["nightbane ground phase rotate ranged positions"] = &RaidKarazhanTriggerContext::nightbane_ground_phase_rotate_ranged_positions;
        creators["nightbane cast fear ward on main tank"] = &RaidKarazhanTriggerContext::nightbane_cast_fear_ward_on_main_tank;
        creators["nightbane control pet aggression"] = &RaidKarazhanTriggerContext::nightbane_control_pet_aggression;
        creators["nightbane flight phase movement"] = &RaidKarazhanTriggerContext::nightbane_flight_phase_movement;
        creators["nightbane manage timers and trackers"] = &RaidKarazhanTriggerContext::nightbane_manage_timers_and_trackers;
    }

private:
    static Trigger* attumen_the_huntsman_main_tank(PlayerbotAI* botAI) { return new AttumenTheHuntsmanMainTankTrigger(botAI); }
    static Trigger* attumen_the_huntsman_split_bosses(PlayerbotAI* botAI) { return new AttumenTheHuntsmanSplitBossesTrigger(botAI); }
    static Trigger* attumen_the_huntsman_stack_behind(PlayerbotAI* botAI) { return new AttumenTheHuntsmanStackBehindTrigger(botAI); }
    static Trigger* attumen_the_huntsman_manage_timer(PlayerbotAI* botAI) { return new AttumenTheHuntsmanManageTimerTrigger(botAI); }
    
    static Trigger* moroes_mark_target(PlayerbotAI* botAI) { return new MoroesMarkTargetTrigger(botAI); }
   
    static Trigger* maiden_of_virtue_main_tank_movement(PlayerbotAI* botAI) { return new MaidenOfVirtueMainTankMovementTrigger(botAI); }
    static Trigger* maiden_of_virtue_position_ranged(PlayerbotAI* botAI) { return new MaidenOfVirtuePositionRangedTrigger(botAI); }
    
    static Trigger* big_bad_wolf_run_away(PlayerbotAI* botAI) { return new BigBadWolfRunAwayTrigger(botAI); }
    static Trigger* big_bad_wolf_position_boss(PlayerbotAI* botAI) { return new BigBadWolfPositionBossTrigger(botAI); }
    
    static Trigger* romulo_and_julianne_mark_target(PlayerbotAI* botAI) { return new RomuloAndJulianneMarkTargetTrigger(botAI); }
    
    static Trigger* wizard_of_oz_mark_target(PlayerbotAI* botAI) { return new WizardOfOzMarkTargetTrigger(botAI); }
    static Trigger* wizard_of_oz_scorch_strawman(PlayerbotAI* botAI) { return new WizardOfOzScorchStrawmanTrigger(botAI); }
    
    static Trigger* the_curator_mark_astral_flare(PlayerbotAI* botAI) { return new TheCuratorMarkAstralFlareTrigger(botAI); }
    static Trigger* the_curator_position_boss(PlayerbotAI* botAI) { return new TheCuratorPositionBossTrigger(botAI); }
    static Trigger* the_curator_spread_ranged(PlayerbotAI* botAI) { return new TheCuratorSpreadRangedTrigger(botAI); }
    
    static Trigger* terestian_illhoof_mark_target(PlayerbotAI* botAI) { return new TerestianIllhoofMarkTargetTrigger(botAI); }
    
    static Trigger* shade_of_aran_arcane_explosion_run_away(PlayerbotAI* botAI) { return new ShadeOfAranArcaneExplosionRunAwayTrigger(botAI); }
    static Trigger* shade_of_aran_flame_wreath_stop_movement(PlayerbotAI* botAI) { return new ShadeOfAranFlameWreathStopMovementTrigger(botAI); }
    static Trigger* shade_of_aran_mark_conjured_elemental(PlayerbotAI* botAI) { return new ShadeOfAranMarkConjuredElementalTrigger(botAI); }
    static Trigger* shade_of_aran_spread_ranged(PlayerbotAI* botAI) { return new ShadeOfAranSpreadRangedTrigger(botAI); }
    
    static Trigger* netherspite_block_red_beam(PlayerbotAI* botAI) { return new NetherspiteBlockRedBeamTrigger(botAI); }
    static Trigger* netherspite_block_blue_beam(PlayerbotAI* botAI) { return new NetherspiteBlockBlueBeamTrigger(botAI); }
    static Trigger* netherspite_block_green_beam(PlayerbotAI* botAI) { return new NetherspiteBlockGreenBeamTrigger(botAI); }
    static Trigger* netherspite_avoid_beam_and_void_zone(PlayerbotAI* botAI) { return new NetherspiteAvoidBeamAndVoidZoneTrigger(botAI); }
    static Trigger* netherspite_banish_phase_avoid_void_zone(PlayerbotAI* botAI) { return new NetherspiteBanishPhaseAvoidVoidZoneTrigger(botAI); }
    static Trigger* netherspite_manage_timers(PlayerbotAI* botAI) { return new NetherspiteManageTimersTrigger(botAI); }
    
    static Trigger* prince_malchezaar_enfeebled_avoid_hazard(PlayerbotAI* botAI) { return new PrinceMalchezaarEnfeebledAvoidHazardTrigger(botAI); }
    static Trigger* prince_malchezaar_non_tank_avoid_infernal(PlayerbotAI* botAI) { return new PrinceMalchezaarNonTankAvoidInfernalTrigger(botAI); }
    // static Trigger* prince_malchezaar_non_tank_avoid_hazard(PlayerbotAI* botAI) { return new PrinceMalchezaarNonTankAvoidHazardTrigger(botAI); }
    static Trigger* prince_malchezaar_main_tank_movement(PlayerbotAI* botAI) { return new PrinceMalchezaarMainTankMovementTrigger(botAI); }
    
    static Trigger* nightbane_ground_phase_position_boss(PlayerbotAI* botAI) { return new NightbaneGroundPhasePositionBossTrigger(botAI); }
    static Trigger* nightbane_ground_phase_rotate_ranged_positions(PlayerbotAI* botAI) { return new NightbaneGroundPhaseRotateRangedPositionsTrigger(botAI); }
    static Trigger* nightbane_control_pet_aggression(PlayerbotAI* botAI) { return new NightbaneControlPetAggressionTrigger(botAI); }
    static Trigger* nightbane_cast_fear_ward_on_main_tank(PlayerbotAI* botAI) { return new NightbaneCastFearWardOnMainTankTrigger(botAI); }
    static Trigger* nightbane_flight_phase_movement(PlayerbotAI* botAI) { return new NightbaneFlightPhaseMovementTrigger(botAI); }
    static Trigger* nightbane_manage_timers_and_trackers(PlayerbotAI* botAI) { return new NightbaneManageTimersAndTrackersTrigger(botAI); }
};

#endif