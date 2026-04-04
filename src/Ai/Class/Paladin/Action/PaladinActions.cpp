/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinActions.h"

#include "AiFactory.h"
#include "Event.h"
#include "GenericBuffUtils.h"
#include "ObjectAccessor.h"
#include "PaladinBlessingPriorityData.h"
#include "Playerbots.h"
#include "SharedDefines.h"

using ai::buff::MakeAuraQualifierForBuff;
using ai::gbless::SPELL_BLESSING_OF_SANCTUARY;
using ai::gbless::SPELL_GREATER_BLESSING_OF_SANCTUARY;

static inline bool IsTankRole(Player* player)
{
    if (!player)
        return false;

    if (player->HasTankSpec())
        return true;

    if (PlayerbotAI* otherAI = GET_PLAYERBOT_AI(player))
    {
        if (otherAI->HasStrategy("tank", BOT_STATE_NON_COMBAT) ||
            otherAI->HasStrategy("tank", BOT_STATE_COMBAT)     ||
            otherAI->HasStrategy("tank face", BOT_STATE_NON_COMBAT) ||
            otherAI->HasStrategy("tank face", BOT_STATE_COMBAT)     ||
            otherAI->HasStrategy("bear", BOT_STATE_NON_COMBAT) ||
            otherAI->HasStrategy("bear", BOT_STATE_COMBAT))
            return true;
    }

    return false;
}

static inline bool IsOnlyPaladinInGroup(Player* bot)
{
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return true;

    uint32 paladins = 0u;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* player = ref->GetSource();
        if (!player || !player->IsInWorld()) continue;
        if (player->getClass() == CLASS_PALADIN) ++paladins;
    }

    return paladins == 1u;
}

inline std::string const GetActualBlessingOfMight(Unit* target)
{
    if (!target->ToPlayer())
        return "blessing of might";

    uint8 tab = AiFactory::GetPlayerSpecTab(target->ToPlayer());
    switch (target->getClass())
    {
        case CLASS_MAGE:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
            return "blessing of wisdom";
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ELEMENTAL || tab == SHAMAN_TAB_RESTORATION)
                return "blessing of wisdom";
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_RESTORATION || tab == DRUID_TAB_BALANCE)
                return "blessing of wisdom";
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_HOLY)
                return "blessing of wisdom";
            break;
    }

    return "blessing of might";
}

inline std::string const GetActualBlessingOfWisdom(Unit* target)
{
    if (!target->ToPlayer())
        return "blessing of might";

    uint8 tab = AiFactory::GetPlayerSpecTab(target->ToPlayer());
    switch (target->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
        case CLASS_HUNTER:
            return "blessing of might";
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ENHANCEMENT)
                return "blessing of might";
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_FERAL)
                return "blessing of might";
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_PROTECTION || tab == PALADIN_TAB_RETRIBUTION)
                return "blessing of might";
            break;
    }

    return "blessing of wisdom";
}

inline std::string const GetActualBlessingOfSanctuary(Unit* target, Player* bot)
{
    if (!bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY))
        return "";

    Player* targetPlayer = target->ToPlayer();
    if (!targetPlayer)
        return "";

    if (auto* botAI = GET_PLAYERBOT_AI(bot))
    {
        if (Unit* mainTank = botAI->GetAiObjectContext()->GetValue<Unit*>("main tank")->Get())
        {
            if (mainTank == target)
                return "blessing of sanctuary";
        }
    }

    if (targetPlayer->HasTankSpec())
        return "blessing of sanctuary";

    return "";
}

Value<Unit*>* CastBlessingOnPartyAction::GetTargetValue()
{

    return context->GetValue<Unit*>("party member without aura", MakeAuraQualifierForBuff(spell));
}

bool CastBlessingOfMightAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = GetActualBlessingOfMight(target);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfMightOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>(
    "party member without aura",
    "blessing of might,greater blessing of might,blessing of wisdom,greater blessing of wisdom,"
    "blessing of sanctuary,greater blessing of sanctuary"
    );
}

bool CastBlessingOfMightOnPartyAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = GetActualBlessingOfMight(target);
    return botAI->CastSpell(castName, target);
}

bool CastBlessingOfWisdomAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = GetActualBlessingOfWisdom(target);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfWisdomOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>(
    "party member without aura",
    "blessing of wisdom,greater blessing of wisdom,blessing of might,greater blessing of might,"
    "blessing of sanctuary,greater blessing of sanctuary"
    );
}

bool CastBlessingOfWisdomOnPartyAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    Player* targetPlayer = target->ToPlayer();

    if (Group* group = bot->GetGroup())
        if (targetPlayer && !group->IsMember(targetPlayer->GetGUID()))
            return false;

    if (botAI->HasStrategy("bmana", BOT_STATE_NON_COMBAT) &&
        targetPlayer && IsTankRole(targetPlayer))
        return false;

    std::string castName = GetActualBlessingOfWisdom(target);
    if (castName.empty())
        return false;

    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfSanctuaryOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>(
        "party member without aura",
        "blessing of sanctuary,greater blessing of sanctuary"
    );
}

