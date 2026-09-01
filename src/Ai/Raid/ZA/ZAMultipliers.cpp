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

bool IsHazardousMovement(Action* action)
{
    return (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action)) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action);
}

}

// General

float ZulAmanDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->GetMapId() != ZA_MAP_ID) // In case strategy persists outside (e.g., server reset)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    // Every Zul'Aman boss, and nothing else in the instance, runs a BossAI.
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
        return 1.0f;

    if (boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 0.0f;

    // Further restrictions on Bloodlust for Zul'jin and Jan'alai
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastBloodlustAction*>(action) &&
        !dynamic_cast<CastHeroismAction*>(action))
    {
        return 1.0f;
    }

    // Zul'jin: hold until Phase 3 (or later)
    if (boss->GetEntry() == Id(ZaNpcs::NPC_ZULJIN))
    {
        return (boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ||
            boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_LYNX)) ||
            boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK))) ? 1.0f : 0.0f;
    }

    // Jan'alai: hold until time to burn Hatchlings (see comments to the constants in ZAHelpers.h)
    if (boss->GetEntry() == Id(ZaNpcs::NPC_JANALAI))
    {
        if (boss->GetHealthPct() <= JANALAI_HATCH_ALL_HEALTH_PCT)
            return 1.0f;

        return CountAttackersByEntry(botAI, Id(ZaNpcs::NPC_AMANI_DRAGONHAWK_HATCHLING)) >=
            JANALAI_BLOODLUST_HATCHLING_COUNT ? 1.0f : 0.0f;
    }

    return 1.0f;
}

// Malacrass siphoning a Warrior soul and Zul'jin have very similar Whirlwinds
float ZulAmanAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 1.0f;
    }

    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
        return 1.0f;

    uint32 whirlwind = 0;
    switch (boss->GetEntry())
    {
        case Id(ZaNpcs::NPC_ZULJIN):
            whirlwind = Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND);
            break;
        case Id(ZaNpcs::NPC_HEX_LORD_MALACRASS):
            whirlwind = Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND);
            break;
        default:
            return 1.0f;
    }

    if (boss->GetVictim() == bot)
        return 1.0f;

    if (!boss->HasAura(whirlwind))
        return 1.0f;

    return bot->GetExactDist2d(boss) <= ZA_WHIRLWIND_HOLD_DISTANCE ? 0.0f : 1.0f;
}

float ZulAmanDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isTankFace = dynamic_cast<TankFaceAction*>(action);
    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action);
    bool const isTaunt = !isTankFace && !isTankAssist && IsTauntAction(bot, action);

    if (!isTankFace && !isTankAssist && !isTaunt)
        return 1.0f;

    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
        return 1.0f;

    // Nalorakk: A tank swap is used by form so suppress taunts for the opposite-form tank.
    if (boss->GetEntry() == Id(ZaNpcs::NPC_NALORAKK))
    {
        if (!isTaunt)
            return 0.0f;

        bool const isInBearForm = IsNalorakkInBearForm(boss);

        if (!isInBearForm && PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
            return 0.0f;

        return isInBearForm && PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
    }

    // The assist tank picks up the Spirit of the Lynx so disable the main tank from taunting it
    if (boss->GetEntry() == Id(ZaNpcs::NPC_HALAZZI))
    {
        if (!isTaunt)
            return 0.0f;

        if (!PlayerbotAI::IsMainTank(bot))
            return 1.0f;

        Unit* target = action->GetTarget();
        return target && target->GetEntry() == Id(ZaNpcs::NPC_SPIRIT_OF_THE_LYNX) ? 0.0f : 1.0f;
    }

    if (isTaunt)
        return 1.0f;

    // Jan'alai: Allow tank assist for the assist tank to pick up the Hatchlings. Tank face is
    // already disabled as part of ZulAmanDisableCombatFormationMoveMultiplier so the addition here
    // is just belt-and-suspenders for no cost.
    if (boss->GetEntry() == Id(ZaNpcs::NPC_JANALAI))
        return isTankFace || PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;

    // Zul'jin: Allow tank face in Phase 5 for the tank to turn him away from the raid.
    if (boss->GetEntry() == Id(ZaNpcs::NPC_ZULJIN))
    {
        return isTankFace &&
            !boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK)) ? 0.0f : 1.0f;
    }

    // Akil'zon disables only tank face action, and that is already handled by
    // ZulAmanDisableCombatFormationMoveMultiplier.
    return 1.0f;
}

