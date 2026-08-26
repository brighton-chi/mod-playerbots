/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "EncounterHelpers.h"
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
#include "ZAActions.h"
#include "ZAHelpers.h"

using namespace ZaHelpers;
using namespace EncounterHelpers;

namespace
{

bool const IsHazardousMovement(Action* action)
{
    return (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action)) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action);
}

}

// General

float ZulAmanAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action))
    {
        return 1.0f;
    }

    constexpr float hazardRadius = 15.0f;

    if (Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin"))
    {
        if (zuljin->GetVictim() == bot)
            return 1.0f;

        if (!zuljin->HasAura(Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND)))
            return 1.0f;

        return bot->GetDistance2d(zuljin) <= hazardRadius ? 0.0f : 1.0f;
    }

    if (Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass"))
    {
        if (malacrass->GetVictim() == bot)
            return 1.0f;

        if (!malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND)))
            return 1.0f;

        return bot->GetDistance2d(malacrass) <= hazardRadius ? 0.0f : 1.0f;
    }

    return 1.0f;
}

float ZulAmanDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "jan'alai") ||
        AI_VALUE2(Unit*, "find target", "akil'zon") ? 0.0f : 1.0f;
}

// Akil'zon <Eagle Avatar>

float AkilzonStayInEyeOfTheStormMultiplier::GetValue(Action* action)
{
    if (!IsHazardousMovement(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return 1.0f;

    if (dynamic_cast<AkilzonMoveToEyeOfTheStormAction*>(action))
        return 1.0f;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return 1.0f;

    return IsInStormWindow(it->second) ? 0.0f : 1.0f;
}

// Nalorakk <Bear Avatar>

float NalorakkDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isTankAction = dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<TankFaceAction*>(action);

    if (!isTankAction && !IsTauntAction(bot, action))
        return 1.0f;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk)
        return 1.0f;

    if (isTankAction)
        return 0.0f;

    // IsTauntAction
    bool const inBearForm = IsNalorakkInBearForm(nalorakk);

    if (!inBearForm && PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return 0.0f;

    return inBearForm && PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
}

float NalorakkControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "nalorakk") ? 0.0f : 1.0f;
}

// Jan'alai <Dragonhawk Avatar>

float JanalaiDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isTankFaceAction = dynamic_cast<TankFaceAction*>(action);
    if (!isTankFaceAction && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return 1.0f;

    if (isTankFaceAction)
        return 0.0f;

    // TankAssistAction
    return PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
}

float JanalaiStayAwayFromFireBombsMultiplier::GetValue(Action* action)
{
    if (!IsHazardousMovement(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return 1.0f;

    if (dynamic_cast<JanalaiAvoidFireBombsAction*>(action))
        return 1.0f;

    return HasFireBombNearby(bot) ? 0.0f : 1.0f;
}

float JanalaiDoNotCrowdControlHatchersMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CastCrowdControlSpellAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "amani'shi hatcher") ? 0.0f : 1.0f;
}

float JanalaiDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastBloodlustAction*>(action) &&
        !dynamic_cast<CastHeroismAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "amani dragonhawk hatchling") ? 1.0f : 0.0f;
}

// Halazzi <Lynx Avatar>

float HalazziDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<TankFaceAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "halazzi") ? 0.0f : 1.0f;
}

float HalazziControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "halazzi") ? 0.0f : 1.0f;
}

float HalazziDisableAutoDpsTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot))
        return false;

    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "halazzi") ? 0.0f : 1.0f;
}


// Hex Lord Malacrass

// Unstable Affliction is considered a magic effect, not a curse
float HexLordMalacrassUnstableAfflictionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() == CLASS_PRIEST)
    {
        if (!dynamic_cast<CastDispelMagicOnPartyAction*>(action) &&
            !dynamic_cast<CastDispelMagicAction*>(action) &&
            !dynamic_cast<CastMassDispelAction*>(action))
        {
            return 1.0f;
        }
    }
    else if (bot->getClass() == CLASS_PALADIN)
    {
        if (!dynamic_cast<CastCleanseMagicOnPartyAction*>(action) &&
            !dynamic_cast<CastCleanseMagicAction*>(action))
        {
            return 1.0f;
        }
    }
    else
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "hex lord malacrass"))
        return 1.0f;

    Unit* target = AI_VALUE2(Unit*, "party member to dispel", DISPEL_MAGIC);
    if (!target)
        return 1.0f;

    return target->HasAura(Id(ZaSpells::SPELL_UNSTABLE_AFFLICTION)) ? 0.0f : 1.0f;
}

float HexLordMalacrassSpellReflectionMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsCaster(bot))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass)
        return 1.0f;

    return malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_SPELL_REFLECTION)) ? 0.0f : 1.0f;
}

// Zul'jin

float ZuljinDisableTankFaceMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankFaceAction*>(action))
        return 1.0f;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return 1.0f;

    return zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK)) ? 1.0f : 0.0f;
}

float ZuljinEagleDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return 1.0f;

    return zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ? 0.0f : 1.0f;
}

float ZuljinDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastBloodlustAction*>(action) &&
        !dynamic_cast<CastHeroismAction*>(action))
    {
        return 1.0f;
    }

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return 1.0f;

    return zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ? 1.0f : 0.0f;
}
