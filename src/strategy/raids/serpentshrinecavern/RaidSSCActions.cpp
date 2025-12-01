#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "AiFactory.h"
#include "Corpse.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

using namespace SerpentShrineCavernHelpers;
using namespace SerpentShrineCavernPositions;

// Trash Mobs

// Non-combat method; some colossi leave a toxic pool upon death
// Without this method, bots just stand (or drink) in the pool and die
bool UnderbogColossusEscapeToxicPoolAction::Execute(Event event)
{
    Aura* aura = bot->GetAura(SPELL_TOXIC_POOL);
    if (!aura)
        return false;

    DynamicObject* dynObj = aura->GetDynobjOwner();
    if (!dynObj)
        return false;

    float radius = dynObj->GetRadius();
    if (radius <= 0.0f)
    {
        const SpellInfo* sInfo = sSpellMgr->GetSpellInfo(dynObj->GetSpellId());
        if (sInfo)
        {
            for (int e = 0; e < MAX_SPELL_EFFECTS; ++e)
            {
                auto const& eff = sInfo->Effects[e];
                if (eff.Effect == SPELL_EFFECT_SCHOOL_DAMAGE ||
                    (eff.Effect == SPELL_EFFECT_APPLY_AURA && eff.ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE))
                {
                    radius = eff.CalcRadius();
                    break;
                }
            }
        }
    }

    if (radius <= 0.0f)
        return false;

    const float buffer = 3.0f;
    const float centerThreshold = 1.0f;
    float dx = bot->GetPositionX() - dynObj->GetPositionX();
    float dy = bot->GetPositionY() - dynObj->GetPositionY();
    float distSq = dx * dx + dy * dy;
    const float insideThresh = radius + centerThreshold;
    const float insideThreshSq = insideThresh * insideThresh;

    if (distSq > insideThreshSq)
        return false;

    float safeDist = radius + buffer;
    float moveX, moveY;

    if (distSq == 0.0f)
    {
        float angle = frand(0.0f, static_cast<float>(M_PI * 2.0));
        moveX = dynObj->GetPositionX() + std::cos(angle) * safeDist;
        moveY = dynObj->GetPositionY() + std::sin(angle) * safeDist;
    }
    else
    {
        float dist = sqrt(dx * dx + dy * dy);
        float inv = 1.0f / dist;
        moveX = dynObj->GetPositionX() + (dx * inv) * safeDist;
        moveY = dynObj->GetPositionY() + (dy * inv) * safeDist;
    }

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool GreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event event)
{
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_WATER_ELEMENTAL_TOTEM);
    if (!totem)
        return false;

    MarkTargetWithSkull(bot, totem);
    return false;
}

// Hydross the Unstable <Duke of Currents>

// (1) When tanking, move to designated tanking spot on frost side
// (2) 5 seconds into 100% Mark of Hydross, move to nature tank's spot to hand off boss
// (3) When Hydross is in nature form, move back to frost tank spot and wait for transition
bool HydrossTheUnstablePositionFrostTankAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (!hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfHydrossAt100Percent(bot))
    {
        MarkTargetWithSquare(bot, hydross);
        SetRtiTarget(botAI, "square", hydross);

        if (bot->GetVictim() != hydross)
            return Attack(hydross);

        if (hydross->GetVictim() == bot)
        {
            const Position& position = HydrossFrostTankPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
    }

    if (!hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfHydrossAt100Percent(bot) && hydross->GetVictim() == bot)
    {
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToNaturePhaseTimer.find(SSC_MAP_ID);

        if (it != hydrossChangeToNaturePhaseTimer.end() && (now - it->second) >= 5)
        {
            const Position& position = HydrossNatureTankPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                            MovementPriority::MOVEMENT_COMBAT, true, true);
            }
            else
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return true;
            }
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION))
    {
        const Position& position = HydrossFrostTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return true;
        }
    }

    return false;
}

// (1) When tanking, move to designated tanking spot on nature side
// (2) 5 seconds into 100% Mark of Corruption, move to frost tank's spot to hand off boss
// (3) When Hydross is in frost form, move back to nature tank spot and wait for transition
bool HydrossTheUnstablePositionNatureTankAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfCorruptionAt100Percent(bot))
    {
        MarkTargetWithTriangle(bot, hydross);
        SetRtiTarget(botAI, "triangle", hydross);

        if (bot->GetVictim() != hydross)
            return Attack(hydross);

        if (hydross->GetVictim() == bot)
        {
            const Position& position = HydrossNatureTankPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfCorruptionAt100Percent(bot) && hydross->GetVictim() == bot)
    {
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToFrostPhaseTimer.find(SSC_MAP_ID);

        if (it != hydrossChangeToFrostPhaseTimer.end() && (now - it->second) >= 5)
        {
            const Position& position = HydrossFrostTankPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT, true, true);
            }
            else
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return true;
            }
        }
    }

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        const Position& position = HydrossNatureTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return true;
        }
    }

    return false;
}

bool HydrossTheUnstablePrioritizeElementalAddsAction::Execute(Event event)
{
    Unit* waterElemental = GetFirstAliveUnitByEntry(botAI, NPC_PURE_SPAWN_OF_HYDROSS);
    if (waterElemental)
    {
        if (IsMapIDTimerManager(botAI, bot))
            MarkTargetWithSkull(bot, waterElemental);

        SetRtiTarget(botAI, "skull", waterElemental);

        if (bot->GetTarget() != waterElemental->GetGUID())
        {
            bot->SetTarget(waterElemental->GetGUID());
            return Attack(waterElemental);
        }
    }
    else if (Unit* natureElemental = GetFirstAliveUnitByEntry(botAI, NPC_TAINTED_SPAWN_OF_HYDROSS))
    {
        if (IsMapIDTimerManager(botAI, bot))
            MarkTargetWithSkull(bot, natureElemental);

        SetRtiTarget(botAI, "skull", natureElemental);

        if (bot->GetTarget() != natureElemental->GetGUID())
        {
            bot->SetTarget(natureElemental->GetGUID());
            return Attack(natureElemental);
        }
    }

    return false;
}

// To mitigate the effect of Water Tomb
bool HydrossTheUnstableFrostPhaseSpreadOutAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    Group* group = bot->GetGroup();
    if (!hydross || !group)
        return false;

    const uint32 minInterval = 500;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (bot->GetExactDist2d(member) < 6.0f)
            return FleePosition(member->GetPosition(), 8.0f, minInterval);
    }

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    Group* group = bot->GetGroup();
    if (!hydross || !group)
        return false;

    if (TryMisdirectToFrostTank(hydross, group))
        return true;

    if (TryMisdirectToNatureTank(hydross, group))
        return true;

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::TryMisdirectToFrostTank(Unit* hydross, Group* group)
{
    Player* frostTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            frostTank = member;
            break;
        }
    }

    if (HasNoMarkOfHydross(bot) && !hydross->HasAura(SPELL_CORRUPTION) && frostTank)
    {
        if (botAI->CanCastSpell("misdirection", frostTank))
            return botAI->CastSpell("misdirection", frostTank);

        if (!bot->HasAura(SPELL_MISDIRECTION))
            return false;

        if (botAI->CanCastSpell("steady shot", hydross))
            return botAI->CastSpell("steady shot", hydross);
    }

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::TryMisdirectToNatureTank(Unit* hydross, Group* group)
{
    Player* natureTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 0))
        {
            natureTank = member;
            break;
        }
    }

    if (HasNoMarkOfCorruption(bot) && hydross->HasAura(SPELL_CORRUPTION) && natureTank)
    {
        if (botAI->CanCastSpell("misdirection", natureTank))
            return botAI->CastSpell("misdirection", natureTank);

        if (!bot->HasAura(SPELL_MISDIRECTION))
            return false;

        if (botAI->CanCastSpell("steady shot", hydross))
            return botAI->CastSpell("steady shot", hydross);
    }

    return false;
}

bool HydrossTheUnstableStopDpsUponPhaseChangeAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    const time_t now = std::time(nullptr);
    const int phaseEndStopSeconds = 6;
    const int phaseStartStopSeconds = 5;

    bool shouldStopDps = false;

    // 6 seconds after 100% Mark of Corruption, stop DPS until transition into frost phase
    auto itNature = hydrossChangeToNaturePhaseTimer.find(SSC_MAP_ID);
    if (itNature != hydrossChangeToNaturePhaseTimer.end() && (now - itNature->second) >= phaseEndStopSeconds)
        shouldStopDps = true;

    // Keep DPS stopped for 5 seconds after transition into frost phase
    auto itFrostDps = hydrossFrostDpsWaitTimer.find(SSC_MAP_ID);
    if (itFrostDps != hydrossFrostDpsWaitTimer.end() && (now - itFrostDps->second) < phaseStartStopSeconds)
        shouldStopDps = true;

    // 6 seconds after 100% Mark of Hydross, stop DPS until transition into nature phase
    auto itFrost = hydrossChangeToFrostPhaseTimer.find(SSC_MAP_ID);
    if (itFrost != hydrossChangeToFrostPhaseTimer.end() && (now - itFrost->second) >= phaseEndStopSeconds)
        shouldStopDps = true;

    // Keep DPS stopped for 5 seconds after transition into nature phase
    auto itNatureDps = hydrossNatureDpsWaitTimer.find(SSC_MAP_ID);
    if (itNatureDps != hydrossNatureDpsWaitTimer.end() && (now - itNatureDps->second) < phaseStartStopSeconds)
        shouldStopDps = true;

    if (shouldStopDps)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return true;
    }

    return false;
}

bool HydrossTheUnstableManageTimersAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    const time_t now = std::time(nullptr);

    if (hydross->GetHealth() == hydross->GetMaxHealth())
    {
        hydrossFrostDpsWaitTimer.erase(SSC_MAP_ID);
        hydrossNatureDpsWaitTimer.erase(SSC_MAP_ID);
        hydrossChangeToFrostPhaseTimer.erase(SSC_MAP_ID);
        hydrossChangeToNaturePhaseTimer.erase(SSC_MAP_ID);
    }

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        hydrossFrostDpsWaitTimer.try_emplace(SSC_MAP_ID, now);
        hydrossNatureDpsWaitTimer.erase(SSC_MAP_ID);
        hydrossChangeToFrostPhaseTimer.erase(SSC_MAP_ID);

        if (HasMarkOfHydrossAt100Percent(bot))
            hydrossChangeToNaturePhaseTimer.try_emplace(SSC_MAP_ID, now);
    }
    else
    {
        hydrossNatureDpsWaitTimer.try_emplace(SSC_MAP_ID, now);
        hydrossFrostDpsWaitTimer.erase(SSC_MAP_ID);
        hydrossChangeToNaturePhaseTimer.erase(SSC_MAP_ID);

        if (HasMarkOfCorruptionAt100Percent(bot))
            hydrossChangeToFrostPhaseTimer.try_emplace(SSC_MAP_ID, now);
    }

    return false;
}

// The Lurker Below

// Run around behind Lurker during Spout, maintaining distance of 20-24 yards
// Stay within 90-degree arc behind Lurker
bool TheLurkerBelowRunAroundBehindBossAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (bot->HasAura(SPELL_TREE_OF_LIFE) && botAI->CanCastSpell("tree of life", bot))
        return botAI->CastSpell("tree of life", bot);

    float bossFacing = lurker->GetOrientation();
    float behindAngle = bossFacing + M_PI + frand(-0.5f, 0.5f) * (M_PI / 2.0f);
    float radius = frand(20.0f, 24.0f);

    float targetX = lurker->GetPositionX() + radius * std::cos(behindAngle);
    float targetY = lurker->GetPositionY() + radius * std::sin(behindAngle);

    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(SSC_MAP_ID, targetX, targetY, lurker->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TheLurkerBelowPositionMainTankAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (bot->GetVictim() != lurker)
        return Attack(lurker);

    const Position& position = LurkerMainTankPosition;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 0.2f)
    {
        return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Assign ranged positions within a 120-degree arc behind Lurker
bool TheLurkerBelowSpreadRangedAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    Group* group = bot->GetGroup();
    if (!lurker || !group)
        return false;

    if (lurker->GetHealth() == lurker->GetMaxHealth())
        lurkerRangedPositions.clear();

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    auto it = lurkerRangedPositions.find(guid);
    if (it == lurkerRangedPositions.end())
    {
        auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex = (findIt != rangedMembers.end()) ? std::distance(rangedMembers.begin(), findIt) : 0;
        size_t count = rangedMembers.size();
        if (count == 0)
            return false;

        const float minRadius = 25.0f;
        const float maxRadius = 27.0f;
        const float referenceOrientation = Position::NormalizeOrientation(2.262f + M_PI);

        const float arcSpan = 2.0f * M_PI / 3.0f; // 120°
        float startAngle = referenceOrientation - arcSpan / 2.0f;

        float angle;
        if (count == 1)
            angle = referenceOrientation;
        else
            angle = startAngle + (static_cast<float>(botIndex) / (count - 1)) * arcSpan;

        float radius = frand(minRadius, maxRadius);

        float tx = lurker->GetPositionX() + radius * std::cos(angle);
        float ty = lurker->GetPositionY() + radius * std::sin(angle);
        float tz = lurker->GetPositionZ();

        lurkerRangedPositions.emplace(guid, Position(tx, ty, tz));
        it = lurkerRangedPositions.find(guid);
    }

    if (it == lurkerRangedPositions.end())
        return false;

    const Position& target = it->second;
    const float returnThreshold = 2.0f;
    if (!bot->IsWithinDist2d(target.GetPositionX(), target.GetPositionY(), returnThreshold))
    {
        return MoveTo(SSC_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// If >= 3 tanks in the raid, the first 3 will each pick up 1 Guardian
bool TheLurkerBelowTanksPickUpAddsAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    Player* firstAssistTank = nullptr;
    Player* secondAssistTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (!mainTank && botAI->IsMainTank(member))
            mainTank = member;
        else if (!firstAssistTank && botAI->IsAssistTankOfIndex(member, 0))
            firstAssistTank = member;
        else if (!secondAssistTank && botAI->IsAssistTankOfIndex(member, 1))
            secondAssistTank = member;
    }

    if (!mainTank || !firstAssistTank || !secondAssistTank)
        return false;

    std::vector<Unit*> guardians;
    GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    for (auto guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_COILFANG_GUARDIAN)
            guardians.push_back(unit);
    }

    if (guardians.size() < 3)
        return false;

    std::vector<Player*> tanks = { mainTank, firstAssistTank, secondAssistTank };
    std::vector<uint8> rtiIndices =
    {
        RtiTargetValue::starIndex,
        RtiTargetValue::circleIndex,
        RtiTargetValue::diamondIndex
    };
    std::vector<std::string> rtiNames = { "star", "circle", "diamond" };

    for (size_t i = 0; i < 3; ++i)
    {
        Player* tank = tanks[i];
        Unit* guardian = guardians[i];
        if (bot == tank)
        {
            MarkTargetWithIcon(bot, guardian, rtiIndices[i]);
            SetRtiTarget(botAI, rtiNames[i], guardian);
            if (bot->GetVictim() != guardian)
            {
                bot->SetTarget(guardian->GetGUID());
                return Attack(guardian);
            }
        }
    }

    return false;
}

bool TheLurkerBelowManageSpoutTimerAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const time_t now = std::time(nullptr);

    if (lurker->GetHealth() == lurker->GetMaxHealth())
    {
        lurkerSpoutTimer.erase(SSC_MAP_ID);
        return false;
    }

    auto it = lurkerSpoutTimer.find(SSC_MAP_ID);
    if (it != lurkerSpoutTimer.end() && it->second <= now)
    {
        lurkerSpoutTimer.erase(it);
        it = lurkerSpoutTimer.end();
    }

    const time_t spoutCastTime = 20;
    if (IsLurkerCastingSpout(lurker) && it == lurkerSpoutTimer.end())
        lurkerSpoutTimer.emplace(SSC_MAP_ID, now + spoutCastTime);

    return false;
}

// Leotheras the Blind

bool LeotherasTheBlindTargetSpellbindersAction::Execute(Event event)
{
    Unit* spellbinder = GetFirstAliveUnitByEntry(botAI, NPC_GREYHEART_SPELLBINDER);
    if (!spellbinder || !spellbinder->IsInCombat())
        return false;

    MarkTargetWithSkull(bot, spellbinder);

    return false;
}

bool LeotherasTheBlindDemonFormTankAttackBossAction::Execute(Event event)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    MarkTargetWithSquare(bot, leotherasDemon);
    SetRtiTarget(botAI, "square", leotherasDemon);

    if (bot->GetTarget() != leotherasDemon->GetGUID())
    {
        bot->SetTarget(leotherasDemon->GetGUID());
        return Attack(leotherasDemon);
    }

    return false;
}

// Intent is to keep enough distance from Leotheras and spread to prepare for Whirlwind
// And stay away from the Warlock tank to avoid Chaos Blasts
bool LeotherasTheBlindPositionRangedAction::Execute(Event event)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    const uint32 minInterval = 500;
    if (bot->GetExactDist2d(leotheras) < 10.0f)
        return FleePosition(leotheras->GetPosition(), 12.0f, minInterval);

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;

            Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
            Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
            if (demonFormTank && demonFormTank == member &&
                leotherasDemon)
            {
                if (bot->GetExactDist2d(member) < 10.0f)
                    return FleePosition(member->GetPosition(), 12.0f, minInterval);
            }
            else if (bot->GetExactDist2d(member) < 5.0f)
                return FleePosition(member->GetPosition(), 6.0f, minInterval);
        }
    }

    return false;
}

