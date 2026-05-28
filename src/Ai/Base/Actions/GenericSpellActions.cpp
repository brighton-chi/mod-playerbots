/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericSpellActions.h"

#include <ctime>
#include <unordered_map>
#include <unordered_set>

#include "Event.h"
#include "ItemTemplate.h"
#include "ObjectDefines.h"
#include "Opcodes.h"
#include "Player.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "WorldPacket.h"
#include "Group.h"
#include "Chat.h"
#include "GenericBuffUtils.h"
#include "PlayerbotAI.h"

using ai::buff::MakeAuraQualifierForBuff;
using ai::spell::HasSpellOrCategoryCooldown;

namespace
{
    std::unordered_set<uint32> const& GetMixedTriggerTrinketSpellIds()
    {
        static std::unordered_set<uint32> const mixedTriggerSpellIds = []()
        {
            std::unordered_set<uint32> onUseSpellIds;
            std::unordered_set<uint32> onEquipSpellIds;
            std::unordered_set<uint32> mixedSpellIds;

            auto const* itemTemplates = sObjectMgr->GetItemTemplateStore();
            if (!itemTemplates)
                return mixedSpellIds;

            auto const markSpellId = [&](int32 spellId, uint8 spellTrigger)
            {
                if (spellId <= 0)
                    return;

                if (spellTrigger == ITEM_SPELLTRIGGER_ON_USE)
                {
                    if (onEquipSpellIds.find(spellId) != onEquipSpellIds.end())
                        mixedSpellIds.insert(spellId);

                    onUseSpellIds.insert(spellId);
                }
                else if (spellTrigger == ITEM_SPELLTRIGGER_ON_EQUIP)
                {
                    if (onUseSpellIds.find(spellId) != onUseSpellIds.end())
                        mixedSpellIds.insert(spellId);

                    onEquipSpellIds.insert(spellId);
                }
            };

            for (auto const& itr : *itemTemplates)
            {
                ItemTemplate const& proto = itr.second;
                if (proto.InventoryType != INVTYPE_TRINKET)
                    continue;

                for (uint8 spellIndex = 0; spellIndex < MAX_ITEM_PROTO_SPELLS; ++spellIndex)
                {
                    auto const& spellData = proto.Spells[spellIndex];
                    markSpellId(spellData.SpellId, spellData.SpellTrigger);
                }
            }

            return mixedSpellIds;
        }();

        return mixedTriggerSpellIds;
    }

    bool IsManaRestoreEffect(SpellEffectInfo const& effectInfo)
    {
        return (effectInfo.Effect == SPELL_EFFECT_ENERGIZE &&
                effectInfo.MiscValue == POWER_MANA) ||
               (effectInfo.Effect == SPELL_EFFECT_APPLY_AURA &&
                effectInfo.ApplyAuraName == SPELL_AURA_PERIODIC_ENERGIZE &&
                effectInfo.MiscValue == POWER_MANA);
    }

