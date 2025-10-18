#ifndef _PLAYERBOT_RAIDKARAZHANACTIONS_CONTEXT_H
#define _PLAYERBOT_RAIDKARAZHANACTIONS_CONTEXT_H

#include "RaidKarazhanActions.h"
#include "NamedObjectContext.h"

class RaidKarazhanActionContext : public NamedObjectContext<Action>
{
public:
    RaidKarazhanActionContext()
    {
        creators["attumen the huntsman main tank"] = &RaidKarazhanActionContext::attumen_the_huntsman_main_tank;
        creators["attumen the huntsman split bosses"] = &RaidKarazhanActionContext::attumen_the_huntsman_split_bosses;
        creators["attumen the huntsman stack behind"] = &RaidKarazhanActionContext::attumen_the_huntsman_stack_behind;
        creators["attumen the huntsman manage timer"] = &RaidKarazhanActionContext::attumen_the_huntsman_manage_timer;

        creators["moroes mark target"] = &RaidKarazhanActionContext::moroes_mark_target;

        creators["maiden of virtue main tank movement"] = &RaidKarazhanActionContext::maiden_of_virtue_main_tank_movement;
        creators["maiden of virtue position ranged"] = &RaidKarazhanActionContext::maiden_of_virtue_position_ranged;

        creators["big bad wolf position boss"] = &RaidKarazhanActionContext::big_bad_wolf_position_boss;
        creators["big bad wolf run away"] = &RaidKarazhanActionContext::big_bad_wolf_run_away;

        creators["romulo and julianne mark target"] = &RaidKarazhanActionContext::romulo_and_julianne_mark_target;

        creators["wizard of oz mark target"] = &RaidKarazhanActionContext::wizard_of_oz_mark_target;
        creators["wizard of oz scorch strawman"] = &RaidKarazhanActionContext::wizard_of_oz_scorch_strawman;

        creators["the curator mark astral flare"] = &RaidKarazhanActionContext::the_curator_mark_astral_flare;
        creators["the curator position boss"] = &RaidKarazhanActionContext::the_curator_position_boss;
        creators["the curator spread ranged"] = &RaidKarazhanActionContext::the_curator_spread_ranged;

        creators["terestian illhoof mark target"] = &RaidKarazhanActionContext::terestian_illhoof_mark_target;

        creators["shade of aran arcane explosion run away"] = &RaidKarazhanActionContext::shade_of_aran_arcane_explosion_run_away;
        creators["shade of aran flame wreath stop movement"] = &RaidKarazhanActionContext::shade_of_aran_flame_wreath_stop_movement;
        creators["shade of aran mark conjured elemental"] = &RaidKarazhanActionContext::shade_of_aran_mark_conjured_elemental;
        creators["shade of aran spread ranged"] = &RaidKarazhanActionContext::shade_of_aran_spread_ranged;

        creators["netherspite block red beam"] = &RaidKarazhanActionContext::netherspite_block_red_beam;
        creators["netherspite block blue beam"] = &RaidKarazhanActionContext::netherspite_block_blue_beam;
        creators["netherspite block green beam"] = &RaidKarazhanActionContext::netherspite_block_green_beam;
        creators["netherspite avoid beam and void zone"] = &RaidKarazhanActionContext::netherspite_avoid_beam_and_void_zone;
        creators["netherspite banish phase avoid void zone"] = &RaidKarazhanActionContext::netherspite_banish_phase_avoid_void_zone;
        creators["netherspite manage timers"] = &RaidKarazhanActionContext::netherspite_manage_timers;

        creators["prince malchezaar enfeebled avoid hazard"] = &RaidKarazhanActionContext::prince_malchezaar_enfeebled_avoid_hazard;
        creators["prince malchezaar non tank avoid infernal"] = &RaidKarazhanActionContext::prince_malchezaar_non_tank_avoid_infernal;
        // creators["prince malchezaar non tank avoid hazard"] = &RaidKarazhanActionContext::prince_malchezaar_non_tank_avoid_hazard;
        creators["prince malchezaar main tank movement"] = &RaidKarazhanActionContext::prince_malchezaar_main_tank_movement;

        creators["nightbane ground phase position boss"] = &RaidKarazhanActionContext::nightbane_ground_phase_position_boss;
        creators["nightbane ground phase rotate ranged positions"] = &RaidKarazhanActionContext::nightbane_ground_phase_rotate_ranged_positions;
        creators["nightbane cast fear ward on main tank"] = &RaidKarazhanActionContext::nightbane_cast_fear_ward_on_main_tank;
        creators["nightbane control pet aggression"] = &RaidKarazhanActionContext::nightbane_control_pet_aggression;
        creators["nightbane flight phase movement"] = &RaidKarazhanActionContext::nightbane_flight_phase_movement;
        creators["nightbane manage timers and trackers"] = &RaidKarazhanActionContext::nightbane_manage_timers_and_trackers;
    }

private:
    static Action* attumen_the_huntsman_main_tank(PlayerbotAI* botAI) { return new AttumenTheHuntsmanMainTankAction(botAI); }
    static Action* attumen_the_huntsman_split_bosses(PlayerbotAI* botAI) { return new AttumenTheHuntsmanSplitBossesAction(botAI); }
    static Action* attumen_the_huntsman_stack_behind(PlayerbotAI* botAI) { return new AttumenTheHuntsmanStackBehindAction(botAI); }
    static Action* attumen_the_huntsman_manage_timer(PlayerbotAI* botAI) { return new AttumenTheHuntsmanManageTimerAction(botAI); }

