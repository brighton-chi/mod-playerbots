#include "RaidZulAmanActions.h"
#include "RaidZulAmanHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

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

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", akilzon))
        return botAI->CastSpell("steady shot", akilzon);

    return false;
}

bool AkilzonTanksPositionBossAction::Execute(Event event)
{
    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (!akilzon)
        return false;

    if (bot->GetTarget() != akilzon->GetGUID())
        return Attack(akilzon);

    if (akilzon->GetVictim() == bot)
    {
        const Position& position = AKILZON_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool AkilzonSpreadRangedAction::Execute(Event event)
{
    constexpr float minDistance = 13.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
        return FleePosition(nearestPlayer->GetPosition(), minDistance, minInterval);

    return false;
}

bool AkilzonMoveToEyeOfTheStormAction::Execute(Event event)
{
    Player* stormTarget = GetElectricalStormTarget(bot);
    if (stormTarget && bot->GetExactDist2d(stormTarget) > 2.0f)
    {
        botAI->Reset();
        return MoveTo(ZULAMAN_MAP_ID, stormTarget->GetPositionX(), stormTarget->GetPositionY(),
                      bot->GetPositionZ(), false, false, false, true,
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

    Player* mainTank = GetGroupMainTank(botAI, bot);
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
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk)
        return false;

    if (botAI->IsMainTank(bot))
        return MainTankPositionTrollForm(nalorakk);
    else
        return FirstAssistTankPositionBearForm(nalorakk);
}

bool NalorakkTanksPositionBossAction::MainTankPositionTrollForm(Unit* nalorakk)
{
    if (!nalorakk->HasAura(SPELL_BEARFORM))
    {
        if (bot->GetTarget() != nalorakk->GetGUID())
            return Attack(nalorakk);

        if (nalorakk->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", Event(), false);

        const Position& position = NALORAKK_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else
    {
        const Position& position = NALORAKK_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool NalorakkTanksPositionBossAction::FirstAssistTankPositionBearForm(Unit* nalorakk)
{
    if (nalorakk->HasAura(SPELL_BEARFORM))
    {
        if (bot->GetTarget() != nalorakk->GetGUID())
            return Attack(nalorakk);

        if (nalorakk->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", Event(), false);

        const Position& position = NALORAKK_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else
    {
        const Position& position = NALORAKK_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool NalorakkSpreadRangedAction::Execute(Event event)
{
    constexpr float minDistance = 11.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
        return FleePosition(nearestPlayer->GetPosition(), minDistance, minInterval);

    return false;
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", janalai))
        return botAI->CastSpell("steady shot", janalai);

    return false;
}

bool JanalaiTanksPositionBossAction::Execute(Event event)
{
    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    if (bot->GetTarget() != janalai->GetGUID())
        return Attack(janalai);

    if (janalai->GetVictim() == bot)
    {
        const Position& position = JANALAI_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool JanalaiSpreadRangedInCircleAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t botIndex =
        (findIt != rangedMembers.end()) ? std::distance(rangedMembers.begin(), findIt) : 0;
    size_t count = rangedMembers.size();
    if (count == 0)
        return false;

    constexpr float radius = 15.0f;
    float angle = (count == 1) ? 0.0f :
            (2.0f * M_PI * static_cast<float>(botIndex) / static_cast<float>(count));

    float targetX = JANALAI_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
    float targetY = JANALAI_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
    {
        return MoveTo(ZULAMAN_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool JanalaiAvoidFireBombsAction::Execute(Event event)
{
    auto const& bombs = GetAllFireBombTriggers(botAI, bot);
    if (bombs.empty())
        return false;

    constexpr float hazardRadius = 6.0f;
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
    constexpr float maxRadius = 30.0f;

    Position safestPos = FindSafestNearbyPosition(bombs, janalaiCenter, maxRadius, hazardRadius);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(ZULAMAN_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  bot->GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

Position JanalaiAvoidFireBombsAction::FindSafestNearbyPosition(const std::vector<Unit*>& bombs,
    const Position& janalaiCenter, float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float minDistance = 2.0f;
    constexpr float maxDistance = 30.0f;
    constexpr float distanceStep = 1.0f;

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

bool JanalaiAvoidFireBombsAction::IsPathSafeFromFireBombs(const Position& start,
    const Position& end, const std::vector<Unit*>& bombs, float hazardRadius)
{
    constexpr uint8 numChecks = 10;
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

std::vector<Unit*> JanalaiAvoidFireBombsAction::GetAllFireBombTriggers(
    PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> fireBombs;
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        constexpr float maxSearchRadius = 40.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_FIRE_BOMB &&
            bot->GetExactDist2d(unit) < maxSearchRadius)
            fireBombs.push_back(unit);
    }

    return fireBombs;
}

bool JanalaiMarkAmaniHatchersAction::Execute(Event event)
{
    auto [hatcherLow, hatcherHigh] = GetAmaniHatcherPair(botAI);

    // When hatchers spawn, mark one with Skull and the other with Moon
    if (hatcherLow && hatcherHigh && hatcherHigh != hatcherLow)
    {
        MarkTargetWithSkull(bot, hatcherLow);
        MarkTargetWithMoon(bot, hatcherHigh);
        SetRtiTarget(botAI, "skull", hatcherLow);
        return true;
    }

    return false;
}

// Halazzi <Lynx Avatar>

bool HalazziMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (!halazzi)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
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

    if (halazzi->GetVictim() == bot)
    {
        const Position& position = HALAZZI_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HalazziFirstAssistTankAttackSpiritLynxAction::Execute(Event event)
{
    if (Unit* lynx = AI_VALUE2(Unit*, "find target", "spirit of the lynx"))
    {
        MarkTargetWithCircle(bot, lynx);
        SetRtiTarget(botAI, "circle", lynx);

        if (bot->GetTarget() != lynx->GetGUID())
            return Attack(lynx);

        if (lynx->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", Event(), false);

        const Position& position = HALAZZI_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else if (Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi"))
    {
        SetRtiTarget(botAI, "star", halazzi);

        if (bot->GetTarget() != halazzi->GetGUID())
            return Attack(halazzi);

        const Position& position = HALAZZI_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HalazziAssignDpsPriorityAction::Execute(Event event)
{
    // Target priority 1: Corrupted Lightning Totems
    if (Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_CORRUPTED_LIGHTNING_TOTEM))
    {
        MarkTargetWithSkull(bot, totem);
        SetRtiTarget(botAI, "skull", totem);

        if (bot->GetTarget() != totem->GetGUID())
            return Attack(totem);

        return false;
    }

    // Target priority 2: Halazzi
    if (Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi"))
    {
        SetRtiTarget(botAI, "star", halazzi);

        if (bot->GetTarget() != halazzi->GetGUID())
            return Attack(halazzi);
    }

    return false;
}

// Hex Lord Malacrass

bool HexLordMalacrassMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
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

    if (Unit* target = GetFirstAliveUnitByEntries(botAI, priorityEntries))
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

    const char* dispelSpells[] = { "devour magic", "dispel magic", "purge" };
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

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", zuljin))
        return botAI->CastSpell("steady shot", zuljin);

    return false;
}

bool ZuljinTanksPositionBossAction::Execute(Event event)
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    if (bot->GetTarget() != zuljin->GetGUID())
        return Attack(zuljin);

    if (zuljin->GetVictim() == bot)
    {
        const Position& position = ZULJIN_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool ZuljinRunAwayFromWhirlwindAction::Execute(Event event)
{
    if (Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin"))
    {
        float currentDistance = bot->GetExactDist2d(zuljin);
        constexpr float safeDistance = 10.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(zuljin, safeDistance - currentDistance);
        }
    }

    return false;
}

bool ZuljinAvoidCyclonesAction::Execute(Event event)
{
    auto const& cyclones = GetAllCycloneTriggers(botAI, bot);
    if (cyclones.empty())
        return false;

    const float hazardRadius = 6.0f;
    bool inDanger = false;
    for (Unit* cyclone : cyclones)
    {
        if (bot->GetExactDist2d(cyclone) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const Position& zuljinCenter = ZULJIN_TANK_POSITION;
    constexpr float maxRadius = 30.0f;

    Position safestPos =
        FindSafestNearbyPosition(cyclones, zuljinCenter, maxRadius, hazardRadius);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(ZULAMAN_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  bot->GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

Position ZuljinAvoidCyclonesAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& cyclones, const Position& zuljinCenter,
    float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float minDistance = 2.0f;
    constexpr float maxDistance = 30.0f;
    constexpr float distanceStep = 1.0f;

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

            if (zuljinCenter.GetExactDist2d(x, y) > maxRadius)
                continue;

            bool isSafe = true;
            for (Unit* cyclone : cyclones)
            {
                if (cyclone->GetExactDist2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            Position testPos(x, y, bot->GetPositionZ());

            bool pathSafe =
                IsPathSafeFromCyclones(bot->GetPosition(), testPos, cyclones, hazardRadius);
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

bool ZuljinAvoidCyclonesAction::IsPathSafeFromCyclones(const Position& start,
    const Position& end, const std::vector<Unit*>& cyclones, float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* cyclone : cyclones)
        {
            float distToCyclone = cyclone->GetExactDist2d(checkX, checkY);
            if (distToCyclone < hazardRadius)
                return false;
        }
    }

    return true;
}

std::vector<Unit*> ZuljinAvoidCyclonesAction::GetAllCycloneTriggers(
    PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> cyclones;
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        constexpr float maxSearchRadius = 30.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_FEATHER_VORTEX &&
            bot->GetExactDist2d(unit) < maxSearchRadius)
        {
            cyclones.push_back(unit);
        }
    }

    return cyclones;
}

bool ZuljinSpreadRangedAction::Execute(Event event)
{
    constexpr float minDistance = 6.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
        return FleePosition(nearestPlayer->GetPosition(), minDistance, minInterval);

    return false;
}
