#ifndef _PLAYERBOT_RAIDZULAMANTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDZULAMANTRIGGERCONTEXT_H

#include "RaidZulAmanTriggers.h"
#include "AiObjectContext.h"

class RaidZulAmanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidZulAmanTriggerContext()
    {
        // Trash
        creators["amani'shi medicine man summoned ward"] =
            &RaidZulAmanTriggerContext::amanishi_medicine_man_summoned_ward;

        // Akil'zon <Eagle Avatar>
        creators["akil'zon pulling boss"] =
            &RaidZulAmanTriggerContext::akilzon_pulling_boss;

        creators["akil'zon boss engaged by main tank"] =
            &RaidZulAmanTriggerContext::akilzon_boss_engaged_by_main_tank;

        creators["akil'zon boss casts static disruption"] =
            &RaidZulAmanTriggerContext::akilzon_boss_casts_static_disruption;

        creators["akil'zon electrical storm has formed"] =
            &RaidZulAmanTriggerContext::akilzon_electrical_storm_has_formed;

        // Nalorakk <Bear Avatar>
        creators["nalorakk pulling boss"] =
            &RaidZulAmanTriggerContext::nalorakk_pulling_boss;

        creators["nalorakk boss casts surge"] =
            &RaidZulAmanTriggerContext::nalorakk_boss_casts_surge;

        creators["nalorakk boss switches forms"] =
            &RaidZulAmanTriggerContext::nalorakk_boss_switches_forms;

        // Jan'alai <Dragonhawk Avatar>
        creators["jan'alai pulling boss"] =
            &RaidZulAmanTriggerContext::janalai_pulling_boss;

        creators["jan'alai boss engaged by main tank"] =
            &RaidZulAmanTriggerContext::janalai_boss_engaged_by_main_tank;

        creators["jan'alai boss casts flame breath"] =
            &RaidZulAmanTriggerContext::janalai_boss_casts_flame_breath;

        creators["jan'alai ranged position tracker bot is not in combat"] =
            &RaidZulAmanTriggerContext::janalai_ranged_position_tracker_bot_is_not_in_combat;

        creators["jan'alai boss summoning fire bombs"] =
            &RaidZulAmanTriggerContext::janalai_boss_summoning_fire_bombs;

        creators["jan'alai amani hatchers spawned"] =
            &RaidZulAmanTriggerContext::janalai_amani_hatchers_spawned;

        // Halazzi <Lynx Avatar>
        creators["halazzi pulling boss"] =
            &RaidZulAmanTriggerContext::halazzi_pulling_boss;

        creators["halazzi boss engaged by main tank"] =
            &RaidZulAmanTriggerContext::halazzi_boss_engaged_by_main_tank;

        creators["halazzi boss summons spirit lynx"] =
            &RaidZulAmanTriggerContext::halazzi_boss_summons_spirit_lynx;

        creators["halazzi determining dps target"] =
            &RaidZulAmanTriggerContext::halazzi_determining_dps_target;

        // Hex Lord Malacrass

        creators["hex lord malacrass pulling boss"] =
            &RaidZulAmanTriggerContext::hex_lord_malacrass_pulling_boss;

        creators["hex lord malacrass determining kill order"] =
            &RaidZulAmanTriggerContext::hex_lord_malacrass_determining_kill_order;

        creators["hex lord malacrass boss has siphoned buff"] =
            &RaidZulAmanTriggerContext::hex_lord_malacrass_boss_has_siphoned_buff;

        creators["hex lord malacrass party member is mind controlled"] =
            &RaidZulAmanTriggerContext::hex_lord_malacrass_party_member_is_mind_controlled;

        // Zul'jin

        creators["zul'jin main tank needs aggro upon pull or phase change"] =
            &RaidZulAmanTriggerContext::zuljin_main_tank_needs_aggro_upon_pull_or_phase_change;

        creators["zul'jin boss engaged by main tank"] =
            &RaidZulAmanTriggerContext::zuljin_boss_engaged_by_main_tank;

        creators["zul'jin boss is channeling whirlwind in troll form"] =
            &RaidZulAmanTriggerContext::zuljin_boss_is_channeling_whirlwind_in_troll_form;

        creators["zul'jin boss casts aoe abilities"] =
            &RaidZulAmanTriggerContext::zuljin_boss_casts_aoe_abilities;

        creators["zul'jin boss is charging players in lynx form"] =
            &RaidZulAmanTriggerContext::zuljin_boss_is_charging_players_in_lynx_form;
    }