    bool IsManaEfficiencyEffect(SpellEffectInfo const& effectInfo)
    {
        return effectInfo.Effect == SPELL_EFFECT_APPLY_AURA &&
               (((effectInfo.ApplyAuraName == SPELL_AURA_MOD_POWER_REGEN ||
                  effectInfo.ApplyAuraName == SPELL_AURA_MOD_POWER_REGEN_PERCENT) &&
                 effectInfo.MiscValue == POWER_MANA) ||
                effectInfo.ApplyAuraName == SPELL_AURA_MOD_POWER_COST_SCHOOL ||
                effectInfo.ApplyAuraName == SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT ||
                effectInfo.ApplyAuraName == SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
    }

    bool IsDefensiveTankEffect(SpellEffectInfo const& effectInfo)
    {
        if (effectInfo.Effect != SPELL_EFFECT_APPLY_AURA)
            return false;

        uint32 const tankRatingsMask =
            (1u << CR_DEFENSE_SKILL) |
            (1u << CR_DODGE) |
            (1u << CR_PARRY) |
            (1u << CR_BLOCK) |
            (1u << CR_HIT_TAKEN_MELEE) |
            (1u << CR_HIT_TAKEN_RANGED) |
            (1u << CR_HIT_TAKEN_SPELL) |
            (1u << CR_CRIT_TAKEN_MELEE) |
            (1u << CR_CRIT_TAKEN_RANGED) |
            (1u << CR_CRIT_TAKEN_SPELL);

        switch (effectInfo.ApplyAuraName)
        {
            case SPELL_AURA_MOD_RESISTANCE:
                return (effectInfo.MiscValue & SPELL_SCHOOL_MASK_NORMAL) != 0;
            case SPELL_AURA_MOD_RATING:
                return (effectInfo.MiscValue & tankRatingsMask) != 0;
            case SPELL_AURA_MOD_INCREASE_HEALTH:
            case SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT:
            case SPELL_AURA_MOD_PARRY_PERCENT:
            case SPELL_AURA_MOD_DODGE_PERCENT:
            case SPELL_AURA_MOD_BLOCK_PERCENT:
            case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:
                return true;
            default:
                return false;
        }
    }
}

CastSpellAction::CastSpellAction(PlayerbotAI* botAI, std::string const spell)
    : Action(botAI, spell), range(botAI->GetRange("spell")), spell(spell) {}

bool CastSpellAction::Execute(Event /*event*/)
{
    if (spell == "conjure food" || spell == "conjure water")
    {
        uint32 castId = 0;

        for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
        {
            uint32 spellId = itr->first;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            std::string const namepart = spellInfo->SpellName[0];
            std::wstring wnamepart;
            if (!Utf8toWStr(namepart, wnamepart))
                return false;

            wstrToLower(wnamepart);

            if (!Utf8FitTo(spell, wnamepart) || spellInfo->Effects[0].Effect != SPELL_EFFECT_CREATE_ITEM)
                continue;

            uint32 itemId = spellInfo->Effects[0].ItemType;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto || bot->CanUseItem(proto) != EQUIP_ERR_OK)
                continue;

            if (spellInfo->Id > castId)
                castId = spellInfo->Id;
        }

        return botAI->CastSpell(castId, bot);
    }

    return botAI->CastSpell(spell, GetTarget());
}

bool CastSpellAction::isUseful()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        bot->Dismount();
        return false;
    }

    Unit* spellTarget = GetTarget();
    if (!spellTarget || !spellTarget->IsInWorld() || spellTarget->GetMapId() != bot->GetMapId())
        return false;

    return AI_VALUE2(bool, "spell cast useful", spell);
}

bool CastSpellAction::isPossible()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
    {
        if (!sPlayerbotAIConfig.logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster()))
            LOG_DEBUG("playerbots", "Can cast spell failed. Vehicle. - bot name: {}", bot->GetName());

        return false;
    }

    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        if (!sPlayerbotAIConfig.logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster()))
            LOG_DEBUG("playerbots", "Can cast spell failed. Mount. - bot name: {}", bot->GetName());

        bot->Dismount();
        return false;
    }
    return botAI->CanCastSpell(spell, GetTarget());
}

CastMeleeSpellAction::CastMeleeSpellAction(
    PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell)
{
    range = ATTACK_DISTANCE;
}

bool CastMeleeSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !bot->IsWithinMeleeRange(target))
        return false;

    return CastSpellAction::isUseful();
}

CastMeleeDebuffSpellAction::CastMeleeDebuffSpellAction(
    PlayerbotAI* botAI, std::string const spell, bool isOwner, float needLifeTime) :
    CastDebuffSpellAction(botAI, spell, isOwner, needLifeTime)
{
    range = ATTACK_DISTANCE;
}

bool CastMeleeDebuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !bot->IsWithinMeleeRange(target))
        return false;

    return CastDebuffSpellAction::isUseful();
}

