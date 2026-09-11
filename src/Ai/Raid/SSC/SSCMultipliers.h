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

// Shared Bosses

// For Lady Vashj, Fathom-Lord Karathress, and Hydross
class SscControlMisdirectionMultiplier : public SscEncounterMultiplier
{
public:
    SscControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "ssc control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstableDisableTankReachMultiplier : public SscEncounterMultiplier
{
public:
    HydrossTheUnstableDisableTankReachMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "hydross the unstable disable tank reach") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HydrossTheUnstableDisableAutoTargetAndMoveMultiplier : public SscEncounterMultiplier
{
public:
    HydrossTheUnstableDisableAutoTargetAndMoveMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "hydross the unstable disable auto target and move") {}

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

// The Lurker Below

class TheLurkerBelowStayAwayFromSpoutMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowStayAwayFromSpoutMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below stay away from spout") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TheLurkerBelowMaintainRangedSpreadMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowMaintainRangedSpreadMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below maintain ranged spread") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TheLurkerBelowDisableTankAssistMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowDisableTankAssistMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below disable tank assist") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Leotheras the Blind

class LeotherasTheBlindAvoidWhirlwindMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind avoid whirlwind") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindDisableTankActionsMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind disable tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindMeleeAvoidChaosBlastMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindMeleeAvoidChaosBlastMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind melee avoid chaos blast") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindFocusOnInnerDemonMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindFocusOnInnerDemonMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind focus on inner demon") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindWaitForDpsMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindWaitForDpsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindDelayBloodlustAndHeroismMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind delay bloodlust and heroism") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Fathom-Lord Karathress

class FathomLordKarathressDisableTankActionsMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress disable tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressDisableAutoTargetMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressDisableAutoTargetMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress disable auto target") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressDisableAoeMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressDisableAoeMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress disable aoe") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressWaitForDpsMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressWaitForDpsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressMaintainPositionMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressMaintainPositionMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress maintain position") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier : public SscEncounterMultiplier
{
public:
    MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "morogrim tidewalker delay bloodlust and heroism") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MorogrimTidewalkerDisableTankActionsMultiplier : public SscEncounterMultiplier
{
public:
    MorogrimTidewalkerDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "morogrim tidewalker disable tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MorogrimTidewalkerMaintainPhase2StackingMultiplier : public SscEncounterMultiplier
{
public:
    MorogrimTidewalkerMaintainPhase2StackingMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "morogrim tidewalker maintain phase2 stacking") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjDelayCooldownsMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj delay cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjSetGroundingTotemMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjSetGroundingTotemMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj set grounding totem") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjMaintainPhase1RangedSpreadMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjMaintainPhase1RangedSpreadMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj maintain phase1 ranged spread") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjStaticChargeStayAwayFromGroupMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjStaticChargeStayAwayFromGroupMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj static charge stay away from group") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjDoNotLootTheTaintedCoreMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjDoNotLootTheTaintedCoreMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj do not loot the tainted core") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjCorePassersPrioritizePositioningMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjCorePassersPrioritizePositioningMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj core passers prioritize positioning") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjDisableAutoTargetAndMoveMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjDisableAutoTargetAndMoveMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj disable auto target and move") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjSaveHandOfFreedomMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjSaveHandOfFreedomMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj save hand of freedom") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