bool CastBlessingOfSanctuaryOnPartyAction::Execute(Event /*event*/)
{
    if (!bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY))
        return false;

    Unit* target = GetTarget();
    if (!target)
        target = bot;

    Player* targetPlayer = target ? target->ToPlayer() : nullptr;

    const auto HasKingsAura = [&](Unit* unit) -> bool {
        return botAI->HasAura("blessing of kings", unit) || botAI->HasAura("greater blessing of kings", unit);
    };
    const auto HasSanctAura = [&](Unit* unit) -> bool {
        return botAI->HasAura("blessing of sanctuary", unit) || botAI->HasAura("greater blessing of sanctuary", unit);
    };

    if (Group* group = bot->GetGroup())
    {
        if (targetPlayer && !group->IsMember(targetPlayer->GetGUID()))
        {
            target = bot;
            targetPlayer = bot->ToPlayer();
        }
    }

    if (Player* self = bot->ToPlayer())
    {
        bool selfHasSanct = HasSanctAura(self);
        bool needSelf = IsTankRole(self) && !selfHasSanct;

        if (needSelf)
        {
            target = self;
            targetPlayer = self;
        }
    }

    bool targetOk = false;
    if (targetPlayer)
    {
        bool hasSanct = HasSanctAura(targetPlayer);
        targetOk = IsTankRole(targetPlayer) && !hasSanct;
    }

    if (!targetOk)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* player = ref->GetSource();
                if (!player) continue;
                if (!player->IsInWorld() || !player->IsAlive()) continue;
                if (!IsTankRole(player)) continue;

                bool hasSanct = HasSanctAura(player);
                if (!hasSanct)
                {
                    target = player;
                    targetPlayer = player;
                    targetOk = true;
                    break;
                }
            }
        }
    }

    {
        bool hasKings = HasKingsAura(target);
        bool hasSanct = HasSanctAura(target);
        bool knowSanct = bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY);
    }

    std::string castName = GetActualBlessingOfSanctuary(target, bot);
    if (castName.empty())
    {
        if (targetPlayer)
        {
            if (IsTankRole(targetPlayer))
                castName = "blessing of sanctuary";
            else
                return false;
        }
        else
            return false;
    }
    castName = "blessing of sanctuary";

    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfKingsOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>(
        "party member without aura",
        "blessing of kings,greater blessing of kings,blessing of sanctuary,greater blessing of sanctuary"
    );
}

bool CastBlessingOfKingsOnPartyAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (botAI->HasStrategy("bstats", BOT_STATE_NON_COMBAT) && IsOnlyPaladinInGroup(bot))
    {
        if (target->GetGUID() == bot->GetGUID())
            return false;
    }

    Player* targetPlayer = target->ToPlayer();
    if (targetPlayer && !group->IsMember(targetPlayer->GetGUID()))
        return false;

    const bool hasBmana  = botAI->HasStrategy("bmana",  BOT_STATE_NON_COMBAT);
    const bool hasBstats = botAI->HasStrategy("bstats", BOT_STATE_NON_COMBAT);

    if (hasBmana && (!targetPlayer || !IsTankRole(targetPlayer)))
        return false;

    if (targetPlayer)
    {
        const bool isTank = IsTankRole(targetPlayer);
        const bool hasSanctFromMe =
            target->HasAura(SPELL_BLESSING_OF_SANCTUARY, bot->GetGUID()) ||
            target->HasAura(SPELL_GREATER_BLESSING_OF_SANCTUARY, bot->GetGUID());
        const bool hasSanctAny =
            botAI->HasAura("blessing of sanctuary", target) ||
            botAI->HasAura("greater blessing of sanctuary", target);

        if (isTank && hasSanctFromMe)
            return false;

        if (hasBstats && isTank && hasSanctAny)
            return false;
    }

    return botAI->CastSpell("blessing of kings", target);
}

bool CastSealSpellAction::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

Value<Unit*>* CastTurnUndeadAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

Unit* CastHandOfFreedomOnPartyAction::GetTarget()
{
    bool const selfImpaired = botAI->IsMovementImpaired(bot);
    bool const hasSelfHand = selfImpaired && ai::paladin::HasAnyPaladinHandFromCaster(bot, bot);

    if (!bot->GetGroup())
    {
        if (selfImpaired && !hasSelfHand)
            return bot;

        return nullptr;
    }

    if (selfImpaired && !hasSelfHand)
        return bot;

    return CastBuffSpellAction::GetTarget();
}

Value<Unit*>* CastHandOfFreedomOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member snared target");
}

bool CastHandOfFreedomOnPartyAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return CastBuffSpellAction::isUseful() && !ai::paladin::HasAnyPaladinHandFromCaster(target, bot);
}

Unit* CastRighteousDefenseAction::GetTarget()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
        return nullptr;

    return current_target->GetVictim();
}

bool CastDivineSacrificeAction::isUseful()
{
    return GetTarget() && (GetTarget() != nullptr) && CastSpellAction::isUseful() &&
           !botAI->HasAura("divine guardian", GetTarget(), false, false, -1, true);
}

bool CastCancelDivineSacrificeAction::Execute(Event /*event*/)
{
    botAI->RemoveAura("divine sacrifice");
    return true;
}

bool CastCancelDivineSacrificeAction::isUseful()
{
    return botAI->HasAura("divine sacrifice", GetTarget(), false, true, -1, true);
}