bool CastAuraSpellAction::isUseful()
{
    if (!GetTarget() || !CastSpellAction::isUseful())
        return false;

    Aura* aura = botAI->GetAura(spell, GetTarget(), isOwner, checkDuration);
    if (!aura || (beforeDuration && aura->GetDuration() < beforeDuration))
        return true;

    return false;
}

bool CastBuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !CastSpellAction::isUseful())
        return false;

    Aura* aura = botAI->GetAura(spell, target, isOwner, checkDuration);
    if (!aura || (beforeDuration && aura->GetDuration() < beforeDuration))
        return true;

    return false;
}

bool CastBuffSpellAction::Execute(Event /*event*/)
{
    return botAI->CastSpell(spell, GetTarget());
}

bool GroupBuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !CastSpellAction::isUseful())
        return false;

    if (ai::buff::IsGroupVariantEnabled(bot, spell))
    {
        std::string const groupVariant = ai::buff::GroupVariantFor(spell);
        if (!groupVariant.empty() && botAI->HasAura(groupVariant, target, false, isOwner, -1, checkDuration))
            return false;
    }

    Aura* aura = botAI->GetAura(spell, target, isOwner, checkDuration);
    if (!aura || (beforeDuration && aura->GetDuration() < beforeDuration))
        return true;

    return false;
}

bool GroupBuffSpellAction::Execute(Event /*event*/)
{
    std::string const castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, spell);
    return botAI->CastSpell(castName, GetTarget());
}

CastEnchantItemMainHandAction::CastEnchantItemMainHandAction(
    PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell) {}

bool CastEnchantItemMainHandAction::Execute(Event /*event*/)
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    return item && botAI->CastSpell(spell, bot, item);
}

bool CastEnchantItemMainHandAction::isPossible()
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!item || item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_MISC ||
        item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE ||
        item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
    {
        return false;
    }

    return botAI->CanCastSpell(spell, bot, item);
}

CastEnchantItemOffHandAction::CastEnchantItemOffHandAction(
    PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell) {}

bool CastEnchantItemOffHandAction::Execute(Event /*event*/)
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    return item && botAI->CastSpell(spell, bot, item);
}

bool CastEnchantItemOffHandAction::isPossible()
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!item || item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_MISC ||
        item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
    {
        return false;
    }

    return botAI->CanCastSpell(spell, bot, item);
}

CastHealingSpellAction::CastHealingSpellAction(PlayerbotAI* botAI, std::string const spell, uint8 estAmount,
                                               HealingManaEfficiency manaEfficiency, bool isOwner)
    : CastAuraSpellAction(botAI, spell, isOwner), estAmount(estAmount), manaEfficiency(manaEfficiency)
{
    range = botAI->GetRange("heal");
}

CastCureSpellAction::CastCureSpellAction(
    PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell)
{
    range = botAI->GetRange("heal");
}

Value<Unit*>* BuffOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", spell);
}

Value<Unit*>* GroupBuffOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", MakeAuraQualifierForBuff(spell));
}

CastShootAction::CastShootAction(
    PlayerbotAI* botAI) : CastSpellAction(botAI, "shoot"), shootSpellId(0)
{
    if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
    {
        spell = "shoot";

        switch (pItem->GetTemplate()->SubClass)
        {
            case ITEM_SUBCLASS_WEAPON_GUN:
                spell += " gun";
                shootSpellId = 3018;
                break;
            case ITEM_SUBCLASS_WEAPON_BOW:
                spell += " bow";
                shootSpellId = 3018;
                break;
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                spell += " crossbow";
                shootSpellId = 3018;
                break;
            case ITEM_SUBCLASS_WEAPON_THROWN:
                spell = "throw";
                shootSpellId = 2764;
                break;
        }
    }
}

