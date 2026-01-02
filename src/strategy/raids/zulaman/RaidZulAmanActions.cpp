#include "RaidZulAmanActions.h"
#include "RaidZulAmanHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

using namespace ZulAmanHelpers;

// Trash

bool AmanishiMedicineManMarkWardAction::Execute(Event event)
{
    if (Unit* protectiveWard = GetFirstAliveUnitByEntry(botAI, NPC_AMANI_PROTECTIVE_WARD))
        MarkTargetWithSkull(bot, protectiveWard);
    else if (Unit* healingWard = GetFirstAliveUnitByEntry(botAI, NPC_AMANI_HEALING_WARD))
        MarkTargetWithSkull(bot, healingWard);

    return false;
}

// Akil'zon <Eagle Avatar>

bool AkilzonMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (!akilzon)
        return false;

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

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", akilzon))
        return botAI->CastSpell("steady shot", akilzon);

    return false;
}

bool AkilzonMainTankPositionBossAction::Execute(Event event)
{
    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (!akilzon)
        return false;

    if (bot->GetTarget() != akilzon->GetGUID())
        return Attack(akilzon);

    if (akilzon->GetVictim() == bot)
    {
        const Position& position = AKILZON_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool AkilzonSpreadRangedAction::Execute(Event event)
{
    const float minDistance = 13.0f;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
    {
        const uint32 minInterval = 1000;
        return FleePosition(nearestPlayer->GetPosition(), minDistance + 0.5f, minInterval);
    }

    return false;
}

bool AkilzonMoveToEyeOfTheStormAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* stormTarget = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;

        if (member->HasAura(SPELL_ELECTRICAL_STORM))
            stormTarget = member;
    }

    if (!stormTarget)
        return false;

    if (bot->GetExactDist2d(stormTarget) > 2.0f)
    {
        botAI->Reset();
        return MoveTo(ZULAMAN_MAP_ID, stormTarget->GetPositionX(), stormTarget->GetPositionY(),
                      stormTarget->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Nalorakk <Bear Avatar>

bool NalorakkMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk)
        return false;

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

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", nalorakk))
        return botAI->CastSpell("steady shot", nalorakk);

    return false;
}

bool NalorakkTanksPositionBossAction::Execute(Event event)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk)
        return false;

    if (botAI->IsMainTank(bot) && MainTankPositionTrollForm(bot, nalorakk))
        return true;

    if (botAI->IsAssistTankOfIndex(bot, 0) && FirstAssistTankPositionBearForm(bot, nalorakk))
        return true;

    return false;
}