bool LeotherasTheBlindRunAwayFromWhirlwindAction::Execute(Event event)
{
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    if (leotherasPhase3Demon && demonFormTank == bot)
        return false;

    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    if (leotherasHuman)
    {
        float currentDistance = bot->GetExactDist2d(leotherasHuman);
        const float safeDistance = 20.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(leotherasHuman, safeDistance - currentDistance + 5.0f);
        }
    }

    return false;
}

// Tanks and healers have no ability to kill their own Inner Demons
// Hunters, Affliction Warlocks, Shadow Priests, and (for some reason) Arcane Mages also struggle
bool LeotherasTheBlindInnerDemonCheatAction::Execute(Event event)
{
    Unit* innerDemon = nullptr;
    GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        Creature* creature = unit ? unit->ToCreature() : nullptr;
        if (creature && creature->IsAlive() && creature->GetEntry() == NPC_INNER_DEMON
            && creature->GetSummonerGUID() == bot->GetGUID())
        {
            innerDemon = creature;
            break;
        }
    }

    if (innerDemon)
    {
        uint8 tab = AiFactory::GetPlayerSpecTab(bot);
        Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

        if (botAI->IsHeal(bot) || botAI->IsTank(bot) ||
            bot->getClass() == CLASS_HUNTER ||
            (bot->getClass() == CLASS_PRIEST && tab == 2) ||
            (bot->getClass() == CLASS_WARLOCK && tab == 0) ||
            (bot->getClass() == CLASS_MAGE && tab == 0) ||
            (demonFormTank && demonFormTank == bot))
        {
            Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 25, nullptr,
                             DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);

            return true;
        }
    }

    return false;
}

// Everybody except the Warlock tank should focus on Leotheras in Phase 3
bool LeotherasTheBlindFinalPhaseAssignDpsPriorityAction::Execute(Event event)
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasDemon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasHuman || !leotherasDemon)
        return false;

    MarkTargetWithStar(bot, leotherasHuman);
    SetRtiTarget(botAI, "star", leotherasHuman);

    if (bot->GetVictim() != leotherasHuman)
    {
        bot->SetTarget(leotherasHuman->GetGUID());
        return Attack(leotherasHuman);
    }

    if (botAI->IsTank(bot) && leotherasHuman->GetVictim() == bot)
    {
        if (leotherasHuman->GetExactDist2d(leotherasDemon) < 25.0f)
        {
            float angle = atan2(bot->GetPositionY() - leotherasDemon->GetPositionY(),
                                bot->GetPositionX() - leotherasDemon->GetPositionX());
            float targetX = bot->GetPositionX() + 27.0f * std::cos(angle);
            float targetY = bot->GetPositionY() + 27.0f * std::sin(angle);

            return MoveTo(SSC_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, false,
                            MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool LeotherasTheBlindMisdirectBossToDemonFormTankAction::Execute(Event event)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (!leotherasDemon || !demonFormTank)
        return false;

    if (botAI->CanCastSpell("misdirection", demonFormTank))
        return botAI->CastSpell("misdirection", demonFormTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", leotherasDemon))
        return botAI->CastSpell("steady shot", leotherasDemon);

    return false;
}

// This does not pause DPS after a Whirlwind, which is also an aggro wipe
bool LeotherasTheBlindManageTimersAndTrackersAction::Execute(Event event)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    const time_t now = std::time(nullptr);

    // Encounter start/reset: clear all timers
    if (leotheras->HasAura(SPELL_LEOTHERAS_BANISHED))
    {
        leotherasHumanFormDpsWaitTimer.erase(SSC_MAP_ID);
        leotherasDemonFormDpsWaitTimer.erase(SSC_MAP_ID);
        leotherasFinalPhaseDpsWaitTimer.erase(SSC_MAP_ID);
        return false;
    }

    // Human Phase
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (leotherasHuman && !leotherasPhase3Demon)
    {
        leotherasHumanFormDpsWaitTimer.try_emplace(SSC_MAP_ID, now);
        leotherasDemonFormDpsWaitTimer.erase(SSC_MAP_ID);
    }
    // Demon Phase
    else if (Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI))
    {
        leotherasDemonFormDpsWaitTimer.try_emplace(SSC_MAP_ID, now);
        leotherasHumanFormDpsWaitTimer.erase(SSC_MAP_ID);
    }
    // Final Phase (<15% HP)
    else if (leotherasHuman && leotherasPhase3Demon)
    {
        leotherasFinalPhaseDpsWaitTimer.try_emplace(SSC_MAP_ID, now);
        leotherasHumanFormDpsWaitTimer.erase(SSC_MAP_ID);
        leotherasDemonFormDpsWaitTimer.erase(SSC_MAP_ID);
    }

    return false;
}

// Fathom-Lord Karathress
// Note: Four tanks are required for the full strategy, but
// Caribdis hits for nothing so just respec a DPS warrior and put on a shield

// Karathress is tanked near his starting position
bool FathomLordKarathressMainTankPositionBossAction::Execute(Event event)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    MarkTargetWithTriangle(bot, karathress);
    SetRtiTarget(botAI, "triangle", karathress);

    if (bot->GetVictim() != karathress)
        return Attack(karathress);

    if (karathress->GetVictim() == bot)
    {
        const Position& position = KarathressTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Caribdis is pulled far to the West in the corner
// Best to use a Warrior or Druid tank for interrupts
bool FathomLordKarathressFirstAssistTankPositionCaribdisAction::Execute(Event event)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    MarkTargetWithDiamond(bot, caribdis);
    SetRtiTarget(botAI, "diamond", caribdis);

    if (bot->GetVictim() != caribdis)
        return Attack(caribdis);

    if (caribdis->GetVictim() == bot)
    {
        const Position& position = CaribdisTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Sharkkis is pulled North to the other side of the ramp
bool FathomLordKarathressSecondAssistTankPositionSharkkisAction::Execute(Event event)
{
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (!sharkkis)
        return false;

    MarkTargetWithStar(bot, sharkkis);
    SetRtiTarget(botAI, "star", sharkkis);

    if (bot->GetVictim() != sharkkis)
        return Attack(sharkkis);

    if (sharkkis->GetVictim() == bot)
    {
        const Position& position = SharkkisTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Tidalvess is pulled Northwest near the pillar
bool FathomLordKarathressThirdAssistTankPositionTidalvessAction::Execute(Event event)
{
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (!tidalvess)
        return false;

    MarkTargetWithCircle(bot, tidalvess);
    SetRtiTarget(botAI, "circle", tidalvess);

    if (bot->GetVictim() != tidalvess)
        return Attack(tidalvess);

    if (tidalvess->GetVictim() == bot)
    {
        const Position& position = TidalvessTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Caribdis's tank spot is far away so a dedicated healer is needed
// Use the assistant flag to select the healer (Paladin recommended)
bool FathomLordKarathressPositionCaribdisTankHealerAction::Execute(Event event)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    const Position& position = CaribdisHealerPosition;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float dist = sqrt(dX * dX + dY * dY);
        float moveDist = std::min(7.0f, dist);
        float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
        float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

        return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Misdirect priority: (1) Caribdis tank, (2) Tidalvess tank, (3) Sharkkis tank
bool FathomLordKarathressMisdirectBossesToTanksAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);

        if (hunters.size() >= 3)
            break;
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* bossTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 0))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 1)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 2))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 2)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 1))
            {
                tankTarget = member;
                break;
            }
        }
    }

    if (!bossTarget || !tankTarget)
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", bossTarget))
        return botAI->CastSpell("steady shot", bossTarget);

    return false;
}