bool CastShootAction::isPossible()
{
    if (shootSpellId)
        return botAI->CanCastSpell(shootSpellId, GetTarget(), false);

    return CastSpellAction::isPossible();
}

bool CastShootAction::Execute(Event /*event*/)
{
    if (shootSpellId)
        return botAI->CastSpell(shootSpellId, GetTarget());

    return botAI->CastSpell(spell, GetTarget());
}

CastBuffSpellAction::CastBuffSpellAction(
    PlayerbotAI* botAI, std::string const spell, bool checkIsOwner, uint32 beforeDuration)
    : CastAuraSpellAction(botAI, spell, checkIsOwner, false, beforeDuration)
{
    range = botAI->GetRange("spell");
}

bool CastVehicleSpellAction::isPossible()
{
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", spell);
    return botAI->CanCastVehicleSpell(spellId, GetTarget());
}

bool CastVehicleSpellAction::Execute(Event /*event*/)
{
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", spell);
    return botAI->CastVehicleSpell(spellId, GetTarget());
}

bool CastEveryManForHimselfAction::isPossible()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    return spellId && bot->HasSpell(spellId) && !HasSpellOrCategoryCooldown(bot, spellId);
}

bool CastEveryManForHimselfAction::isUseful()
{
    return (bot->HasAuraType(SPELL_AURA_MOD_STUN) ||
            bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
            bot->HasAuraType(SPELL_AURA_MOD_ROOT) ||
            bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) ||
            bot->HasAuraType(SPELL_AURA_MOD_CHARM)) &&
           CastSpellAction::isUseful();
}

bool CastWillOfTheForsakenAction::isPossible()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    return spellId && bot->HasSpell(spellId) && !HasSpellOrCategoryCooldown(bot, spellId);
}

bool CastWillOfTheForsakenAction::isUseful()
{
    return (bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
            bot->HasAuraType(SPELL_AURA_MOD_CHARM) ||
            bot->HasAuraType(SPELL_AURA_AOE_CHARM) ||
            bot->HasAuraWithMechanic(1 << MECHANIC_SLEEP)) &&
           CastSpellAction::isUseful();
}

bool UseTrinketAction::Execute(Event /*event*/)
{
    Item* trinket1 = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TRINKET1);

    if (trinket1 && UseTrinket(trinket1))
        return true;

    Item* trinket2 = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TRINKET2);
    if (trinket2 && UseTrinket(trinket2))
        return true;

    return false;
}