// Nalorakk: Don't Misdirect the boss in troll form to the Main Tank.
// Halazzi: Don't Misdirect the Spirit of the Lynx to the Main Tank.
float ZulAmanControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
        return 1.0f;

    uint32 const entry = boss->GetEntry();
    return entry == Id(ZaNpcs::NPC_NALORAKK) || entry == Id(ZaNpcs::NPC_HALAZZI) ? 0.0f : 1.0f;
}

// CombatFormationMoveAction is the action for the "disperse" command. It is also the parent class
// for SetBehindTargetAction and TankFaceAction.
float ZulAmanDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
        return 1.0f;

    uint32 const entry = boss->GetEntry();
    return entry == Id(ZaNpcs::NPC_JANALAI) || entry == Id(ZaNpcs::NPC_AKILZON) ? 0.0f : 1.0f;
}

// Akil'zon <Eagle Avatar>

float AkilzonStayInEyeOfTheStormMultiplier::GetValue(Action* action)
{
    if (!IsHazardousMovement(action))
        return 1.0f;

    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss || boss->GetEntry() != Id(ZaNpcs::NPC_AKILZON))
        return 1.0f;

    if (dynamic_cast<AkilzonMoveToEyeOfTheStormAction*>(action))
        return 1.0f;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return 1.0f;

    return IsInStormWindow(it->second) ? 0.0f : 1.0f;
}

// Nalorakk <Bear Avatar>

// N/A

// Jan'alai <Dragonhawk Avatar>

float JanalaiStayAwayFromFireBombsMultiplier::GetValue(Action* action)
{
    if (!IsHazardousMovement(action))
        return 1.0f;

    if (dynamic_cast<JanalaiAvoidFireBombsAction*>(action))
        return 1.0f;

    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss || boss->GetEntry() != Id(ZaNpcs::NPC_JANALAI))
        return 1.0f;

    return IsJanalaiBombing(boss) ? 0.0f : 1.0f;
}

float JanalaiDoNotCrowdControlHatchersMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CastCrowdControlSpellAction*>(action))
        return 1.0f;

    Unit* target = action->GetTarget();
    return target && target->GetEntry() == Id(ZaNpcs::NPC_AMANISHI_HATCHER) ? 0.0f : 1.0f;
}

// Halazzi <Lynx Avatar>

float HalazziDisableAutoDpsTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return 1.0f;
    }

    Unit* boss = AI_VALUE(Unit*, "boss target");
    return boss && boss->GetEntry() == Id(ZaNpcs::NPC_HALAZZI) ? 0.0f : 1.0f;
}

// Hex Lord Malacrass

// Weirdly, Unstable Affliction is considered a magic effect, not a curse.
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

    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss || boss->GetEntry() != Id(ZaNpcs::NPC_HEX_LORD_MALACRASS))
        return 1.0f;

    Unit* target = AI_VALUE2(Unit*, "party member to dispel", DISPEL_MAGIC);
    return target && target->HasAura(Id(ZaSpells::SPELL_UNSTABLE_AFFLICTION)) ? 0.0f : 1.0f;
}

float HexLordMalacrassSpellReflectionMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsCaster(bot))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    Unit* boss = AI_VALUE(Unit*, "boss target");
    return boss && boss->GetEntry() == Id(ZaNpcs::NPC_HEX_LORD_MALACRASS) &&
        boss->HasAura(Id(ZaSpells::SPELL_HEX_LORD_SPELL_REFLECTION)) ? 0.0f : 1.0f;
}

// Zul'jin

// AvoidAoeAction is otherwise triggered by the Feather Vortices, and it is useless as they chase
// players at player run speed (the bot runs away when it gets hit, and the vortex just chases the
// bot at the same speed).
float ZuljinEagleDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    Unit* boss = AI_VALUE(Unit*, "boss target");
    return boss && boss->GetEntry() == Id(ZaNpcs::NPC_ZULJIN) &&
        boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ? 0.0f : 1.0f;
}