// Kill order is different from what is recommended for players because bots handle
// Caribdis Cyclones poorly and need more time to get her down (normally, ranged would help get
// Sharkkis down first)
bool FathomLordKarathressAssignDpsPriorityAction::Execute(Event event)
{
    // Target priority 1: Spitfire Totems for melee dps
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_SPITFIRE_TOTEM);
    if (totem && botAI->IsMelee(bot) && botAI->IsDps(bot))
    {
        MarkTargetWithSkull(bot, totem);
        SetRtiTarget(botAI, "skull", totem);

        if (bot->GetTarget() != totem->GetGUID())
        {
            bot->SetTarget(totem->GetGUID());
            return Attack(totem);
        }

        return false;
    }

    // Target priority 2: Tidalvess for all dps
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (tidalvess && tidalvess->IsAlive())
    {
        MarkTargetWithCircle(bot, tidalvess);
        SetRtiTarget(botAI, "circle", tidalvess);

        if (bot->GetTarget() != tidalvess->GetGUID())
        {
            bot->SetTarget(tidalvess->GetGUID());
            return Attack(tidalvess);
        }

        return false;
    }

    // Target priority 3: Caribdis for ranged
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (botAI->IsRangedDps(bot) && caribdis && caribdis->IsAlive())
    {
        MarkTargetWithDiamond(bot, caribdis);
        SetRtiTarget(botAI, "diamond", caribdis);

        const Position& position = CaribdisRangedDpsPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, true,
                        MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        if (bot->GetTarget() != caribdis->GetGUID())
        {
            bot->SetTarget(caribdis->GetGUID());
            return Attack(caribdis);
        }

        return false;
    }

    // Target priority 4: Sharkkis for melee (and ranged if Caribdis is down first)
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis && sharkkis->IsAlive())
    {
        MarkTargetWithStar(bot, sharkkis);
        SetRtiTarget(botAI, "star", sharkkis);

        if (bot->GetTarget() != sharkkis->GetGUID())
        {
            bot->SetTarget(sharkkis->GetGUID());
            return Attack(sharkkis);
        }

        return false;
    }

    // Target priority 5: Sharkkis pets for all dps
    Unit* fathomSporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat");
    if (fathomSporebat && fathomSporebat->IsAlive() && botAI->IsMelee(bot))
    {
        MarkTargetWithCross(bot, fathomSporebat);
        SetRtiTarget(botAI, "cross", fathomSporebat);

        if (bot->GetTarget() != fathomSporebat->GetGUID())
        {
            bot->SetTarget(fathomSporebat->GetGUID());
            return Attack(fathomSporebat);
        }

        return false;
    }

    Unit* fathomLurker = AI_VALUE2(Unit*, "find target", "fathom lurker");
    if (fathomLurker && fathomLurker->IsAlive() && botAI->IsMelee(bot))
    {
        MarkTargetWithSquare(bot, fathomLurker);
        SetRtiTarget(botAI, "square", fathomLurker);

        if (bot->GetTarget() != fathomLurker->GetGUID())
        {
            bot->SetTarget(fathomLurker->GetGUID());
            return Attack(fathomLurker);
        }

        return false;
    }

    // Target priority 6: Karathress for all dps
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress && karathress->IsAlive())
    {
        MarkTargetWithTriangle(bot, karathress);
        SetRtiTarget(botAI, "triangle", karathress);

        if (bot->GetTarget() != karathress->GetGUID())
        {
            bot->SetTarget(karathress->GetGUID());
            return Attack(karathress);
        }
    }

    return false;
}

bool FathomLordKarathressManageDpsTimerAction::Execute(Event event)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    const time_t now = std::time(nullptr);

    if (karathress->GetHealth() == karathress->GetMaxHealth())
        karathressDpsWaitTimer.insert_or_assign(SSC_MAP_ID, now);

    return false;
}

// Morogrim Tidewalker

bool MorogrimTidewalkerMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    Group* group = bot->GetGroup();
    if (!tidewalker || !group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", tidewalker))
        return botAI->CastSpell("steady shot", tidewalker);

    return false;
}

bool MorogrimTidewalkerMoveBossToTankPositionAction::Execute(Event event)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    if (bot->GetVictim() != tidewalker)
        return Attack(tidewalker);

    if (tidewalker->GetVictim() == bot)
    {
        if (tidewalker->GetHealthPct() > 26.0f)
            return MoveToPhase1TankPosition(tidewalker);
        else
            return MoveToPhase2TankPosition(tidewalker);
    }

    return false;
}

// Phase 1 tank position is up against the Northeast pillar
bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase1TankPosition(Unit* tidewalker)
{
    const Position& phase1 = TidewalkerPhase1TankPosition;
    if (bot->GetExactDist2d(phase1.GetPositionX(), phase1.GetPositionY()) > 1.0f)
    {
        float dX = phase1.GetPositionX() - bot->GetPositionX();
        float dY = phase1.GetPositionY() - bot->GetPositionY();
        float dist = sqrt(dX * dX + dY * dY);
        float moveDist = std::min(4.5f, dist);
        float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
        float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

        return MoveTo(SSC_MAP_ID, moveX, moveY, phase1.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

// Phase 2: move in two steps to get around the pillar and back into the Northeast corner
bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase2TankPosition(Unit* tidewalker)
{
    const Position& phase2 = TidewalkerPhase2TankPosition;
    const Position& transition = TidewalkerPhaseTransitionWaypoint;

    const ObjectGuid botGuid = bot->GetGUID();
    auto itStep = tidewalkerTankStep.find(botGuid);
    uint8 step = (itStep != tidewalkerTankStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        if (bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY()) > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(4.5f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, transition.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
        else
            tidewalkerTankStep.emplace(botGuid, 1);
    }

    if (step == 1)
    {
        if (bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY()) > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(4.5f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, phase2.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Stack behind the boss in the Northeast corner in phase 2
bool MorogrimTidewalkerPhase2RepositionRangedAction::Execute(Event event)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    const Position& phase2 = TidewalkerPhase2RangedPosition;
    const Position& transition = TidewalkerPhaseTransitionWaypoint;

    const ObjectGuid botGuid = bot->GetGUID();
    auto itStep = tidewalkerRangedStep.find(botGuid);
    uint8 step = (itStep != tidewalkerRangedStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        if (bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY()) > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, transition.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            tidewalkerRangedStep.emplace(botGuid, 1);
            step = 1;
        }
    }

    if (step == 1)
    {
        if (bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY()) > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, phase2.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool MorogrimTidewalkerResetPhaseTransitionStepsAction::Execute(Event event)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    const ObjectGuid botGuid = bot->GetGUID();

    if (tidewalker->GetHealth() == tidewalker->GetMaxHealth())
    {
        tidewalkerTankStep.erase(botGuid);
        tidewalkerRangedStep.erase(botGuid);
    }

    return false;
}

// Lady Vashj <Coilfang Matron>

// Center of room (phase 1 only)
bool LadyVashjMainTankPositionBossAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (bot->GetVictim() != vashj)
        return Attack(vashj);

    if (vashj->GetVictim() == bot)
    {
        if (IsLadyVashjInPhase1(botAI))
        {
            const Position& position = VashjPlatformCenterPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }

        if (IsLadyVashjInPhase3(botAI))
        {
            Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
            if (enchanted)
            {
                float currentDistance = bot->GetExactDist2d(enchanted);
                const float safeDistance = 10.0f;
                if (currentDistance < safeDistance)
                    return MoveAway(enchanted, safeDistance - currentDistance + 5.0f);
            }
        }
    }

    return false;
}

// Semicircle around center of the room (to allow escape by Static Charged bots)
bool LadyVashjPhase1PositionRangedAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> spreadMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && GET_PLAYERBOT_AI(member))
        {
            if (botAI->IsRanged(member))
                spreadMembers.push_back(member);
        }
    }

    const ObjectGuid guid = bot->GetGUID();

    auto itPos = vashjRangedPositions.find(guid);
    auto itReached = hasReachedVashjRangedPosition.find(guid);
    if (itPos == vashjRangedPositions.end())
    {
        auto it = std::find(spreadMembers.begin(), spreadMembers.end(), bot);
        size_t botIndex = (it != spreadMembers.end()) ? std::distance(spreadMembers.begin(), it) : 0;
        size_t count = spreadMembers.size();
        if (count == 0)
            return false;

        const Position& center = VashjPlatformCenterPosition;
        const float minRadius = 20.0f;
        const float maxRadius = 30.0f;

        const float referenceAngle = M_PI / 2.0f; // north
        const float arcSpan = M_PI; // 180°
        const float startAngle = referenceAngle - arcSpan / 2.0f;

        float angle;
        if (count == 1)
            angle = referenceAngle;
        else
            angle = startAngle + (static_cast<float>(botIndex) / (count - 1)) * arcSpan;

        float radius = frand(minRadius, maxRadius);
        float targetX = center.GetPositionX() + radius * std::cos(angle);
        float targetY = center.GetPositionY() + radius * std::sin(angle);
        float tz = center.GetPositionZ();

        auto res = vashjRangedPositions.emplace(guid, Position(targetX, targetY, tz));
        itPos = res.first;
        hasReachedVashjRangedPosition.emplace(guid, false);
        itReached = hasReachedVashjRangedPosition.find(guid);
    }

    if (itPos == vashjRangedPositions.end())
        return false;

    Position targetPosition = itPos->second;
    if (itReached == hasReachedVashjRangedPosition.end() || !(itReached->second))
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            return MoveTo(SSC_MAP_ID, targetPosition.GetPositionX(), targetPosition.GetPositionY(), targetPosition.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        if (itReached != hasReachedVashjRangedPosition.end())
            itReached->second = true;
    }

    return false;
}

// For absorbing Shock Burst
bool LadyVashjSetGroundingTotemInMainTankGroupAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    float dist = bot->GetExactDist2d(mainTank);
    if (dist >= 27.0f)
    {
        float angle = atan2(bot->GetPositionY() - mainTank->GetPositionY(),
                      bot->GetPositionX() - mainTank->GetPositionX());
        float targetX = mainTank->GetPositionX() + 25.0f * std::cos(angle);
        float targetY = mainTank->GetPositionY() + 25.0f * std::sin(angle);

        return MoveTo(SSC_MAP_ID, targetX, targetY, mainTank->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (!botAI->HasStrategy("grounding totem", BotState::BOT_STATE_COMBAT))
        botAI->ChangeStrategy("+grounding totem", BotState::BOT_STATE_COMBAT);

    if (!bot->HasAura(SPELL_GROUNDING_TOTEM_EFFECT) && botAI->CanCastSpell("grounding totem", bot))
        return botAI->CastSpell("grounding totem", bot);

    return false;
}

bool LadyVashjMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Group* group = bot->GetGroup();
    if (!vashj || !group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", vashj))
        return botAI->CastSpell("steady shot", vashj);

    return false;
}

bool LadyVashjStaticChargeMoveAwayFromGroupAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // If the main tank has Static Charge, other group members should move away
    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member) &&
            member->HasAura(SPELL_STATIC_CHARGE))
        {
            mainTank = member;
            break;
        }
    }

    if (mainTank && bot != mainTank)
    {
        float currentDistance = bot->GetExactDist2d(mainTank);
        const float safeDistance = 10.0f;
        if (currentDistance < safeDistance)
            return MoveFromGroup(safeDistance + 0.5f);
    }

    // If any other bot has static charge, it should move away from other group members
    if (!botAI->IsMainTank(bot) && bot->HasAura(SPELL_STATIC_CHARGE))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            float currentDistance = bot->GetExactDist2d(member);
            const float safeDistance = 10.0f;
            if (currentDistance < safeDistance)
                return MoveFromGroup(safeDistance + 0.5f);
        }
    }

    return false;
}