bool UseTrinketAction::UseTrinket(Item* item)
{
    if (bot->CanUseItem(item) != EQUIP_ERR_OK || bot->IsNonMeleeSpellCast(true))
        return false;

    struct SkullCooldownProbeState
    {
        bool initialized = false;
        bool hasSpellCooldown = false;
        bool hasCooldownEntry = false;
        uint32 cooldownItemId = 0;
        uint32 cooldownCategory = 0;
        bool lastCanCast = false;
        uint32 lastUseMs = 0;
    };

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 targetFlag = TARGET_FLAG_NONE;
    uint32 spellId = 0;
    uint8 healthPct = 0;
    uint8 manaPct = 0;
    bool logHealthPct = false;
    bool logManaPct = false;
    bool logPreviouslyExcludedProcUse = false;
    uint32 spellProcFlag = 0;
    int32 itemSpellCooldown = 0;
    uint32 itemSpellCategory = 0;
    int32 itemSpellCategoryCooldown = 0;
    static std::unordered_map<ObjectGuid::LowType, std::unordered_map<uint64, uint32>> trinketItemCooldownExpiries;
    static std::unordered_map<ObjectGuid::LowType, std::unordered_map<uint32, uint32>> trinketCategoryCooldownExpiries;
    static std::unordered_map<ObjectGuid::LowType, SkullCooldownProbeState> skullProbeStates;
    auto const logCooldownProbe = [&](char const* phase, bool includeCanCastResult = false, bool canCastResult = false)
    {
        if (item->GetEntry() != 32483 && spellId != 40396)
            return;

        SkullCooldownProbeState& state = skullProbeStates[bot->GetGUID().GetCounter()];

        auto const& cooldownMap = bot->GetSpellCooldownMap();
        auto const cooldownItr = cooldownMap.find(spellId);
        bool const hasSpellCooldown = bot->HasSpellCooldown(spellId);
        bool const hasCooldownEntry = cooldownItr != cooldownMap.end();
        uint32 const cooldownItemId = hasCooldownEntry ? cooldownItr->second.itemid : 0;
        uint32 const cooldownCategory = hasCooldownEntry ? cooldownItr->second.category : 0;
        uint32 const cooldownDelay = bot->GetSpellCooldownDelay(spellId);
        uint32 const now = getMSTime();
        bool const cooldownClearedEarly = state.initialized && state.hasSpellCooldown && !hasSpellCooldown && state.lastUseMs &&
            getMSTimeDiff(state.lastUseMs, now) + 1000u < static_cast<uint32>(itemSpellCooldown);
        bool const stateChanged = !state.initialized ||
            state.hasSpellCooldown != hasSpellCooldown ||
            state.hasCooldownEntry != hasCooldownEntry ||
            state.cooldownItemId != cooldownItemId ||
            state.cooldownCategory != cooldownCategory ||
            (includeCanCastResult && state.lastCanCast != canCastResult);

        if (!stateChanged && !cooldownClearedEarly)
            return;

        if (includeCanCastResult)
        {
            LOG_INFO("playerbots",
                     "Skull cooldown probe [{}]: bot {}, item {}, spell {}, canCast {}, hasSpellCooldown {}, cooldownDelay {}, cooldownEntry {}, cooldownItemId {}, cooldownCategory {}, itemCooldown {}, itemCategory {}, itemCategoryCooldown {}, cooldownClearedEarly {}",
                     phase, bot->GetName().c_str(), item->GetEntry(), spellId, canCastResult, hasSpellCooldown,
                     cooldownDelay, hasCooldownEntry, cooldownItemId, cooldownCategory, itemSpellCooldown,
                     itemSpellCategory, itemSpellCategoryCooldown, cooldownClearedEarly);
        }

        if (!includeCanCastResult)
        {
            LOG_INFO("playerbots",
                     "Skull cooldown probe [{}]: bot {}, item {}, spell {}, hasSpellCooldown {}, cooldownDelay {}, cooldownEntry {}, cooldownItemId {}, cooldownCategory {}, itemCooldown {}, itemCategory {}, itemCategoryCooldown {}, cooldownClearedEarly {}",
                     phase, bot->GetName().c_str(), item->GetEntry(), spellId, hasSpellCooldown, cooldownDelay,
                     hasCooldownEntry, cooldownItemId, cooldownCategory, itemSpellCooldown, itemSpellCategory,
                     itemSpellCategoryCooldown, cooldownClearedEarly);
        }

        state.initialized = true;
        state.hasSpellCooldown = hasSpellCooldown;
        state.hasCooldownEntry = hasCooldownEntry;
        state.cooldownItemId = cooldownItemId;
        state.cooldownCategory = cooldownCategory;
        if (includeCanCastResult)
            state.lastCanCast = canCastResult;
    };
    auto const logSkullUseProbe = [&]()
    {
        if (item->GetEntry() != 32483 && spellId != 40396)
            return;

        SkullCooldownProbeState& state = skullProbeStates[bot->GetGUID().GetCounter()];
        uint32 const now = getMSTime();
        uint32 const elapsedSinceLastUse = state.lastUseMs ? getMSTimeDiff(state.lastUseMs, now) : 0;
        bool const reusedEarly = state.lastUseMs && elapsedSinceLastUse + 1000u < static_cast<uint32>(itemSpellCooldown);

        LOG_INFO("playerbots",
                 "Skull use probe: bot {}, item {}, spell {}, elapsedSinceLastUse {}, itemCooldown {}, itemCategory {}, itemCategoryCooldown {}, reusedEarly {}",
                 bot->GetName().c_str(), item->GetEntry(), spellId, elapsedSinceLastUse, itemSpellCooldown,
                 itemSpellCategory, itemSpellCategoryCooldown, reusedEarly);

        state.initialized = true;
        state.lastUseMs = now;
    };
    auto const logOracleAuraProbe = [&](char const* phase)
    {
        Aura* const oracleAura = bot->GetAura(59787);
        if (item->GetEntry() != 44870 && spellId != 59787 && !oracleAura)
            return;

        LOG_INFO("playerbots",
                 "Oracle aura probe [{}]: bot {}, item {}, spell {}, auraPresent {}, auraStacks {}, auraDuration {}",
                 phase, bot->GetName().c_str(), item->GetEntry(), spellId, oracleAura != nullptr,
                 oracleAura ? oracleAura->GetStackAmount() : 0, oracleAura ? oracleAura->GetDuration() : 0);
    };

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0 &&
            item->GetTemplate()->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            spellProcFlag = spellInfo ? spellInfo->ProcFlags : 0;
            itemSpellCooldown = item->GetTemplate()->Spells[i].SpellCooldown;
            itemSpellCategory = item->GetTemplate()->Spells[i].SpellCategory;
            itemSpellCategoryCooldown = item->GetTemplate()->Spells[i].SpellCategoryCooldown;

            if (!spellInfo || !spellInfo->IsPositive())
                return false;

            bool applyAura = false;
            bool restoresMana = false;
            bool improvesManaEfficiency = false;
            bool defensiveTankEffect = false;
            for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
            {
                const SpellEffectInfo& effectInfo = spellInfo->Effects[i];
                if (effectInfo.Effect == SPELL_EFFECT_APPLY_AURA)
                    applyAura = true;

                restoresMana = restoresMana || IsManaRestoreEffect(effectInfo);
                improvesManaEfficiency = improvesManaEfficiency || IsManaEfficiencyEffect(effectInfo);
                defensiveTankEffect = defensiveTankEffect || IsDefensiveTankEffect(effectInfo);
            }

            if (!applyAura && !restoresMana)
                return false;

            if (restoresMana || improvesManaEfficiency)
            {
                if (!AI_VALUE2(bool, "has mana", "self target"))
                    return false;

                manaPct = AI_VALUE2(uint8, "mana", "self target");
                logManaPct = true;
                if ((restoresMana && manaPct >= sPlayerbotAIConfig.mediumMana) ||
                    manaPct >= sPlayerbotAIConfig.highMana)
                {
                    return false;
                }
            }

            if (defensiveTankEffect)
            {
                healthPct = AI_VALUE2(uint8, "health", "self target");
                logHealthPct = true;
                if (healthPct > sPlayerbotAIConfig.lowHealth)
                    return false;
            }

            auto const& mixedTriggerTrinketSpellIds = GetMixedTriggerTrinketSpellIds();

            // Exclude malformed trinket rows that expose the same spell as both ON_EQUIP and
            // ON_USE across trinket item templates. Those are equip/proc effects leaking into
            // the active-use path, as seen with Oracle Talisman of Ablution and Frenzyheart.
            if (mixedTriggerTrinketSpellIds.find(spellId) != mixedTriggerTrinketSpellIds.end())
                return false;

            ObjectGuid::LowType const botGuid = bot->GetGUID().GetCounter();
            auto& itemCooldownExpiries = trinketItemCooldownExpiries[botGuid];
            auto& categoryCooldownExpiries = trinketCategoryCooldownExpiries[botGuid];
            uint64 const itemCooldownKey = (static_cast<uint64>(item->GetEntry()) << 32) | spellId;
            uint32 const now = getMSTime();

            if (itemSpellCooldown > 0)
            {
                auto const itemCooldownItr = itemCooldownExpiries.find(itemCooldownKey);
                if (itemCooldownItr != itemCooldownExpiries.end())
                {
                    if (itemCooldownItr->second > now)
                        return false;

                    itemCooldownExpiries.erase(itemCooldownItr);
                }
            }

            if (itemSpellCategory && itemSpellCategoryCooldown > 0)
            {
                auto const categoryCooldownItr = categoryCooldownExpiries.find(itemSpellCategory);
                if (categoryCooldownItr != categoryCooldownExpiries.end())
                {
                    if (categoryCooldownItr->second > now)
                        return false;

                    categoryCooldownExpiries.erase(categoryCooldownItr);
                }
            }

            logCooldownProbe("before CanCastSpell");
            logOracleAuraProbe("before CanCastSpell");

            bool const canCast = botAI->CanCastSpell(spellId, bot, false, nullptr, item);
            logCooldownProbe("after CanCastSpell", true, canCast);
            logOracleAuraProbe("after CanCastSpell");
            if (!canCast)
                return false;

            break;
        }
    }

    if (!spellId)
        return false;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << cast_count << spellId << item_guid << glyphIndex << castFlags;

    targetFlag = TARGET_FLAG_NONE;
    packet << targetFlag << bot->GetPackGUID();

    if (logHealthPct && logManaPct)
    {
        LOG_INFO("playerbots", "Bot {} used gated trinket {} (health: {}%, mana: {}%)",
                 bot->GetName().c_str(), item->GetTemplate()->Name1.c_str(), healthPct, manaPct);
    }
    else if (logHealthPct)
    {
        LOG_INFO("playerbots", "Bot {} used gated trinket {} (health: {}%)",
                 bot->GetName().c_str(), item->GetTemplate()->Name1.c_str(), healthPct);
    }
    else if (logManaPct)
    {
        LOG_INFO("playerbots", "Bot {} used gated trinket {} (mana: {}%)",
                 bot->GetName().c_str(), item->GetTemplate()->Name1.c_str(), manaPct);
    }

    if (logPreviouslyExcludedProcUse)
    {
        LOG_INFO("playerbots",
                 "Bot {} used trinket {} (item {}, spell {}, procFlags {}, itemCooldown {}, itemCategory {}, itemCategoryCooldown {}) after narrowed proc gate",
                 bot->GetName().c_str(), item->GetTemplate()->Name1.c_str(), item->GetEntry(), spellId, spellProcFlag,
                 itemSpellCooldown, itemSpellCategory, itemSpellCategoryCooldown);
    }

    logCooldownProbe("before HandleUseItemOpcode");
    logOracleAuraProbe("before HandleUseItemOpcode");
    bot->GetSession()->HandleUseItemOpcode(packet);
    logCooldownProbe("after HandleUseItemOpcode");
    logSkullUseProbe();
    logOracleAuraProbe("after HandleUseItemOpcode");

    ObjectGuid::LowType const botGuid = bot->GetGUID().GetCounter();
    uint32 const now = getMSTime();
    uint32 const cooldownDelay = bot->GetSpellCooldownDelay(spellId);
    if (cooldownDelay > 0)
    {
        if (itemSpellCooldown > 0)
        {
            uint64 const itemCooldownKey = (static_cast<uint64>(item->GetEntry()) << 32) | spellId;
            trinketItemCooldownExpiries[botGuid][itemCooldownKey] = now + static_cast<uint32>(itemSpellCooldown);
        }

        if (itemSpellCategory && itemSpellCategoryCooldown > 0)
            trinketCategoryCooldownExpiries[botGuid][itemSpellCategory] = now + static_cast<uint32>(itemSpellCategoryCooldown);
    }

    return true;
}
bool CastDebuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    return CastAuraSpellAction::isUseful() &&
           (target->GetHealth() / AI_VALUE(float, "estimated group dps")) >= needLifeTime;
}
