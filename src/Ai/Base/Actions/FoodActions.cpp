/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoodActions.h"
#include "Event.h"
#include "Playerbots.h"

namespace
{
constexpr uint32 BG_WS_SPELL_WARSONG_FLAG = 23333;
constexpr uint32 BG_WS_SPELL_SILVERWING_FLAG = 23335;
constexpr uint32 BG_EY_NETHERSTORM_FLAG_SPELL = 34976;
constexpr uint32 SPELL_GRACCUS_MINCE_MEAT_FRUITCAKE = 25990;
// Graccu's Mince Meat Fruitcake lasts 20s, which therefore is the longest the bot should sit.
constexpr uint32 RECOVERY_MAX_MS = 20 * IN_MILLISECONDS;

uint32 RecoveryDurationMs(uint32 current, uint32 max)
{
    return static_cast<uint32>(RECOVERY_MAX_MS * (static_cast<float>(max - current) / max));
}

bool IsDisallowedShapeshiftForm(Player* bot)
{
    if (bot->getClass() == CLASS_DRUID)
    {
        ShapeshiftForm form = bot->GetShapeshiftForm();
        return form == FORM_TRAVEL || form == FORM_AQUA || form == FORM_CAT || form == FORM_BEAR ||
               form == FORM_DIREBEAR || form == FORM_FLIGHT || form == FORM_FLIGHT_EPIC;
    }

    return bot->getClass() == CLASS_PRIEST && bot->GetShapeshiftForm() == FORM_SPIRITOFREDEMPTION;
}
}

bool DrinkAction::Execute(Event event)
{
    if (botAI->HasCheat(BotCheatMask::food))
    {
        bot->ClearUnitState(UNIT_STATE_CHASE);
        bot->ClearUnitState(UNIT_STATE_FOLLOW);

        if (bot->isMoving())
            bot->StopMoving();

        bot->SetStandState(UNIT_STAND_STATE_SIT);
        bot->CastStop();

        SetDuration(RecoveryDurationMs(bot->GetPower(POWER_MANA), bot->GetMaxPower(POWER_MANA)));

        bot->AddAura(SPELL_GRACCUS_MINCE_MEAT_FRUITCAKE, bot);
        return true;
    }

    return UseItemAction::Execute(event);
}

bool DrinkAction::isUseful()
{
    return bot->GetPower(POWER_MANA) < bot->GetMaxPower(POWER_MANA);
}

bool DrinkAction::isPossible()
{
    if (bot->IsInCombat() || bot->IsMounted() || IsDisallowedShapeshiftForm(bot))
        return false;

    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible();
}

bool EatAction::Execute(Event event)
{
    if (botAI->HasCheat(BotCheatMask::food))
    {
        bot->ClearUnitState(UNIT_STATE_CHASE);
        bot->ClearUnitState(UNIT_STATE_FOLLOW);

        if (bot->isMoving())
            bot->StopMoving();

        bot->SetStandState(UNIT_STAND_STATE_SIT);
        bot->CastStop();

        SetDuration(RecoveryDurationMs(bot->GetHealth(), bot->GetMaxHealth()));

        bot->AddAura(SPELL_GRACCUS_MINCE_MEAT_FRUITCAKE, bot);
        return true;
    }

    return UseItemAction::Execute(event);
}

bool EatAction::isUseful()
{
    return !bot->IsFullHealth();
}

bool EatAction::isPossible()
{
    if (bot->IsInCombat() || bot->IsMounted() || IsDisallowedShapeshiftForm(bot))
        return false;

    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible();
}