bool LadyVashjMisdirectStriderToFirstAssistTankAction::Execute(Event event)
{
    // Strider is not tankable without cheat
    if (!botAI->HasCheat(BotCheatMask::raid))
        return false;

    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* strider = GetFirstAliveUnitByEntry(botAI, NPC_COILFANG_STRIDER);
    Group* group = bot->GetGroup();
    if (!strider || !group)
        return false;

    Player* firstAssistTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 0))
        {
            firstAssistTank = member;
            break;
        }
    }

    if (!firstAssistTank || strider->GetVictim() == firstAssistTank)
        return false;

    if (botAI->CanCastSpell("misdirection", firstAssistTank))
        return botAI->CastSpell("misdirection", firstAssistTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", strider))
        return botAI->CastSpell("steady shot", strider);

    return false;
}

bool LadyVashjTankAttackAndMoveAwayStriderAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Unit* strider = GetFirstAliveUnitByEntry(botAI, NPC_COILFANG_STRIDER);
    Group* group = bot->GetGroup();
    if (!vashj || !strider || !group)
        return false;

    // Raid cheat automatically applies Fear Ward to tanks to make Strider tankable
    // This simulates the real-life strategy where the Strider can be meleed by
    // Bots wearing an Ogre Suit (due to the extended combat reach)
    if (botAI->HasCheat(BotCheatMask::raid) && botAI->IsTank(bot))
    {
        if (!bot->HasAura(SPELL_FEAR_WARD))
            bot->AddAura(SPELL_FEAR_WARD, bot);

        if (botAI->IsAssistTankOfIndex(bot, 0) && bot->GetVictim() != strider)
            return Attack(strider);

        if (strider->GetVictim() == bot)
        {
            float currentDistance = bot->GetExactDist2d(vashj);
            const float safeDistance = 25.0f;

            if (currentDistance < safeDistance)
                return MoveAway(vashj, safeDistance - currentDistance + 5.0f);
        }

        return false;
    }

    // Don't move away if raid cheats are enabled, or in any case if the bot is a tank
    if (!botAI->HasCheat(BotCheatMask::raid) || !botAI->IsTank(bot))
    {
        float currentDistance = bot->GetExactDist2d(strider);
        const float safeDistance = 15.0f;
        if (currentDistance < safeDistance)
            return MoveAway(strider, safeDistance - currentDistance + 5.0f);
    }

    // Try to root/slow the Strider if it is not tankable
    if (!botAI->HasCheat(BotCheatMask::raid))
    {
        if (!strider->HasAura(SPELL_HEAVY_NETHERWEAVE_NET))
        {
            Item* net = bot->GetItemByEntry(ITEM_HEAVY_NETHERWEAVE_NET);
            if (net && botAI->HasItemInInventory(ITEM_HEAVY_NETHERWEAVE_NET) &&
                botAI->CanCastSpell("heavy netherweave net", strider))
                return botAI->CastSpell("heavy netherweave net", strider);
        }

        if (!strider->HasAura(SPELL_FROST_SHOCK) && bot->getClass() == CLASS_SHAMAN &&
            botAI->CanCastSpell("frost shock", strider))
            return botAI->CastSpell("frost shock", strider);

        if (!strider->HasAura(SPELL_CURSE_OF_EXHAUSTION) && bot->getClass() == CLASS_WARLOCK &&
            botAI->CanCastSpell("curse of exhaustion", strider))
            return botAI->CastSpell("curse of exhaustion", strider);

        if (!strider->HasAura(SPELL_SLOW) && bot->getClass() == CLASS_MAGE &&
            botAI->CanCastSpell("slow", strider))
            return botAI->CastSpell("slow", strider);
    }

    return false;
}

bool LadyVashjAssignDpsPriorityAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    Unit* target = nullptr;
    Unit* tainted = nullptr;
    Unit* enchanted = nullptr;
    Unit* elite = nullptr;
    Unit* strider = nullptr;
    Unit* sporebat = nullptr;

    // Search and attack radius are intended to keep bots on the platform (not go down the stairs)
    const Position& center = VashjPlatformCenterPosition;
    const float maxSearchRange = botAI->IsRangedDps(bot) ? 60.0f : (botAI->IsMelee(bot) ? 55.0f : 40.0f);
    const float maxPursueRange = maxSearchRange - 5.0f;

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsValidLadyVashjCombatNpc(unit, botAI))
            continue;

        float distFromCenter = unit->GetExactDist2d(center.GetPositionX(), center.GetPositionY());
        if (IsLadyVashjInPhase2(botAI) && distFromCenter > maxSearchRange)
            continue;

        switch (unit->GetEntry())
        {
            case NPC_ENCHANTED_ELEMENTAL:
                if (!enchanted || vashj->GetExactDist2d(unit) < vashj->GetExactDist2d(enchanted))
                    enchanted = unit;
                break;

            case NPC_COILFANG_ELITE:
                if (!elite || unit->GetHealthPct() < elite->GetHealthPct())
                    elite = unit;
                break;

            case NPC_COILFANG_STRIDER:
                if (!strider || unit->GetHealthPct() < strider->GetHealthPct())
                    strider = unit;
                break;

            case NPC_TOXIC_SPOREBAT:
                if (!sporebat || unit->GetHealthPct() < sporebat->GetHealthPct())
                    sporebat = unit;
                break;

            case NPC_LADY_VASHJ:
                vashj = unit;
                break;

            default:
                break;
        }
    }

    std::vector<Unit*> targets;
    if (IsLadyVashjInPhase2(botAI))
    {
        if (botAI->IsRanged(bot))
        {
            // Hunters and Mages prioritize Enchanted Elementals, while other ranged DPS prioritize Striders
            // This works well with 3 Hunters and 2 Mages; effectiveness may vary based on raid composition
            if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_MAGE)
                targets = { enchanted, strider, elite };
            else
                targets = { strider, elite, enchanted };
        }
        else if (botAI->IsMelee(bot) && botAI->IsDps(bot))
            targets = { enchanted, elite };
        else if (botAI->IsTank(bot))
        {
            // With raid cheats enabled, the first assist tank will tank the Strider
            if (botAI->HasCheat(BotCheatMask::raid) && botAI->IsAssistTankOfIndex(bot, 0))
                targets = { strider, enchanted };
            else
                targets = { elite, enchanted };
        }
        else
            targets = { enchanted, elite, strider };
    }

    if (IsLadyVashjInPhase3(botAI))
    {
        if (botAI->IsTank(bot))
        {
            if (botAI->IsMainTank(bot))
            {
                MarkTargetWithDiamond(bot, vashj);
                SetRtiTarget(botAI, "diamond", vashj);
                targets = { vashj };
            }
            else if (botAI->IsAssistTankOfIndex(bot, 0))
            {
                if (botAI->HasCheat(BotCheatMask::raid))
                    targets = { strider, enchanted, vashj };
                else
                    targets = { elite, enchanted, vashj };
            }
            else
                targets = { elite, enchanted, vashj };
        }
        else if (botAI->IsRanged(bot))
        {
            // Hunters will try to kill Toxic Sporebats (in practice, they are generally not in range)
            if (bot->getClass() == CLASS_HUNTER)
                targets = { enchanted, sporebat, strider, elite, vashj };
            else
                targets = { enchanted, strider, elite, vashj };
        }
        else if (botAI->IsMelee(bot) && botAI->IsDps(bot))
            targets = { enchanted, elite, vashj };
        else
            targets = { enchanted, elite, strider, vashj };
    }

    for (Unit* candidate : targets)
    {
        if (candidate && candidate->IsAlive())
        {
            target = candidate;
            break;
        }
    }

    if (bot->GetVictim() == vashj)
    {
        if (IsLadyVashjInPhase2(botAI))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            bot->SetTarget(ObjectGuid::Empty);
            bot->SetSelection(ObjectGuid());
        }
        else if (IsLadyVashjInPhase3(botAI) && !botAI->IsMainTank(bot))
        {
            if ((enchanted && enchanted->IsAlive()) ||
                (elite && elite->IsAlive()) ||
                (strider && strider->IsAlive()))
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                bot->SetTarget(ObjectGuid::Empty);
                bot->SetSelection(ObjectGuid());
            }
        }
    }

    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();
    if (target && currentTarget == target && IsValidLadyVashjCombatNpc(currentTarget, botAI))
        return false;

    if (target && bot->GetExactDist2d(target) <= maxPursueRange &&
        bot->GetTarget() != target->GetGUID())
    {
        bot->SetTarget(target->GetGUID());
        return Attack(target);
    }

    if (currentTarget && (!currentTarget->IsAlive() || !IsValidLadyVashjCombatNpc(currentTarget, botAI)))
    {
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
    }

    // If bots have wandered too far from the center and are not attacking anything, move them back
    if (!bot->GetVictim())
    {
        Player* designatedLooter = GetDesignatedCoreLooter(bot->GetGroup(), botAI);
        if (designatedLooter && tainted && designatedLooter->GetExactDist2d(tainted) < 5.0f)
            return false;

        const Position& center = VashjPlatformCenterPosition;
        if (bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > 35.0f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);

            return MoveInside(SSC_MAP_ID, center.GetPositionX(), center.GetPositionY(),
                              center.GetPositionZ(), 30.0f, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

// If cheats are enabled, the first returned melee DPS bot will teleport to Tainted Elementals
// Such bot will recover HP and remove Poison Bolt debuff while attacking the elemental
bool LadyVashjTeleportToTaintedElementalAction::Execute(Event event)
{
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    lastTaintedGuid = tainted->GetGUID();
    if (bot->GetExactDist2d(tainted) >= 10.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        bot->TeleportTo(SSC_MAP_ID, tainted->GetPositionX(), tainted->GetPositionY(),
                        tainted->GetPositionZ(), tainted->GetOrientation());
    }

    if (bot->GetVictim() != tainted)
    {
        MarkTargetWithStar(bot, tainted);
        SetRtiTarget(botAI, "star", tainted);

        return Attack(tainted);
    }

    if (bot->GetExactDist2d(tainted) < 5.0f)
    {
        bot->SetFullHealth();
        bot->RemoveAura(SPELL_POISON_BOLT);
    }

    return false;
}

bool LadyVashjLootTaintedCoreAction::Execute(Event)
{
    GuidVector corpses = context->GetValue<GuidVector>("nearest corpses")->Get();
    const float maxLootRange = sPlayerbotAIConfig->lootDistance;

    for (auto const& guid : corpses)
    {
        LootObject loot(bot, guid);
        if (!loot.IsLootPossible(bot))
            continue;

        WorldObject* object = loot.GetWorldObject(bot);
        if (!object)
            continue;

        // The Tainted Elemental lootable corpse is a dead Creature object, not a corpse object
        Creature* creature = object->ToCreature();
        if (!creature)
            continue;

        if (creature->GetEntry() != NPC_TAINTED_ELEMENTAL || creature->IsAlive())
            continue;

        context->GetValue<LootObject>("loot target")->Set(loot);

        float dist = bot->GetDistance(object);

        if (dist > maxLootRange)
            return MoveTo(object, 2.0f, MovementPriority::MOVEMENT_FORCED);

        // Invoke OpenLootAction to request the server's StoreLoot packet for this corpse
        // Attempt a forced autostore (without modifying LootAction) by scheduling a short-timer to send
        // CMSG_AUTOSTORE_LOOT_ITEM (index 0) once the server has had time to send the StoreLoot packet
        OpenLootAction open(botAI);
        bool opened = open.Execute(Event());

        if (!opened)
            return opened;

        // If anyone in the group already has the core, skip creating a duplicate
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                    return true;
            }
        }

        // Schedule autostore attempt + reconcile fallback
        const ObjectGuid botGuid = bot->GetGUID();
        const ObjectGuid corpseGuid = guid;
        const uint8 coreIndex = 0;

        botAI->AddTimedEvent([botGuid, corpseGuid, coreIndex]()
        {
        Player* receiver = botGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(botGuid);
            if (!receiver)
                return;

            // Double-check someone else didn't obtain the core in the meantime using receiver's group
            if (Group* group = receiver->GetGroup())
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (member && member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                        return;
                }
            }

            // Set the loot GUID so server treats the following autostore as targeted to this corpse
            receiver->SetLootGUID(corpseGuid);

            WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
            *packet << coreIndex;
            receiver->GetSession()->QueuePacket(packet);
        }, 600);

        return true;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(group, botAI);
    if (!designatedLooter)
        return false;

    Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
    Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(group, botAI);
    Unit* closestTrigger = GetNearestActiveShieldGeneratorTriggerByEntry(bot, designatedLooter);

    if (!firstCorePasser || !secondCorePasser || !thirdCorePasser || !fourthCorePasser || !closestTrigger)
        return false;

    if (botAI->HasCheat(BotCheatMask::raid))
    {
        if (!bot->HasAura(SPELL_FEAR_WARD))
            bot->AddAura(SPELL_FEAR_WARD, bot);
    }

    const uint8 neededPassers = ComputeNeededPassers(designatedLooter, closestTrigger);

    // Passer order: HealAssistantOfIndex 0, 1, 2, then RangedDpsAssistantOfIndex 0
    if (bot == firstCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE) && neededPassers >= 1)
    {
        if (LineUpFirstCorePasser(designatedLooter, closestTrigger))
            return true;
    }
    else if (bot == secondCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE) && neededPassers >= 2)
    {
        if (LineUpSecondCorePasser(firstCorePasser, closestTrigger))
            return true;
    }
    else if (bot == thirdCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE) && neededPassers >= 3)
    {
        if (LineUpThirdCorePasser(secondCorePasser, closestTrigger))
            return true;
    }
    else if (bot == fourthCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE) && neededPassers >= 4)
    {
        if (LineUpFourthCorePasser(thirdCorePasser, closestTrigger))
            return true;
    }

    Item* item = bot->GetItemByEntry(ITEM_TAINTED_CORE);
    if (item && botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        // Designated core looter logic--applicable only if cheat mode is on and thus looter is a bot
        if (bot == designatedLooter)
        {
            if (IsFirstCorePasserInIntendedPosition(designatedLooter, firstCorePasser, closestTrigger))
            {
                const time_t now = std::time(nullptr);

                auto [it, inserted] = lastImbueAttempt.try_emplace(SSC_MAP_ID, now);
                if (inserted)
                {
                    botAI->ImbueItem(item, firstCorePasser);
                    ScheduleStoreCoreAfterImbue(botAI, bot, firstCorePasser);
                    return true;
                }
                if ((now - it->second) >= 2)
                {
                    it->second = now;
                    botAI->ImbueItem(item, firstCorePasser);
                    ScheduleStoreCoreAfterImbue(botAI, bot, firstCorePasser);
                    return true;
                }
            }
        }
        // First core passer: receive core from looter at the top of the stairs, pass to second core passer
        else if (bot == firstCorePasser)
        {
            if (IsSecondCorePasserInIntendedPosition(firstCorePasser, secondCorePasser, closestTrigger))
            {
                const time_t now = std::time(nullptr);

                auto [it, inserted] = lastImbueAttempt.try_emplace(SSC_MAP_ID, now);
                if (inserted)
                {
                    botAI->ImbueItem(item, secondCorePasser);
                    ScheduleStoreCoreAfterImbue(botAI, bot, secondCorePasser);
                    return true;
                }
                if ((now - it->second) >= 2)
                {
                    it->second = now;
                    botAI->ImbueItem(item, secondCorePasser);
                    ScheduleStoreCoreAfterImbue(botAI, bot, secondCorePasser);
                    return true;
                }
            }
        }
        // Second core passer: if closest usable generator is within passing distance of the first passer, move
        // to the generator; otherwise, move as close as possible to the generator while remaining in passing range
        // Usually, the second core passer will be able to use the generator, but it depends on where the elemental spawns
        else if (bot == secondCorePasser)
        {
            if (!UseCoreOnNearestGenerator())
            {
                if (IsThirdCorePasserInIntendedPosition(secondCorePasser, thirdCorePasser, closestTrigger))
                {
                    const time_t now = std::time(nullptr);

                    auto [it, inserted] = lastImbueAttempt.try_emplace(SSC_MAP_ID, now);
                    if (inserted)
                    {
                        botAI->ImbueItem(item, thirdCorePasser);
                        ScheduleStoreCoreAfterImbue(botAI, bot, thirdCorePasser);
                        return true;
                    }
                    if ((now - it->second) >= 2)
                    {
                        it->second = now;
                        botAI->ImbueItem(item, thirdCorePasser);
                        ScheduleStoreCoreAfterImbue(botAI, bot, thirdCorePasser);
                        return true;
                    }
                }
            }
        }
        // Third core passer: if closest usable generator is within passing distance of the second passer, move
        // to the generator; otherwise, move as close as possible to the generator while remaining in passing range
        else if (bot == thirdCorePasser)
        {
            if (!UseCoreOnNearestGenerator())
            {
                if (IsFourthCorePasserInIntendedPosition(thirdCorePasser, fourthCorePasser, closestTrigger))
                {
                    const time_t now = std::time(nullptr);

                    auto [it, inserted] = lastImbueAttempt.try_emplace(SSC_MAP_ID, now);
                    if (inserted)
                    {
                        botAI->ImbueItem(item, fourthCorePasser);
                        ScheduleStoreCoreAfterImbue(botAI, bot, fourthCorePasser);
                        return true;
                    }
                    if ((now - it->second) >= 2)
                    {
                        it->second = now;
                        botAI->ImbueItem(item, fourthCorePasser);
                        ScheduleStoreCoreAfterImbue(botAI, bot, fourthCorePasser);
                        return true;
                    }
                }
            }
        }
        // Fourth core passer: the fourth passer is rarely needed and will always be enough to reach a usable generator
        else if (bot == fourthCorePasser)
        {
            UseCoreOnNearestGenerator();
        }
    }

    return false;
}

