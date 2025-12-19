#include "RaidGruulsLairMultipliers.h"
#include "RaidGruulsLairActions.h"
#include "RaidGruulsLairHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "WarriorActions.h"

using namespace GruulsLairHelpers;

float HighKingMaulgarWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar)
        return 1.0f;

    if (botAI->IsTank(bot) || IsKroshMageTank(botAI, bot) ||
        IsKigglerMoonkinTank(botAI, bot))
        return 1.0f;

    if (dynamic_cast<HighKingMaulgarMisdirectOlmAndBlindeyeAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 8;

    auto it = maulgarDpsWaitTimer.find(maulgar->GetMap()->GetInstanceId());
    if (it == maulgarDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        if (dynamic_cast<AttackAction*>(action) ||
            (dynamic_cast<CastSpellAction*>(action) &&
             !dynamic_cast<CastHealingSpellAction*>(action)))
             return 0.0f;
    }

    return 1.0f;
}

float HighKingMaulgarDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (maulgar)
    {
        if (dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<TankFaceAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Don't run back in during Whirlwind
float HighKingMaulgarAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (maulgar && maulgar->HasAura(SPELL_WHIRLWIND) &&
        (!kiggler || !kiggler->IsAlive()) &&
        (!krosh || !krosh->IsAlive()) &&
        (!olm || !olm->IsAlive()) &&
        (!blindeye || !blindeye->IsAlive()))
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
            (dynamic_cast<MovementAction*>(action) &&
             !dynamic_cast<AttackAction*>(action) &&
             !dynamic_cast<HighKingMaulgarRunAwayFromWhirlwindAction*>(action)))
             return 0.0f;
    }

    return 1.0f;
}

// Arcane Shot will remove Spell Shield, which the mage tank needs to survive
float HighKingMaulgarDisableArcaneShotOnKroshMultiplier::GetValue(Action* action)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh)
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->GetGUID() == krosh->GetGUID())
    {
        if (dynamic_cast<CastArcaneShotAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float HighKingMaulgarDisableMageTankAoeMultiplier::GetValue(Action* action)
{
    if (IsKroshMageTank(botAI, bot))
    {
        if (dynamic_cast<CastFrostNovaAction*>(action) ||
            dynamic_cast<CastBlizzardAction*>(action) ||
            dynamic_cast<CastConeOfColdAction*>(action) ||
            dynamic_cast<CastFlamestrikeAction*>(action) ||
            dynamic_cast<CastDragonsBreathAction*>(action) ||
            dynamic_cast<CastBlastWaveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float GruulTheDragonkillerDisableTankFaceMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (gruul)
    {
        if (dynamic_cast<TankFaceAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float GruulTheDragonkillerGroundSlamMultiplier::GetValue(Action* action)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return 1.0f;

    if (bot->HasAura(SPELL_GROUND_SLAM_1) ||
        bot->HasAura(SPELL_GROUND_SLAM_2))
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
            (dynamic_cast<MovementAction*>(action) &&
             !dynamic_cast<GruulTheDragonkillerShatterSpreadAction*>(action)))
             return 0.0f;
    }

    return 1.0f;
}