    static Action* moroes_mark_target(PlayerbotAI* botAI) { return new MoroesMarkTargetAction(botAI); }

    static Action* maiden_of_virtue_main_tank_movement(PlayerbotAI* botAI) { return new MaidenOfVirtueMainTankMovementAction(botAI); }
    static Action* maiden_of_virtue_position_ranged(PlayerbotAI* botAI) { return new MaidenOfVirtuePositionRangedAction(botAI); }

    static Action* big_bad_wolf_position_boss(PlayerbotAI* botAI) { return new BigBadWolfPositionBossAction(botAI); }
    static Action* big_bad_wolf_run_away(PlayerbotAI* botAI) { return new BigBadWolfRunAwayAction(botAI); }

    static Action* romulo_and_julianne_mark_target(PlayerbotAI* botAI) { return new RomuloAndJulianneMarkTargetAction(botAI); }

    static Action* wizard_of_oz_mark_target(PlayerbotAI* botAI) { return new WizardOfOzMarkTargetAction(botAI); }
    static Action* wizard_of_oz_scorch_strawman(PlayerbotAI* botAI) { return new WizardOfOzScorchStrawmanAction(botAI); }

    static Action* the_curator_mark_astral_flare(PlayerbotAI* botAI) { return new TheCuratorMarkAstralFlareAction(botAI); }
    static Action* the_curator_position_boss(PlayerbotAI* botAI) { return new TheCuratorPositionBossAction(botAI); }
    static Action* the_curator_spread_ranged(PlayerbotAI* botAI) { return new TheCuratorSpreadRangedAction(botAI); }

    static Action* terestian_illhoof_mark_target(PlayerbotAI* botAI) { return new TerestianIllhoofMarkTargetAction(botAI); }

    static Action* shade_of_aran_arcane_explosion_run_away(PlayerbotAI* botAI) { return new ShadeOfAranArcaneExplosionRunAwayAction(botAI); }
    static Action* shade_of_aran_flame_wreath_stop_movement(PlayerbotAI* botAI) { return new ShadeOfAranFlameWreathStopMovementAction(botAI); }
    static Action* shade_of_aran_mark_conjured_elemental(PlayerbotAI* botAI) { return new ShadeOfAranMarkConjuredElementalAction(botAI); }
    static Action* shade_of_aran_spread_ranged(PlayerbotAI* botAI) { return new ShadeOfAranSpreadRangedAction(botAI); }

    static Action* netherspite_block_red_beam(PlayerbotAI* botAI) { return new NetherspiteBlockRedBeamAction(botAI); }
    static Action* netherspite_block_blue_beam(PlayerbotAI* botAI) { return new NetherspiteBlockBlueBeamAction(botAI); }
    static Action* netherspite_block_green_beam(PlayerbotAI* botAI) { return new NetherspiteBlockGreenBeamAction(botAI); }
    static Action* netherspite_avoid_beam_and_void_zone(PlayerbotAI* botAI) { return new NetherspiteAvoidBeamAndVoidZoneAction(botAI); }
    static Action* netherspite_banish_phase_avoid_void_zone(PlayerbotAI* botAI) { return new NetherspiteBanishPhaseAvoidVoidZoneAction(botAI); }
    static Action* netherspite_manage_timers(PlayerbotAI* botAI) { return new NetherspiteManageTimersAction(botAI); }

    static Action* prince_malchezaar_enfeebled_avoid_hazard(PlayerbotAI* botAI) { return new PrinceMalchezaarEnfeebledAvoidHazardAction(botAI); }
    static Action* prince_malchezaar_non_tank_avoid_infernal(PlayerbotAI* botAI) { return new PrinceMalchezaarNonTankAvoidInfernalAction(botAI); }
    // static Action* prince_malchezaar_non_tank_avoid_hazard(PlayerbotAI* botAI) { return new PrinceMalchezaarNonTankAvoidHazardAction(botAI); }
    static Action* prince_malchezaar_main_tank_movement(PlayerbotAI* botAI) { return new PrinceMalchezaarMainTankMovementAction(botAI); }

    static Action* nightbane_ground_phase_position_boss(PlayerbotAI* botAI) { return new NightbaneGroundPhasePositionBossAction(botAI); }
    static Action* nightbane_ground_phase_rotate_ranged_positions(PlayerbotAI* botAI) { return new NightbaneGroundPhaseRotateRangedPositionsAction(botAI); }
    static Action* nightbane_cast_fear_ward_on_main_tank(PlayerbotAI* botAI) { return new NightbaneCastFearWardOnMainTankAction(botAI); }
    static Action* nightbane_control_pet_aggression(PlayerbotAI* botAI) { return new NightbaneControlPetAggressionAction(botAI); }
    static Action* nightbane_flight_phase_movement(PlayerbotAI* botAI) { return new NightbaneFlightPhaseMovementAction(botAI); }
    static Action* nightbane_manage_timers_and_trackers(PlayerbotAI* botAI) { return new NightbaneManageTimersAndTrackersAction(botAI); }
};

#endif
