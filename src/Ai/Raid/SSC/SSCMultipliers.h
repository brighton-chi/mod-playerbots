/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCMULTIPLIERS_H
#define PLAYERBOTS_SSCMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "SSCHelpers.h"
#include <string>

class SscEncounterMultiplier : public Multiplier
{
public:
    SscEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, SscHelpers::SSC_MAP_ID)
            ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

// Trash

class UnderbogColossusEscapeToxicPoolMultiplier : public Multiplier
{
public:
    UnderbogColossusEscapeToxicPoolMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "underbog colossus escape toxic pool") {}
    float GetValue(Action* action) override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstableDisableTankActionsMultiplier
    : public SscEncounterMultiplier
{
public:
    HydrossTheUnstableDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(
            botAI, "hydross the unstable disable tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HydrossTheUnstableWaitForDpsMultiplier : public SscEncounterMultiplier
{
public:
    HydrossTheUnstableWaitForDpsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "hydross the unstable wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HydrossTheUnstableControlMisdirectionMultiplier : public SscEncounterMultiplier
{
public:
    HydrossTheUnstableControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(
            botAI, "hydross the unstable control misdirection") {}
    float GetValue(Action* action) override;
};

// The Lurker Below

class TheLurkerBelowStayAwayFromSpoutMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowStayAwayFromSpoutMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below stay away from spout") {}
    float GetValue(Action* action) override;
};

class TheLurkerBelowMaintainRangedSpreadMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowMaintainRangedSpreadMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(
            botAI, "the lurker below maintain ranged spread") {}
    float GetValue(Action* action) override;
};

class TheLurkerBelowDisableTankAssistMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowDisableTankAssistMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below disable tank assist") {}
    float GetValue(Action* action) override;
};

// Leotheras the Blind

class LeotherasTheBlindAvoidWhirlwindMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind avoid whirlwind") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindDisableTankActionsMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind disable tank actions") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind melee dps avoid chaos blast") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindFocusOnInnerDemonMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindFocusOnInnerDemonMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind focus on inner demon") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindWaitForDpsMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindWaitForDpsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind wait for dps") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindDelayBloodlustAndHeroismMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind delay bloodlust and heroism") {}
    float GetValue(Action* action) override;
};

// Fathom-Lord Karathress

class FathomLordKarathressDisableTankActionsMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress disable tank actions") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressDisableAoeMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressDisableAoeMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress disable aoe") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressControlMisdirectionMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress control misdirection") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressWaitForDpsMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressWaitForDpsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress wait for dps") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressCaribdisTankHealerMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressCaribdisTankHealerMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress caribdis tank healer") {}
    float GetValue(Action* action) override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier : public SscEncounterMultiplier
{
public:
    MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "morogrim tidewalker delay bloodlust and heroism") {}
    float GetValue(Action* action) override;
};

class MorogrimTidewalkerDisableTankActionsMultiplier : public SscEncounterMultiplier
{
public:
    MorogrimTidewalkerDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "morogrim tidewalker disable tank actions") {}
    float GetValue(Action* action) override;
};

class MorogrimTidewalkerMaintainPhase2StackingMultiplier : public SscEncounterMultiplier
{
public:
    MorogrimTidewalkerMaintainPhase2StackingMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "morogrim tidewalker maintain phase2 stacking") {}
    float GetValue(Action* action) override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjDelayCooldownsMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj delay cooldowns") {}
    float GetValue(Action* action) override;
};

class LadyVashjMainTankGroupShamanUseGroundingTotemMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjMainTankGroupShamanUseGroundingTotemMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj main tank group shaman use grounding totem") {}
    float GetValue(Action* action) override;
};

class LadyVashjMaintainPhase1RangedSpreadMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjMaintainPhase1RangedSpreadMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj maintain phase1 ranged spread") {}
    float GetValue(Action* action) override;
};

class LadyVashjStaticChargeStayAwayFromGroupMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjStaticChargeStayAwayFromGroupMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj static charge stay away from group") {}
    float GetValue(Action* action) override;
};

class LadyVashjDoNotLootTheTaintedCoreMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjDoNotLootTheTaintedCoreMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj do not loot the tainted core") {}
    float GetValue(Action* action) override;
};

class LadyVashjCorePassersPrioritizePositioningMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjCorePassersPrioritizePositioningMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj core passers prioritize positioning") {}
    float GetValue(Action* action) override;
};

class LadyVashjDisableAutomaticTargetingAndMovementModifier : public SscEncounterMultiplier
{
public:
    LadyVashjDisableAutomaticTargetingAndMovementModifier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj disable automatic targeting and movement") {}
    float GetValue(Action* action) override;
};

#endif