private:
    // Trash
    static Trigger* amanishi_medicine_man_summoned_ward(
        PlayerbotAI* botAI) { return new AmanishiMedicineManSummonedWardTrigger(botAI); }

    // Akil'zon <Eagle Avatar>
    static Trigger* akilzon_pulling_boss(
        PlayerbotAI* botAI) { return new AkilzonPullingBossTrigger(botAI); }

    static Trigger* akilzon_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new AkilzonBossEngagedByMainTankTrigger(botAI); }

    static Trigger* akilzon_boss_casts_static_disruption(
        PlayerbotAI* botAI) { return new AkilzonBossCastsStaticDisruptionTrigger(botAI); }

    static Trigger* akilzon_electrical_storm_has_formed(
        PlayerbotAI* botAI) { return new AkilzonElectricalStormHasFormedTrigger(botAI); }

    // Nalorakk <Bear Avatar>
    static Trigger* nalorakk_pulling_boss(
        PlayerbotAI* botAI) { return new NalorakkPullingBossTrigger(botAI); }

    static Trigger* nalorakk_boss_casts_surge(
        PlayerbotAI* botAI) { return new NalorakkBossCastsSurgeTrigger(botAI); }

    static Trigger* nalorakk_boss_switches_forms(
        PlayerbotAI* botAI) { return new NalorakkBossSwitchesFormsTrigger(botAI); }

    // Jan'alai <Dragonhawk Avatar>
    static Trigger* janalai_pulling_boss(
        PlayerbotAI* botAI) { return new JanalaiPullingBossTrigger(botAI); }

    static Trigger* janalai_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new JanalaiBossEngagedByMainTankTrigger(botAI); }

    static Trigger* janalai_boss_casts_flame_breath(
        PlayerbotAI* botAI) { return new JanalaiBossCastsFlameBreathTrigger(botAI); }

    static Trigger* janalai_ranged_position_tracker_bot_is_not_in_combat(
        PlayerbotAI* botAI) { return new JanalaiRangedPositionTrackerBotIsNotInCombatTrigger(botAI); }

    static Trigger* janalai_boss_summoning_fire_bombs(
        PlayerbotAI* botAI) { return new JanalaiBossSummoningFireBombsTrigger(botAI); }

    static Trigger* janalai_amani_hatchers_spawned(
        PlayerbotAI* botAI) { return new JanalaiAmaniHatchersSpawnedTrigger(botAI); }

    // Halazzi <Lynx Avatar>
    static Trigger* halazzi_pulling_boss(
        PlayerbotAI* botAI) { return new HalazziPullingBossTrigger(botAI); }

    static Trigger* halazzi_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new HalazziBossEngagedByMainTankTrigger(botAI); }

    static Trigger* halazzi_boss_summons_spirit_lynx(
        PlayerbotAI* botAI) { return new HalazziBossSummonsSpiritLynxTrigger(botAI); }

    static Trigger* halazzi_determining_dps_target(
        PlayerbotAI* botAI) { return new HalazziDeterminingDpsTargetTrigger(botAI); }

    // Hex Lord Malacrass

    static Trigger* hex_lord_malacrass_pulling_boss(
        PlayerbotAI* botAI) { return new HexLordMalacrassPullingBossTrigger(botAI); }

    static Trigger* hex_lord_malacrass_determining_kill_order(
        PlayerbotAI* botAI) { return new HexLordMalacrassDeterminingKillOrderTrigger(botAI); }

    static Trigger* hex_lord_malacrass_boss_has_siphoned_buff(
        PlayerbotAI* botAI) { return new HexLordMalacrassBossHasSiphonedBuffTrigger(botAI); }

    static Trigger* hex_lord_malacrass_party_member_is_mind_controlled(
        PlayerbotAI* botAI) { return new HexLordMalacrassPartyMemberIsMindControlledTrigger(botAI); }

    // Zul'jin

    static Trigger* zuljin_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new ZuljinBossEngagedByMainTankTrigger(botAI); }

    static Trigger* zuljin_main_tank_needs_aggro_upon_pull_or_phase_change(
        PlayerbotAI* botAI) { return new ZuljinMainTankNeedsAggroUponPullOrPhaseChangeTrigger(botAI); }

    static Trigger* zuljin_boss_is_channeling_whirlwind_in_troll_form(
        PlayerbotAI* botAI) { return new ZuljinBossIsChannelingWhirlwindInTrollFormTrigger(botAI); }

    static Trigger* zuljin_boss_casts_aoe_abilities(
        PlayerbotAI* botAI) { return new ZuljinBossCastsAoeAbilitiesTrigger(botAI); }

    static Trigger* zuljin_boss_is_charging_players_in_lynx_form(
        PlayerbotAI* botAI) { return new ZuljinBossIsChargingPlayersInLynxFormTrigger(botAI); }
};

#endif
