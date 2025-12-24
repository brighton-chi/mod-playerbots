#include "RaidZulAmanMultipliers.h"
#include "RaidZulAmanActions.h"
#include "RaidZulAmanHelpers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

using namespace ZulAmanHelpers;

// Akil'zon <Eagle Avatar>

float AkilzonDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) && !botAI->IsRanged(bot))
        return 1.0f;

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (akilzon)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float AkilzonStayInEyeOfTheStormMultiplier::GetValue(Action* action)
{
    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (!akilzon)
        return 1.0f;

    if (!IsElectricalStormWindowActive(akilzon))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Nalorakk <Bear Avatar>

float NalorakkDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    if (!nalorakk->HasAura(SPELL_BEARFORM) && bot->GetVictim() != nullptr &&
        botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<CastTauntAction*>(action) ||
            dynamic_cast<CastGrowlAction*>(action) ||
            dynamic_cast<CastHandOfReckoningAction*>(action) ||
            dynamic_cast<CastDarkCommandAction*>(action))
            return 0.0f;
    }
    else if (nalorakk->HasAura(SPELL_BEARFORM) && bot->GetVictim() != nullptr &&
             botAI->IsMainTank(bot))
    {
        if (dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<CastTauntAction*>(action) ||
            dynamic_cast<CastGrowlAction*>(action) ||
            dynamic_cast<CastHandOfReckoningAction*>(action) ||
            dynamic_cast<CastDarkCommandAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float NalorakkControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (nalorakk)
    {
        if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Jan'alai <Dragonhawk Avatar>

float JanalaiDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (botAI->IsMainTank(bot))
    {
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;
    }

    Unit* hatchling = GetFirstAliveUnitByEntry(botAI, NPC_AMANI_DRAGONHAWK_HATCHLING);
    if (!hatchling)
    {
        if (botAI->IsAssistTank(bot) && bot->GetVictim() != nullptr)
        {
            if (dynamic_cast<TankAssistAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

float JanalaiDisableDispersalMultiplier::GetValue(Action* action)
{
    if (!botAI->IsRanged(bot))
        return 1.0f;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (janalai)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float JanalaiStayAwayFromFireBombsMultiplier::GetValue(Action* action)
{
    if (!AnyNearbyNpcWithEntry(botAI, NPC_FIRE_BOMB))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float JanalaiDoNotCrowdControlHatchersMultiplier::GetValue(Action* action)
{
    Unit* hatcher = AI_VALUE2(Unit*, "find target", "amani'shi hatcher");
    if (hatcher)
    {
        if (dynamic_cast<CastCrowdControlSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float JanalaiDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return 1.0f;

    if (janalai->GetHealthPct() > 35.0f)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Halazzi <Lynx Avatar>

float HalazziDisableTankActionsMultiplier::GetValue(Action* action)
{
    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (!halazzi)
        return 1.0f;

    if (!botAI->IsTank(bot))
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (!botAI->IsMainTank(bot))
    {
        Unit* lynx = AI_VALUE2(Unit*, "find target", "spirit of the lynx");
        if (!lynx)
            return 1.0f;

        if (dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<CastTauntAction*>(action) ||
            dynamic_cast<CastGrowlAction*>(action) ||
            dynamic_cast<CastHandOfReckoningAction*>(action) ||
            dynamic_cast<CastDarkCommandAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float HalazziControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (halazzi)
    {
        if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Hex Lord Malacrass

float HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_PRIEST &&
        bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_WARLOCK)
        return 1.0f;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass)
        return 1.0f;

    bool hasUnstableAffliction = false;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = bot->GetGroup()->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->HasAura(SPELL_UNSTABLE_AFFLICTION))
            {
                hasUnstableAffliction = true;
                break;
            }
        }
    }

    if (!hasUnstableAffliction)
        return 1.0f;

    if (dynamic_cast<CastDevourMagicCleanseAction*>(action) ||
        dynamic_cast<CastDispelMagicAction*>(action) ||
        dynamic_cast<CastDispelMagicOnPartyAction*>(action) ||
        dynamic_cast<CastMassDispelAction*>(action) ||
        dynamic_cast<CastPurgeAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Zul'jin

float ZuljinDisableTankFaceMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (zuljin)
    {
        if (dynamic_cast<TankFaceAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float ZuljinAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (botAI->IsMainTank(bot))
        return 1.0f;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return 1.0f;

    if (zuljin->HasAura(SPELL_WHIRLWIND))
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
            dynamic_cast<CastKillingSpreeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<ReachTargetAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float ZuljinDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (zuljin && !zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE))
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float ZuljinStayCloseToLynxFormMultiplier::GetValue(Action* action)
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (zuljin && zuljin->HasAura(SPELL_SHAPE_OF_THE_LYNX))
    {
        if (dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<FollowAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
