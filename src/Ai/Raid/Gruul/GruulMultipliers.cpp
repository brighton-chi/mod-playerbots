/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulMultipliers.h"
#include "ChooseTargetActions.h"
#include "GruulActions.h"
#include "GruulHelpers.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

using namespace GruulHelpers;

float GruulsLairDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (gruul && gruul->GetHealthPct() > 95.0f)
        return 0.0f;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (blindeye && blindeye->GetHealthPct() > 75.0f)
        return 0.0f;

    return 1.0f;
}

float HighKingMaulgarControlTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "high king maulgar"))
        return 0.0f;

    return 1.0f;
}

float HighKingMaulgarDontTauntKigglerMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isAoeThreat = IsAoeThreatAction(bot, action);

    if (!IsTauntAction(bot, action) && !isAoeThreat)
        return 1.0f;

    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler)
        return 1.0f;

    // The check for Kiggler presumes that Blindeye and Olm are already dead; aoe threat abilities
    // are okay only when Maulgar and Krosh are left
    if (isAoeThreat)
        return 0.0f;

    // Kiggler is the only ogre for which taunting is a problem because he is the only one that is
    // both (1) tanked by a non-tank and (2) attacked by tanks (after Blindeye and Olm are down)
    if (AI_VALUE(Unit*, "current target") == kiggler)
        return 0.0f;

    return 1.0f;
}

float HighKingMaulgarDisableDpsAssistMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "high king maulgar"))
        return 0.0f;

    return 1.0f;
}

float HighKingMaulgarAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<HighKingMaulgarRunAwayFromWhirlwindAction*>(action))
        return 1.0f;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar || !maulgar->HasAura(Id(GruulSpells::SPELL_WHIRLWIND)))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    if (bot->GetDistance2d(maulgar) < 15.0f)
        return 0.0f;

    return 1.0f;
}

float HighKingMaulgarControlHunterActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    bool const isMainTankMisdirect = dynamic_cast<CastMisdirectionOnMainTankAction*>(action);
    if (!isMainTankMisdirect && !dynamic_cast<CastArcaneShotAction*>(action))
        return 1.0f;

    // Krosh/Kiggler will be the last to die before Maulgar
    // When only Maulgar is left, the standard Misdirection strategy is fine
    if (isMainTankMisdirect &&
        ((AI_VALUE2(Unit*, "find target", "krosh firehand")) ||
         (AI_VALUE2(Unit*, "find target", "kiggler the crazed"))))
    {
        return 0.0f;
    }

    // Arcane Shot removes Spell Shield, which the mage tank needs to survive
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (krosh && action->GetTarget() == krosh)
        return 0.0f;

    return 1.0f;
}

float HighKingMaulgarControlMageTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_MAGE)
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (!castSpellAction)
        return 1.0f;

    if (castSpellAction->getThreatType() != Action::ActionThreatType::Aoe &&
        !dynamic_cast<CastIceBlockAction*>(action) &&
        !dynamic_cast<CastInvisibilityAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "krosh firehand"))
        return 1.0f;

    if (GetKroshMageTank(bot) == bot)
        return 0.0f;

    return 1.0f;
}

float GruulTheDragonkillerControlTankMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action))
    {
        return 1.0f;
    }

    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (gruul && gruul->GetVictim() == bot)
        return 0.0f;

    return 1.0f;
}

float GruulTheDragonkillerStaySpreadForShatterMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM)))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<GruulTheDragonkillerShatterSpreadAction*>(action))
        return 1.0f;

    return 0.0f;
}