// Determine, by distance to the closest trigger, how many core passers are needed
uint8 LadyVashjPassTheTaintedCoreAction::ComputeNeededPassers(Player* designatedLooter, Unit* closestTrigger)
{
    if (!designatedLooter || !closestTrigger)
        return static_cast<uint8>(1);

    const float farDistance = 38.0f;
    float dist = designatedLooter->GetExactDist2d(closestTrigger);

    int needed = 1;
    if (farDistance > 0.0f)
        needed = static_cast<int>(std::ceil(dist / farDistance));

    if (needed < 1) needed = 1;
    if (needed > 4) needed = 4;

    return static_cast<uint8>(needed);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFirstCorePasser(Player* designatedLooter, Unit* closestTrigger)
{
    const float centerX = VashjPlatformCenterPosition.GetPositionX();
    const float centerY = VashjPlatformCenterPosition.GetPositionY();
    const float radius = 57.5f;

    float mx = designatedLooter->GetPositionX();
    float my = designatedLooter->GetPositionY();
    float angle = atan2(my - centerY, mx - centerX);

    float targetX = centerX + radius * std::cos(angle);
    float targetY = centerY + radius * std::sin(angle);
    const float targetZ = 41.097f;

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, targetZ));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ,
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpSecondCorePasser(Player* firstCorePasser, Unit* closestTrigger)
{
    float fx = firstCorePasser->GetPositionX();
    float fy = firstCorePasser->GetPositionY();

    float dx = closestTrigger->GetPositionX() - fx;
    float dy = closestTrigger->GetPositionY() - fy;
    float distToTrigger = std::sqrt(dx*dx + dy*dy);

    if (distToTrigger == 0.0f)
        return false;

    dx /= distToTrigger; dy /= distToTrigger;

    // Target is on a line between firstCorePasser and closestTrigger
    float targetX, targetY, targetZ;
    // if firstCorePasser is within this distance of closestTrigger, go to nearTriggerDist short of closestTrigger
    const float thresholdDist = 40.0f;
    const float nearTriggerDist = 1.5f;
    // if firstCorePasser is not thresholdDist yards from closestTrigger, go to farDistance from firstCorePasser
    const float farDistance = 38.0f;

    if (distToTrigger <= thresholdDist)
    {
        float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
        targetX = fx + dx * moveDist;
        targetY = fy + dy * moveDist;
        targetZ = 42.985f;
    }
    else
    {
        targetX = fx + dx * farDistance;
        targetY = fy + dy * farDistance;
        targetZ = 42.985f;
    }

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, targetZ));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ,
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpThirdCorePasser(Player* secondCorePasser, Unit* closestTrigger)
{
    float sx = secondCorePasser->GetPositionX();
    float sy = secondCorePasser->GetPositionY();

    float dx = closestTrigger->GetPositionX() - sx;
    float dy = closestTrigger->GetPositionY() - sy;
    float distToTrigger = std::sqrt(dx*dx + dy*dy);

    if (distToTrigger == 0.0f)
        return false;

    dx /= distToTrigger; dy /= distToTrigger;

    float targetX, targetY, targetZ;
    const float thresholdDist = 40.0f;
    const float nearTriggerDist = 1.5f;
    const float farDistance = 38.0f;

    if (distToTrigger <= thresholdDist)
    {
        float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
        targetX = sx + dx * moveDist;
        targetY = sy + dy * moveDist;
        targetZ = 42.985f;
    }
    else
    {
        targetX = sx + dx * farDistance;
        targetY = sy + dy * farDistance;
        targetZ = 42.985f;
    }

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, targetZ));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ,
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFourthCorePasser(Player* thirdCorePasser, Unit* closestTrigger)
{
    float sx = thirdCorePasser->GetPositionX();
    float sy = thirdCorePasser->GetPositionY();

    float tx = closestTrigger->GetPositionX();
    float ty = closestTrigger->GetPositionY();

    float dx = tx - sx;
    float dy = ty - sy;
    float length = std::sqrt(dx*dx + dy*dy);

    if (length == 0.0f)
        return false;

    dx /= length; dy /= length;

    float targetX = tx - dx * 2.0f;
    float targetY = ty - dy * 2.0f;
    const float targetZ = 42.985f;

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, targetZ));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ,
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

