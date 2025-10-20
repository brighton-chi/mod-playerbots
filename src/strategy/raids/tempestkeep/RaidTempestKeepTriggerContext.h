#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H

#include "RaidTempestKeepTriggers.h"
#include "AiObjectContext.h"

class RaidTempestKeepTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTempestKeepTriggerContext()
    {
        creators["void reaver boss engaged by tank"] = &RaidTempestKeepTriggerContext::void_reaver_boss_engaged_by_tank;
        creators["void reaver boss engaged by ranged"] = &RaidTempestKeepTriggerContext::void_reaver_boss_engaged_by_ranged;
        creators["void reaver arcane orb launched"] = &RaidTempestKeepTriggerContext::void_reaver_arcane_orb_launched;
        creators["high astromancer solarian phase 1 and 2 movement"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_phase_1_and_2_movement;
        creators["high astromancer solarian bot has wrath of the astromancer"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_bot_has_wrath_of_the_astromancer;
        creators["high astromancer solarian solarium priests spawned"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_solarium_priests_spawned;
        creators["high astromancer solarian transformed into voidwalker"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_transformed_into_voidwalker;
        creators["high astromancer solarian can cast fear ward on main tank"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_can_cast_fear_ward_on_main_tank;
    }

private:
    static Trigger* void_reaver_boss_engaged_by_tank(PlayerbotAI* botAI) { return new VoidReaverBossEngagedByTankTrigger(botAI); }
    static Trigger* void_reaver_boss_engaged_by_ranged(PlayerbotAI* botAI) { return new VoidReaverBossEngagedByRangedTrigger(botAI); }
    static Trigger* void_reaver_arcane_orb_launched(PlayerbotAI* botAI) { return new VoidReaverArcaneOrbLaunchedTrigger(botAI); }
    static Trigger* high_astromancer_solarian_phase_1_and_2_movement(PlayerbotAI* botAI) { return new HighAstromancerSolarianPhase1And2MovementTrigger(botAI); }
    static Trigger* high_astromancer_solarian_bot_has_wrath_of_the_astromancer(PlayerbotAI* botAI) { return new HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger(botAI); }
    static Trigger* high_astromancer_solarian_solarium_priests_spawned(PlayerbotAI* botAI) { return new HighAstromancerSolarianSolariumPriestsSpawnedTrigger(botAI); }
    static Trigger* high_astromancer_solarian_transformed_into_voidwalker(PlayerbotAI* botAI) { return new HighAstromancerSolarianTransformedIntoVoidwalkerTrigger(botAI); }
    static Trigger* high_astromancer_solarian_can_cast_fear_ward_on_main_tank(PlayerbotAI* botAI) { return new HighAstromancerSolarianCanCastFearWardOnMainTankTrigger(botAI); }
};

#endif
