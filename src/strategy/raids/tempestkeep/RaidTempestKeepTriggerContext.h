#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H

#include "RaidTempestKeepTriggers.h"
#include "AiObjectContext.h"

class RaidTempestKeepTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTempestKeepTriggerContext()
    {
        creators["crimson hand centurion casts arcane volley"] = &RaidTempestKeepTriggerContext::crimson_hand_centurion_casts_arcane_volley;

        creators["alar pulling boss"] = &RaidTempestKeepTriggerContext::alar_pulling_boss;
        creators["alar engaged by tanks in phase 1"] = &RaidTempestKeepTriggerContext::alar_engaged_by_tanks_in_phase_1;
        creators["alar boss engaged by melee dps"] = &RaidTempestKeepTriggerContext::alar_boss_engaged_by_melee_dps;
        creators["alar boss engaged by ranged dps"] = &RaidTempestKeepTriggerContext::alar_boss_engaged_by_ranged_dps;
        creators["alar boss engaged by healer"] = &RaidTempestKeepTriggerContext::alar_boss_engaged_by_healer;
        creators["alar embers of alar spawned"] = &RaidTempestKeepTriggerContext::alar_embers_of_alar_spawned;
        creators["alar incoming flame quills"] = &RaidTempestKeepTriggerContext::alar_incoming_flame_quills;
        creators["alar rising from the ashes"] = &RaidTempestKeepTriggerContext::alar_rising_from_the_ashes;
        creators["alar boss tank armor was melted"] = &RaidTempestKeepTriggerContext::alar_boss_tank_armor_was_melted;
        creators["alar boss is performing dive bomb sequence"] = &RaidTempestKeepTriggerContext::alar_boss_is_performing_dive_bomb_sequence;
        creators["alar phase 2 encounter is at room center"] = &RaidTempestKeepTriggerContext::alar_phase_2_encounter_is_at_room_center;

        creators["void reaver boss engaged by tank"] = &RaidTempestKeepTriggerContext::void_reaver_boss_engaged_by_tank;
        creators["void reaver boss launches arcane orbs"] = &RaidTempestKeepTriggerContext::void_reaver_boss_launches_arcane_orbs;
        creators["void reaver tanks lost aggro"] = &RaidTempestKeepTriggerContext::void_reaver_tanks_lost_aggro;

        creators["high astromancer solarian phase 1 and 2 movement"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_phase_1_and_2_movement;
        creators["high astromancer solarian bot has wrath of the astromancer"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_bot_has_wrath_of_the_astromancer;
        creators["high astromancer solarian solarium priests spawned"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_solarium_priests_spawned;
        creators["high astromancer solarian transformed into voidwalker"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_transformed_into_voidwalker;
        creators["high astromancer solarian boss casts psychic scream"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_boss_casts_psychic_scream;

        creators["kaelthas sunstrider thaladred is fixated on bot"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_thaladred_is_fixated_on_bot;
        creators["kaelthas sunstrider sanguinar engaged by main tank"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_sanguinar_engaged_by_main_tank;
        creators["kaelthas sunstrider sanguinar casts bellowing roar"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_sanguinar_casts_bellowing_roar;
        creators["kaelthas sunstrider capernian requires a warlock tank"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_capernian_requires_a_warlock_tank;
        creators["kaelthas sunstrider capernian engaged by warlock tank"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_capernian_engaged_by_warlock_tank;
        creators["kaelthas sunstrider capernian casts arcane burst"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_capernian_casts_arcane_burst;
        creators["kaelthas sunstrider telonicus engaged by first assist tank"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_telonicus_engaged_by_first_assist_tank;
        creators["kaelthas sunstrider pulling tankable advisors"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_pulling_tankable_advisors;
        creators["kaelthas sunstrider waiting for tanks to get aggro on advisors"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_waiting_for_tanks_to_get_aggro_on_advisors;
        creators["kaelthas sunstrider legendary weapons are alive"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_are_alive;
        creators["kaelthas sunstrider devastation channels whirlwind"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_devastation_channels_whirlwind;
        creators["kaelthas sunstrider netherstrand longbow fires multishot"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_netherstrand_longbow_fires_multishot;
        creators["kaelthas sunstrider legendary weapons are dead and lootable"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_are_dead_and_lootable;
        creators["kaelthas sunstrider legendary weapons are equipped"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_are_equipped;
        creators["kaelthas sunstrider legendary weapons were lost"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_legendary_weapons_were_lost;
        creators["kaelthas sunstrider determining advisor kill order"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_determining_advisor_kill_order;
        creators["kaelthas sunstrider boss engaged by main tank"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_boss_engaged_by_main_tank;
        creators["kaelthas sunstrider flame strike appeared under bot"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_flame_strike_appeared_under_bot;
        creators["kaelthas sunstrider raid member is mind controlled"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_raid_member_is_mind_controlled;
        creators["kaelthas sunstrider phoenixes and eggs are spawning"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_phoenixes_and_eggs_are_spawning;
        creators["kaelthas sunstrider boss is casting pyroblast"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_boss_is_casting_pyroblast;
        creators["kaelthas sunstrider boss is manipulating gravity"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_boss_is_manipulating_gravity;
    }

private:
    static Trigger* crimson_hand_centurion_casts_arcane_volley(PlayerbotAI* botAI) { return new CrimsonHandCenturionCastsArcaneVolleyTrigger(botAI); }

    static Trigger* alar_pulling_boss(PlayerbotAI* botAI) { return new AlarPullingBossTrigger(botAI); }
    static Trigger* alar_engaged_by_tanks_in_phase_1(PlayerbotAI* botAI) { return new AlarEngagedByTanksInPhase1Trigger(botAI); }
    static Trigger* alar_boss_engaged_by_melee_dps(PlayerbotAI* botAI) { return new AlarBossEngagedByMeleeDpsTrigger(botAI); }
    static Trigger* alar_boss_engaged_by_ranged_dps(PlayerbotAI* botAI) { return new AlarBossEngagedByRangedDpsTrigger(botAI); }
    static Trigger* alar_boss_engaged_by_healer(PlayerbotAI* botAI) { return new AlarBossEngagedByHealerTrigger(botAI); }
    static Trigger* alar_embers_of_alar_spawned(PlayerbotAI* botAI) { return new AlarEmbersOfAlarSpawnedTrigger(botAI); }
    static Trigger* alar_incoming_flame_quills(PlayerbotAI* botAI) { return new AlarIncomingFlameQuillsTrigger(botAI); }
    static Trigger* alar_rising_from_the_ashes(PlayerbotAI* botAI) { return new AlarRisingFromTheAshesTrigger(botAI); }
    static Trigger* alar_boss_tank_armor_was_melted(PlayerbotAI* botAI) { return new AlarBossTankArmorWasMeltedTrigger(botAI); }
    static Trigger* alar_boss_is_performing_dive_bomb_sequence(PlayerbotAI* botAI) { return new AlarBossIsPerformingDiveBombSequenceTrigger(botAI); }
    static Trigger* alar_phase_2_encounter_is_at_room_center(PlayerbotAI* botAI) { return new AlarPhase2EncounterIsAtRoomCenterTrigger(botAI); }

    static Trigger* void_reaver_boss_engaged_by_tank(PlayerbotAI* botAI) { return new VoidReaverBossEngagedByTankTrigger(botAI); }
    static Trigger* void_reaver_boss_launches_arcane_orbs(PlayerbotAI* botAI) { return new VoidReaverBossLaunchesArcaneOrbsTrigger(botAI); }
    static Trigger* void_reaver_tanks_lost_aggro(PlayerbotAI* botAI) { return new VoidReaverTanksLostAggroTrigger(botAI); }

    static Trigger* high_astromancer_solarian_phase_1_and_2_movement(PlayerbotAI* botAI) { return new HighAstromancerSolarianPhase1And2MovementTrigger(botAI); }
    static Trigger* high_astromancer_solarian_bot_has_wrath_of_the_astromancer(PlayerbotAI* botAI) { return new HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger(botAI); }
    static Trigger* high_astromancer_solarian_solarium_priests_spawned(PlayerbotAI* botAI) { return new HighAstromancerSolarianSolariumPriestsSpawnedTrigger(botAI); }
    static Trigger* high_astromancer_solarian_transformed_into_voidwalker(PlayerbotAI* botAI) { return new HighAstromancerSolarianTransformedIntoVoidwalkerTrigger(botAI); }
    static Trigger* high_astromancer_solarian_boss_casts_psychic_scream(PlayerbotAI* botAI) { return new HighAstromancerSolarianBossCastsPsychicScreamTrigger(botAI); }

    static Trigger* kaelthas_sunstrider_thaladred_is_fixated_on_bot(PlayerbotAI* botAI) { return new KaelthasSunstriderThaladredIsFixatedOnBotTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_sanguinar_engaged_by_main_tank(PlayerbotAI* botAI) { return new KaelthasSunstriderSanguinarEngagedByMainTankTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_sanguinar_casts_bellowing_roar(PlayerbotAI* botAI) { return new KaelthasSunstriderSanguinarCastsBellowingRoarTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_capernian_requires_a_warlock_tank(PlayerbotAI* botAI) { return new KaelthasSunstriderCapernianRequiresAWarlockTankTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_capernian_engaged_by_warlock_tank(PlayerbotAI* botAI) { return new KaelthasSunstriderCapernianEngagedByWarlockTankTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_capernian_casts_arcane_burst(PlayerbotAI* botAI) { return new KaelthasSunstriderCapernianCastsArcaneBurstTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_telonicus_engaged_by_first_assist_tank(PlayerbotAI* botAI) { return new KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_pulling_tankable_advisors(PlayerbotAI* botAI) { return new KaelthasSunstriderPullingTankableAdvisorsTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_waiting_for_tanks_to_get_aggro_on_advisors(PlayerbotAI* botAI) { return new KaelthasSunstriderWaitingForTanksToGetAggroOnAdvisorsTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_legendary_weapons_are_alive(PlayerbotAI* botAI) { return new KaelthasSunstriderLegendaryWeaponsAreAliveTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_devastation_channels_whirlwind(PlayerbotAI* botAI) { return new KaelthasSunstriderDevastationChannelsWhirlwindTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_netherstrand_longbow_fires_multishot(PlayerbotAI* botAI) { return new KaelthasSunstriderNetherstrandLongbowFiresMultiShotTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_legendary_weapons_are_dead_and_lootable(PlayerbotAI* botAI) { return new KaelthasSunstriderLegendaryWeaponsAreDeadAndLootableTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_legendary_weapons_are_equipped(PlayerbotAI* botAI) { return new KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_legendary_weapons_were_lost(PlayerbotAI* botAI) { return new KaelthasSunstriderLegendaryWeaponsWereLostTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_determining_advisor_kill_order(PlayerbotAI* botAI) { return new KaelthasSunstriderDeterminingAdvisorKillOrderTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new KaelthasSunstriderBossEngagedByMainTankTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_flame_strike_appeared_under_bot(PlayerbotAI* botAI) { return new KaelthasSunstriderFlameStrikeAppearedUnderBotTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_raid_member_is_mind_controlled(PlayerbotAI* botAI) { return new KaelthasSunstriderRaidMemberIsMindControlledTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_phoenixes_and_eggs_are_spawning(PlayerbotAI* botAI) { return new KaelthasSunstriderPhoenixesAndEggsAreSpawningTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_boss_is_casting_pyroblast(PlayerbotAI* botAI) { return new KaelthasSunstriderBossIsCastingPyroblastTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_boss_is_manipulating_gravity(PlayerbotAI* botAI) { return new KaelthasSunstriderBossIsManipulatingGravityTrigger(botAI); }
};

#endif