bool NalorakkTanksPositionBossAction::MainTankPositionTrollForm(
    Player* mainTank, Unit* nalorakk)
{
    if (!mainTank)
        return false;

    if (!nalorakk->HasAura(SPELL_BEARFORM))
    {
        if (mainTank->GetTarget() != nalorakk->GetGUID())
        {
            return Attack(nalorakk);
        }
        else if (nalorakk->GetVictim() != mainTank)
        {
            const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, nalorakk))
                    return botAI->CastSpell(spellName, nalorakk);
            }
        }
        else if (nalorakk->GetVictim() == mainTank && mainTank->IsWithinMeleeRange(nalorakk))
        {
            const Position& position = NALORAKK_TANK_POSITION;
            float dist = mainTank->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
            if (dist > 4.0f)
            {
                float dX = position.GetPositionX() - mainTank->GetPositionX();
                float dY = position.GetPositionY() - mainTank->GetPositionY();
                float moveDist = std::min(10.0f, dist);
                float moveX = mainTank->GetPositionX() + (dX / dist) * moveDist;
                float moveY = mainTank->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                              false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }
    else
    {
        // Move in front of Nalorakk in Bear Form
        float bossX = nalorakk->GetPositionX();
        float bossY = nalorakk->GetPositionY();
        float bossZ = nalorakk->GetPositionZ();
        float bossO = nalorakk->GetOrientation();

        float frontDist = 3.0f;
        float targetX = bossX + std::cos(bossO) * frontDist;
        float targetY = bossY + std::sin(bossO) * frontDist;

        float dist = mainTank->GetExactDist2d(targetX, targetY);
        if (dist > 2.0f)
        {
            float dX = targetX - mainTank->GetPositionX();
            float dY = targetY - mainTank->GetPositionY();
            float moveDist = std::min(10.0f, dist);
            float moveX = mainTank->GetPositionX() + (dX / dist) * moveDist;
            float moveY = mainTank->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bossZ, false, false, false,
                          false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool NalorakkTanksPositionBossAction::FirstAssistTankPositionBearForm(
    Player* assistTank, Unit* nalorakk)
{
    if (!assistTank)
        return false;

    if (nalorakk->HasAura(SPELL_BEARFORM))
    {
        if (assistTank->GetTarget() != nalorakk->GetGUID())
        {
            return Attack(nalorakk);
        }
        else if (nalorakk->GetVictim() != assistTank)
        {
            const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, nalorakk))
                    return botAI->CastSpell(spellName, nalorakk);
            }
        }
        else if (nalorakk->GetVictim() == assistTank && assistTank->IsWithinMeleeRange(nalorakk))
        {
            const Position& position = NALORAKK_TANK_POSITION;
            float dist = assistTank->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
            if (dist > 2.0f)
            {
                float dX = position.GetPositionX() - assistTank->GetPositionX();
                float dY = position.GetPositionY() - assistTank->GetPositionY();
                float moveDist = std::min(10.0f, dist);
                float moveX = assistTank->GetPositionX() + (dX / dist) * moveDist;
                float moveY = assistTank->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                            false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }
    else
    {
        // Move in front of Nalorakk in Troll Form
        float bossX = nalorakk->GetPositionX();
        float bossY = nalorakk->GetPositionY();
        float bossZ = nalorakk->GetPositionZ();
        float bossO = nalorakk->GetOrientation();

        float frontDist = 3.0f;
        float targetX = bossX + std::cos(bossO) * frontDist;
        float targetY = bossY + std::sin(bossO) * frontDist;

        float dist = assistTank->GetExactDist2d(targetX, targetY);
        if (dist > 2.0f)
        {
            float dX = targetX - assistTank->GetPositionX();
            float dY = targetY - assistTank->GetPositionY();
            float moveDist = std::min(10.0f, dist);
            float moveX = assistTank->GetPositionX() + (dX / dist) * moveDist;
            float moveY = assistTank->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bossZ, false, false, false,
                          false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }


    return false;
}

bool NalorakkSpreadRangedAction::Execute(Event event)
{
    const float minDistance = 11.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);

    if (nearestPlayer)
    {
        const uint32 minInterval = 1000;
        return FleePosition(nearestPlayer->GetPosition(), minDistance + 1.0f, minInterval);
    }

    return false;
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

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

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", janalai))
        return botAI->CastSpell("steady shot", janalai);

    return false;
}

bool JanalaiMainTankPositionBossAction::Execute(Event event)
{
    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    if (bot->GetTarget() != janalai->GetGUID())
        return Attack(janalai);

    if (janalai->GetVictim() == bot && bot->IsWithinMeleeRange(janalai))
    {
        const Position& position = JANALAI_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool JanalaiSpreadRangedInCircleAction::Execute(Event event)
{
    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    if (janalai->GetHealth() == janalai->GetMaxHealth())
        janalaiRangedPositions.clear();

    std::vector<Player*> rangedMembers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsRanged(member))
                continue;

            rangedMembers.push_back(member);
        }
    }

    if (rangedMembers.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    auto it = janalaiRangedPositions.find(guid);
    if (it == janalaiRangedPositions.end())
    {
        auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex =
            (findIt != rangedMembers.end()) ? std::distance(rangedMembers.begin(), findIt) : 0;
        size_t count = rangedMembers.size();
        if (count == 0)
            return false;

        const float radius = 15.0f;
        float angle = (count == 1) ? 0.0f : (2.0f * M_PI * static_cast<float>(botIndex) / static_cast<float>(count));

        float tx = JANALAI_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
        float ty = JANALAI_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

        janalaiRangedPositions.try_emplace(guid, Position(tx, ty, JANALAI_TANK_POSITION.GetPositionZ()));
        it = janalaiRangedPositions.find(guid);
    }

    if (it == janalaiRangedPositions.end())
        return false;

    const Position& target = it->second;
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 2.0f)
    {
        return MoveTo(ZULAMAN_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                      target.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool JanalaiMoveAwayFromFireBombsAction::Execute(Event event)
{
    auto const& bombs = GetAllFireBombTriggers(botAI, bot);
    if (bombs.empty())
        return false;

    const float hazardRadius = 6.0f;
    bool inDanger = false;
    for (Unit* bomb : bombs)
    {
        if (bot->GetExactDist2d(bomb) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const Position& janalaiCenter = JANALAI_TANK_POSITION;
    const float maxRadius = 30.0f;

    Position safestPos = FindSafestNearbyPosition(bombs, janalaiCenter, maxRadius, hazardRadius);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(ZULAMAN_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

Position JanalaiMoveAwayFromFireBombsAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& bombs, const Position& janalaiCenter,
    float maxRadius, float hazardRadius)
{
    const float searchStep = M_PI / 8.0f;
    const float minDistance = 2.0f;
    const float maxDistance = 30.0f;
    const float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float distance = minDistance;
         distance <= maxDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * std::cos(angle);
            float y = bot->GetPositionY() + distance * std::sin(angle);

            if (janalaiCenter.GetExactDist2d(x, y) > maxRadius)
                continue;

            bool isSafe = true;
            for (Unit* bomb : bombs)
            {
                if (bomb->GetExactDist2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            Position testPos(x, y, bot->GetPositionZ());

            bool pathSafe =
                IsPathSafeFromFireBombs(bot->GetPosition(), testPos, bombs, hazardRadius);
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

bool JanalaiMoveAwayFromFireBombsAction::IsPathSafeFromFireBombs(const Position& start,
    const Position& end, const std::vector<Unit*>& bombs, float hazardRadius)
{
    const uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* bomb : bombs)
        {
            float distToBomb = bomb->GetExactDist2d(checkX, checkY);
            if (distToBomb < hazardRadius)
                return false;
        }
    }

    return true;
}

std::vector<Unit*> JanalaiMoveAwayFromFireBombsAction::GetAllFireBombTriggers(
    PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> fireBombs;
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        const float maxSearchRadius = 40.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_FIRE_BOMB &&
            bot->GetExactDist2d(unit) < maxSearchRadius)
            fireBombs.push_back(unit);
    }

    return fireBombs;
}

bool JanalaiMarkAmaniHatchersAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    auto [hatcherLow, hatcherHigh] = GetAmaniHatcherPair(botAI);

    // When hatchers spawn, mark one with Skull and the other with Moon
    if (hatcherLow && hatcherHigh && hatcherLow != hatcherHigh)
    {
        MarkTargetWithSkull(bot, hatcherLow);
        MarkTargetWithMoon(bot, hatcherHigh);
    }
    // Only one hatcher alive: mark with Moon unless already marked with Skull
    // This allows the player to override the mark when it is time to kill
    // the second hatcher (i.e., after all eggs are hatched for one side)
    else if (hatcherHigh && hatcherHigh->IsAlive() && group)
    {
        ObjectGuid guid = hatcherHigh->GetGUID();
        if (group->GetTargetIcon(RtiTargetValue::skullIndex) != guid)
            MarkTargetWithMoon(bot, hatcherHigh);
    }

    return false;
}

// Halazzi <Lynx Avatar>

bool HalazziMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (!halazzi)
        return false;

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

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", halazzi))
        return botAI->CastSpell("steady shot", halazzi);

    return false;
}

bool HalazziMainTankPositionBossAction::Execute(Event event)
{
    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (!halazzi)
        return false;

    MarkTargetWithStar(bot, halazzi);
    SetRtiTarget(botAI, "star", halazzi);

    if (bot->GetTarget() != halazzi->GetGUID())
        return Attack(halazzi);

    if (halazzi->GetVictim() == bot && bot->IsWithinMeleeRange(halazzi))
    {
        const Position& position = HALAZZI_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HalazziFirstAssistTankAttackSpiritLynxAction::Execute(Event event)
{
    Unit* lynx = AI_VALUE2(Unit*, "find target", "spirit of the lynx");
    if (lynx != nullptr)
    {
        MarkTargetWithCircle(bot, lynx);
        SetRtiTarget(botAI, "circle", lynx);

        if (bot->GetTarget() != lynx->GetGUID())
        {
            return Attack(lynx);
        }
        else if (lynx->GetVictim() != bot)
        {
            const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, lynx))
                    return botAI->CastSpell(spellName, lynx);
            }
        }
    }
    else if (Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi"))
    {
        SetRtiTarget(botAI, "star", halazzi);

        if (bot->GetTarget() != halazzi->GetGUID())
            return Attack(halazzi);

        float bossX = halazzi->GetPositionX();
        float bossY = halazzi->GetPositionY();
        float bossZ = halazzi->GetPositionZ();
        float bossO = halazzi->GetOrientation();

        float frontDist = 3.0f;
        float targetX = bossX + std::cos(bossO) * frontDist;
        float targetY = bossY + std::sin(bossO) * frontDist;

        float dist = bot->GetExactDist2d(targetX, targetY);
        if (dist > 2.0f)
        {
            float dX = targetX - bot->GetPositionX();
            float dY = targetY - bot->GetPositionY();
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bossZ, false, false, false,
                          false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HalazziAssignDpsPriorityAction::Execute(Event event)
{
    // Target priority 1: Corrupted Lightning Totems
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_CORRUPTED_LIGHTNING_TOTEM);
    if (totem)
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

    // Target priority 2: Halazzi
    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (halazzi)
    {
        SetRtiTarget(botAI, "star", halazzi);

        if (bot->GetTarget() != halazzi->GetGUID())
        {
            bot->SetTarget(halazzi->GetGUID());
            return Attack(halazzi);
        }
    }

    return false;
}

// Hex Lord Malacrass

bool HexLordMalacrassMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass)
        return false;

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

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", malacrass))
        return botAI->CastSpell("steady shot", malacrass);

    return false;
}

bool HexLordMalacrassAssignDpsPriorityAction::Execute(Event event)
{
    std::vector<uint32> priorityEntries =
    {
        NPC_LORD_RAADAN,
        NPC_ALYSON_ANTILLE,
        NPC_KORAGG,
        NPC_DARKHEART,
        NPC_FENSTALKER,
        NPC_GAZAKROTH,
        NPC_THURG,
        NPC_SLITHER,
        NPC_HEX_LORD_MALACRASS
    };

    if (GetFirstAliveUnitByEntries(botAI, priorityEntries))
    {
        MarkTargetWithSkull(bot, target);
        SetRtiTarget(botAI, "skull", target);
    }

    return false;
}

bool HexLordMalacrassPurgeBuffFromBossAction::Execute(Event event)
{
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass)
        return false;

    const char* dispelSpells[] = {
        "arcane shot", "devour magic", "dispel magic", "purge", "spellsteal" };
    for (const char* spellName : dispelSpells)
    {
        if (botAI->CanCastSpell(spellName, malacrass))
            return botAI->CastSpell(spellName, malacrass);
    }

    return false;
}

bool HexLordMalacrassDispelMindControlAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mcTarget = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (member->HasAura(SPELL_MIND_CONTROL))
        {
            mcTarget = member;
            break;
        }
    }

    if (!mcTarget)
        return false;

    const char* dispelSpells[] = {
        "devour magic", "dispel magic", "purge" };
    for (const char* spellName : dispelSpells)
    {
        if (botAI->CanCastSpell(spellName, mcTarget))
            return botAI->CastSpell(spellName, mcTarget);
    }

    return false;
}

// Zul'jin

bool ZuljinMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

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

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", zuljin))
        return botAI->CastSpell("steady shot", zuljin);

    return false;
}

bool ZuljinMainTankPositionBossAction::Execute(Event event)
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    if (bot->GetTarget() != zuljin->GetGUID())
        return Attack(zuljin);

    if (zuljin->GetVictim() == bot && bot->IsWithinMeleeRange(zuljin))
    {
        const Position& position = ZULJIN_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool ZuljinRunAwayFromWhirlwindAction::Execute(Event event)
{
    if (Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin"))
    {
        float currentDistance = bot->GetExactDist2d(zuljin);
        const float safeDistance = 10.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(zuljin, safeDistance - currentDistance + 1.0f);
        }
    }

    return false;
}

bool ZuljinSpreadRangedAction::Execute(Event event)
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    const float minDistance = zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE) ? 14.0f : 5.0f;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
    {
        const uint32 minInterval = 1000;
        return FleePosition(nearestPlayer->GetPosition(), minDistance + 1.0f, minInterval);
    }

    return false;
}

bool ZuljinMoveNearGroupAction::Execute(Event event)
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    if (bot->GetExactDist2d(zuljin) > 16.0f)
    {
        return MoveInside(ZULAMAN_MAP_ID, zuljin->GetPositionX(), zuljin->GetPositionY(),
                          zuljin->GetPositionZ(), 15.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}