// The next four functions check if the respective core passer is within 2 yards of their intended position
// And are used to determine when the prior bot in the chain can pass the core
// Known issue: if a passer bot is feared by a strider, the chain can be broken; if this happens, it is best
// to order bots to destroy cores to reset the sequence for the next elemental spawn
bool LadyVashjPassTheTaintedCoreAction::IsFirstCorePasserInIntendedPosition(
    Player* designatedLooter, Player* firstCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(firstCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = firstCorePasser->GetExactDist2d(itSnap->second.GetPositionX(), itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsSecondCorePasserInIntendedPosition(
    Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(secondCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = secondCorePasser->GetExactDist2d(itSnap->second.GetPositionX(), itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsThirdCorePasserInIntendedPosition(
    Player* secondCorePasser, Player* thirdCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(thirdCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = thirdCorePasser->GetExactDist2d(itSnap->second.GetPositionX(), itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsFourthCorePasserInIntendedPosition(
    Player* thirdCorePasser, Player* fourthCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(fourthCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = fourthCorePasser->GetExactDist2d(itSnap->second.GetPositionX(), itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

// Because ImbueItem() does not actually cause the receiving bot to receive the core, we have to simulate
// the passing mechanic by creating the core on the receiver (ImbueItem() does take away the core from the passer)
void LadyVashjPassTheTaintedCoreAction::ScheduleStoreCoreAfterImbue(PlayerbotAI* botAI, Player* giver, Player* receiver)
{
    if (!receiver)
        return;

    const uint32 delayMs = 1500;

    const ObjectGuid giverGuid    = giver ? giver->GetGUID() : ObjectGuid::Empty;
    const ObjectGuid receiverGuid = receiver->GetGUID();

    botAI->AddTimedEvent([botAI, giverGuid, receiverGuid]()
    {
        Player* receiverPlayer = receiverGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(receiverGuid);
        Player* giverPlayer    = giverGuid.IsEmpty()    ? nullptr : ObjectAccessor::FindPlayer(giverGuid);

        if (!receiverPlayer)
        {
            intendedLineup.erase(receiverGuid);
            intendedLineup.erase(giverGuid);
            return;
        }

        // Detect if anyone already has the core
        if (Group* group = receiverPlayer->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();

                if (!member)
                    continue;

                if (member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                {
                    intendedLineup.erase(receiverGuid);
                    intendedLineup.erase(giverGuid);
                    return;
                }
            }
        }

        if (receiverPlayer->HasItemCount(ITEM_TAINTED_CORE, 1, false))
        {
            intendedLineup.erase(receiverGuid);
            intendedLineup.erase(giverGuid);
            return;
        }

        // Store a new core into receiver inventory (sends client/db update)
        ItemPosCountVec dest;
        uint32 count = 1;
        int canStore = receiverPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_TAINTED_CORE, count);

        if (canStore == EQUIP_ERR_OK)
        {
            Item* created = receiverPlayer->StoreNewItem(dest, ITEM_TAINTED_CORE, true,
                Item::GenerateItemRandomPropertyId(ITEM_TAINTED_CORE));
            if (created)
            {
                intendedLineup.erase(receiverGuid);
                intendedLineup.erase(giverGuid);
            }
        }
        else
        {
            intendedLineup.erase(receiverGuid);
            intendedLineup.erase(giverGuid);
        }
    }, delayMs);
}

bool LadyVashjPassTheTaintedCoreAction::UseCoreOnNearestGenerator()
{
    std::vector<GeneratorInfo> generators = GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);
    const GeneratorInfo* nearestGen = GetNearestGeneratorToBot(bot, generators);
    if (!nearestGen)
        return false;

    GameObject* generator = botAI->GetGameObject(nearestGen->guid);
    if (!generator)
        return false;

    float dist = bot->GetExactDist2d(generator);
    if (dist > 4.5f)
        return false;

    if (Item* core = bot->GetItemByEntry(ITEM_TAINTED_CORE))
    {
        const uint8 bagIndex = core->GetBagSlot();
        const uint8 slot = core->GetSlot();
        const uint8 cast_count = 0;
        uint32 spellId = 0;

        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (core->GetTemplate()->Spells[i].SpellId > 0)
            {
                spellId = core->GetTemplate()->Spells[i].SpellId;
                break;
            }
        }

        const ObjectGuid item_guid = core->GetGUID();
        const uint32 glyphIndex = 0;
        const uint8 castFlags = 0;

        WorldPacket packet(CMSG_USE_ITEM);
        packet << bagIndex;
        packet << slot;
        packet << cast_count;
        packet << spellId;
        packet << item_guid;
        packet << glyphIndex;
        packet << castFlags;
        packet << (uint32)TARGET_FLAG_GAMEOBJECT;
        packet << generator->GetGUID().WriteAsPacked();

        bot->GetSession()->HandleUseItemOpcode(packet);
        return true;
    }

    return false;
}

// For dead bots to destroy their cores so the logic can reset for the next attempt
// Or residual cores to be destroyed in Phase 3
bool LadyVashjDestroyTaintedCoreAction::Execute(Event event)
{
    if (Item* core = bot->GetItemByEntry(ITEM_TAINTED_CORE))
    {
        bot->DestroyItem(core->GetBagSlot(), core->GetSlot(), true);
        return true;
    }

    return false;
}

// Custom Toxic Spore avoidance action
// The standard "avoid aoe" strategy does work, but I find it doesn't provide enough
// buffer distance for the toxic pools, and the standard strategy has a tendency to
// take bots down the stairs and get them stuck or out of LoS
bool LadyVashjAvoidToxicSporesAction::Execute(Event event)
{
    std::vector<Unit*> spores = GetAllSporeDropTriggers(botAI, bot);
    if (spores.empty())
        return false;

    const float hazardRadius = 7.0f;
    bool inDanger = false;
    for (Unit* spore : spores)
    {
        if (bot->GetExactDist2d(spore) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const Position& vashjCenter = VashjPlatformCenterPosition;
    const float maxRadius = 60.0f;

    Position safestPos = FindSafestNearbyPosition(spores, vashjCenter, maxRadius, hazardRadius);

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (vashj && vashj->GetVictim() == bot)
    {
        return MoveTo(SSC_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                      safestPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
    }
    else
    {
        return MoveTo(SSC_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                      safestPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
    }
}

Position LadyVashjAvoidToxicSporesAction::FindSafestNearbyPosition(const std::vector<Unit*>& spores,
    const Position& vashjCenter, float maxRadius, float hazardRadius)
{
    const float searchStep = M_PI / 8.0f;
    const float minDistance = 2.0f;
    const float maxDistance = 40.0f;
    const float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = 1000.0f;
    bool foundSafe = false;

    for (float distance = minDistance; distance <= maxDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * std::cos(angle);
            float y = bot->GetPositionY() + distance * std::sin(angle);
            float z = bot->GetPositionZ();

            if (vashjCenter.GetExactDist2d(x, y) > maxRadius)
                continue;

            bool isSafe = true;
            for (Unit* spore : spores)
            {
                if (spore->GetExactDist2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            Position testPos(x, y, z);

            bool pathSafe = IsPathSafeFromSpores(bot->GetPosition(), testPos, spores, hazardRadius);
            if (pathSafe || !foundSafe)
            {
                float moveDistance = bot->GetExactDist2d(x, y);

                if (pathSafe && (!foundSafe || moveDistance < minMoveDistance))
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                    foundSafe = true;
                }
                else if (!foundSafe && moveDistance < minMoveDistance)
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                }
            }
        }

        if (foundSafe)
            break;
    }

    return bestPos;
}

bool LadyVashjAvoidToxicSporesAction::IsPathSafeFromSpores(const Position& start,
    const Position& end, const std::vector<Unit*>& spores, float hazardRadius)
{
    const int numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (int i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* spore : spores)
        {
            float distToSpore = spore->GetExactDist2d(checkX, checkY);
            if (distToSpore < hazardRadius)
                return false;
        }
    }

    return true;
}

// When Toxic Sporebats spit poison, they summon "Spore Drop Trigger" NPCs that create the toxic pools
std::vector<Unit*> LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> sporeDropTriggers;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        const float maxSearchRadius = 40.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_SPORE_DROP_TRIGGER && bot->GetExactDist2d(unit) < maxSearchRadius)
            sporeDropTriggers.push_back(unit);
    }

    return sporeDropTriggers;
}

bool LadyVashjUseFreeActionAbilitiesAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> spores = LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(botAI, bot);
    const float toxicSporeRadius = 6.0f;

    // If Rogues are Entangled and either have Static Charge or are near a spore, use Cloak of Shadows
    if (bot->getClass() == CLASS_ROGUE && bot->HasAura(SPELL_ENTANGLE))
    {
        bool nearSpore = false;
        for (Unit* spore : spores)
        {
            if (bot->GetExactDist2d(spore) < toxicSporeRadius)
            {
                nearSpore = true;
                break;
            }
        }
        if ((bot->HasAura(SPELL_STATIC_CHARGE) || nearSpore) &&
            botAI->CanCastSpell("cloak of shadows", bot))
            return botAI->CastSpell("cloak of shadows", bot);
    }

    // The remainder of the logic is for Paladins to use Hand of Freedom
    Player* mainTankToxic = nullptr;
    Player* anyToxic = nullptr;
    Player* mainTankStatic = nullptr;
    Player* anyStatic = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (!member->HasAura(SPELL_ENTANGLE))
            continue;

        if (!botAI->IsMelee(member))
            continue;

        bool nearToxicSpore = false;
        for (Unit* spore : spores)
        {
            if (member->GetExactDist2d(spore) < toxicSporeRadius)
            {
                nearToxicSpore = true;
                break;
            }
        }

        if (nearToxicSpore)
        {
            if (botAI->IsMainTank(member))
                mainTankToxic = member;

            if (!anyToxic)
                anyToxic = member;
        }

        if (member->HasAura(SPELL_STATIC_CHARGE))
        {
            if (botAI->IsMainTank(member))
                mainTankStatic = member;

            if (!anyStatic)
                anyStatic = member;
        }
    }

    if (bot->getClass() == CLASS_PALADIN)
    {
        // Priority 1: Entangled in Toxic Spores (prefer main tank)
        Player* toxicTarget = mainTankToxic ? mainTankToxic : anyToxic;
        if (toxicTarget)
        {
            if (botAI->CanCastSpell("hand of freedom", toxicTarget))
                return botAI->CastSpell("hand of freedom", toxicTarget);
        }

        // Priority 2: Entangled with Static Charge (prefer main tank)
        Player* staticTarget = mainTankStatic ? mainTankStatic : anyStatic;
        if (staticTarget)
        {
            if (botAI->CanCastSpell("hand of freedom", staticTarget))
                return botAI->CastSpell("hand of freedom", staticTarget);
        }
    }

    return false;
}

bool LadyVashjManageTrackersAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    vashjRangedPositions.clear();
    hasReachedVashjRangedPosition.clear();
    lastImbueAttempt.clear();
    intendedLineup.clear();

    return false;
}
