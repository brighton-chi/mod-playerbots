#include "RaidBlackTempleActions.h"
#include "RaidBlackTempleHelpers.h"
#include "Playerbots.h"

using namespace BlackTempleHelpers;

// High Warlord Naj'entus

bool HighWarlordNajentusMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
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

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", najentus))
        return botAI->CastSpell("steady shot", najentus);

    return false;
}

bool HighWarlordNajentusMainTankPositionBossAction::Execute(Event event)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    if (bot->GetVictim() != najentus)
        return Attack(najentus);

    if (najentus->GetVictim() == bot && bot->IsWithinMeleeRange(najentus))
    {
        const Position& position = NAJENTUS_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool HighWarlordNajentusDisperseRangedAction::Execute(Event event)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    const uint32 minInterval = 1000;

    // Flee if within 10 yards of Naj'entus
    if (bot->GetExactDist2d(najentus) < 10.0f)
    {
        return FleePosition(Position(najentus->GetPositionX(), najentus->GetPositionY(),
                                     najentus->GetPositionZ()), 10.0f, minInterval);
    }

    // Flee if within 7 yards of any other player
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, 7.0f);
    if (nearestPlayer)
    {
        return FleePosition(Position(nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(),
                                     nearestPlayer->GetPositionZ()), 7.0f, minInterval);
    }

    return false;
}

bool HighWarlordNajentusRemoveImpalingSpineAction::Execute(Event event)
{
    // 1. Find the impaled player
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* impaledPlayer = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (member->HasAura(SPELL_IMPALING_SPINE))
        {
            impaledPlayer = member;
            break;
        }
    }
    if (!impaledPlayer)
        return false;

    // 2. Find the Naj'entus Spine GameObject near the impaled player
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    GameObject* spineGo = nullptr;
    for (ObjectGuid const& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go || !go->isSpawned())
            continue;
        if (go->GetEntry() != GO_NAJENTUS_SPINE)
            continue;
        spineGo = go;
        break;
    }
    if (!spineGo)
        return false;

    // 3. Move to the spine if not close enough
    float distToSpine = bot->GetExactDist2d(spineGo);
    if (distToSpine > 3.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, spineGo->GetPositionX(), spineGo->GetPositionY(), spineGo->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        // 4. Interact with the spine GameObject
        spineGo->Use(bot);
        return true;
    }

    return false;
}

bool HighWarlordNajentusThrowImpalingSpineAction::Execute(Event event)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    if (bot->GetExactDist2d(najentus) > 24.0f)
    {
        float angle = atan2(bot->GetPositionY() - najentus->GetPositionY(),
                            bot->GetPositionX() - najentus->GetPositionX());
        float targetX = najentus->GetPositionX() + 23.0f * std::cos(angle);
        float targetY = najentus->GetPositionY() + 23.0f * std::sin(angle);

        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, najentus->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    Item* spine = bot->GetItemByEntry(ITEM_NAJENTUS_SPINE);
    if (spine)
    {
        botAI->ImbueItem(spine, najentus);
        return true;
    }

    return false;
}

// Supremus

bool SupremusMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref && hunters.size() < 3; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER)
            hunters.push_back(member);
    }

    if (hunters.empty())
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

    Player* misdirectTarget = nullptr;
    if (bot == hunters[0] && mainTank)
        misdirectTarget = mainTank;
    else if (hunters.size() > 1 && bot == hunters[1] && firstAssistTank)
        misdirectTarget = firstAssistTank;
    else if (hunters.size() > 2 && bot == hunters[2] && secondAssistTank)
        misdirectTarget = secondAssistTank;

    if (!misdirectTarget)
        return false;

    if (botAI->CanCastSpell("misdirection", misdirectTarget))
        return botAI->CastSpell("misdirection", misdirectTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", supremus))
        return botAI->CastSpell("steady shot", supremus);

    return false;
}

bool SupremusDisperseRangedAction::Execute(Event event)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();

            if (!member || member == bot || !member->IsAlive())
                continue;

            if (bot->GetExactDist2d(member) < 8.0f)
                return FleePosition(Position(member->GetPositionX(), member->GetPositionY(),
                                    member->GetPositionZ()), 8.0f, 1000);
        }
    }

    return false;
}

bool SupremusKiteBossAction::Execute(Event event)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    float currentDistance = bot->GetDistance2d(supremus);
    const float safeDistance = 25.0f;
    if (currentDistance < safeDistance)
    {
        botAI->Reset();
        return MoveAway(supremus, safeDistance - currentDistance);
    }

    return false;
}

bool SupremusMoveAwayFromVolcanosAction::Execute(Event event)
{
    auto const& volcanos = GetAllSupremusVolcanos(botAI, bot);
    if (volcanos.empty())
        return false;

    const float hazardRadius = 14.0f;
    bool inDanger = false;
    for (Unit* volcano : volcanos)
    {
        if (bot->GetDistance2d(volcano) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const float maxRadius = 35.0f;
    Position safestPos = FindSafestNearbyPosition(volcanos, maxRadius, hazardRadius);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(BLACK_TEMPLE_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

Position SupremusMoveAwayFromVolcanosAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& volcanos, float maxRadius, float hazardRadius)
{
    const float searchStep = M_PI / 8.0f;
    const float minDistance = 2.0f;
    const float maxDistance = maxRadius;
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

            bool isSafe = true;
            for (Unit* volcano : volcanos)
            {
                if (volcano->GetDistance2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            Position testPos(x, y, bot->GetPositionZ());

            bool pathSafe =
                IsPathSafeFromVolcanos(bot->GetPosition(), testPos, volcanos, hazardRadius);
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

bool SupremusMoveAwayFromVolcanosAction::IsPathSafeFromVolcanos(const Position& start,
    const Position& end, const std::vector<Unit*>& volcanos, float hazardRadius)
{
    const uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* volcano : volcanos)
        {
            float distToVol = volcano->GetDistance2d(checkX, checkY);
            if (distToVol < hazardRadius)
                return false;
        }
    }

    return true;
}

std::vector<Unit*> SupremusMoveAwayFromVolcanosAction::GetAllSupremusVolcanos(
    PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> volcanos;
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        const float maxSearchRadius = 50.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_SUPREMUS_VOLCANO &&
            bot->GetDistance2d(unit) < maxSearchRadius)
            volcanos.push_back(unit);
    }

    return volcanos;
}

bool SupremusManagePhaseTimerAction::Execute(Event event)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    const time_t now = std::time(nullptr);

    if (supremus->GetHealthPct() > 99.8f)
        supremusPhaseTimer.insert_or_assign(supremus->GetMap()->GetInstanceId(), now);

    return false;
}

// Shade of Akama
// N/A

// Teron Gorefiend

bool TeronGorefiendMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
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

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", gorefiend))
        return botAI->CastSpell("steady shot", gorefiend);

    return false;
}

bool TeronGorefiendMainTankPositionBossAction::Execute(Event event)
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    if (bot->GetVictim() != gorefiend)
        return Attack(gorefiend);

    if (gorefiend->GetVictim() == bot && bot->IsWithinMeleeRange(gorefiend))
    {
        const Position& position = GOREFIEND_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool TeronGorefiendPositionRangedOnBalconyAction::Execute(Event event)
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    if (gorefiend->GetHealthPct() > 99.5f)
        gorefiendRangedPositions.clear();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

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

    auto it = gorefiendRangedPositions.find(guid);
    if (it == gorefiendRangedPositions.end())
    {
        size_t count = rangedMembers.size();
        auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex = (findIt != rangedMembers.end()) ?
            std::distance(rangedMembers.begin(), findIt) : 0;

        const float arcSpan = 2.0f * M_PI / 5.0f;
        const float arcCenter = 6.279f;
        const float arcStart = arcCenter - arcSpan / 2.0f;

        const float radius = 12.0f;
        float angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        float targetX = GOREFIEND_TANK_POSITION.GetPositionX() + radius * std::sin(angle + M_PI_2);
        float targetY = GOREFIEND_TANK_POSITION.GetPositionY() + radius * std::cos(angle + M_PI_2);

        gorefiendRangedPositions.try_emplace(guid, Position(targetX, targetY, bot->GetPositionZ()));
        it = gorefiendRangedPositions.find(guid);
    }

    if (it == gorefiendRangedPositions.end())
        return false;

    const Position& target = it->second;
    float distToTarget = bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY());
    if (distToTarget > 2.0f)
    {
        float dX = target.GetPositionX() - bot->GetPositionX();
        float dY = target.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToTarget);
        float moveX = bot->GetPositionX() + (dX / distToTarget) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToTarget) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, target.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TeronGorefiendAvoidShadowOfDeathAction::Execute(Event event)
{
    botAI->Reset();

    const char* abilities[] = { "divine shield", "feign death", "ice block", "vanish" };
    for (const char* spellName : abilities)
    {
        if (botAI->CanCastSpell(spellName, bot))
            return botAI->CastSpell(spellName, bot);
    }

    return false;
}

/* bool TeronGorefiendMoveToCornerToDieAction::Execute(Event event)
{
    const Position& position = GOREFIEND_DIE_POSITION;
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
    if (distToPosition > 2.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
} */
bool TeronGorefiendMoveToCornerToDieAction::Execute(Event event)
{
    const Position& position = GOREFIEND_DIE_POSITION;
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    LOG_DEBUG("playerbots", "TeronGorefiendMoveToCornerToDieAction: distToPosition={}", distToPosition);

    if (distToPosition > 2.0f)
    {
        LOG_DEBUG("playerbots", "Attempting MoveTo: mapId={}, x={}, y={}, z={}",
            BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ());

        // Diagnostic: check IsMovingAllowed
        if (!MovementAction::IsMovingAllowed(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ()))
        {
            LOG_DEBUG("playerbots", "MoveTo failed: IsMovingAllowed returned false");
            return false;
        }

        // Diagnostic: check IsDuplicateMove
        if (MovementAction::IsDuplicateMove(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ()))
        {
            LOG_DEBUG("playerbots", "MoveTo failed: IsDuplicateMove returned true");
            return false;
        }

        // Diagnostic: check IsWaitingForLastMove
        if (MovementAction::IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
        {
            LOG_DEBUG("playerbots", "MoveTo failed: IsWaitingForLastMove returned true");
            return false;
        }

        bool result = MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                             position.GetPositionZ(), false, false, false, true,
                             MovementPriority::MOVEMENT_FORCED, true, false);

        LOG_DEBUG("playerbots", "MoveTo returned {}", result);
        return result;
    }

    LOG_DEBUG("playerbots", "Already at die position, no move needed");
    return false;
}

/* bool TeronGorefiendControlAndDestroyShadowyConstructsAction::Execute(Event event)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();

    Unit* priorityTarget = nullptr;
    float bestDist = std::numeric_limits<float>::max();

    for (auto guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
        if (unit->GetEntry() != NPC_SHADOWY_CONSTRUCT)
            continue;

        float distToGorefiend = gorefiend->GetExactDist2d(unit);
        if (distToGorefiend < bestDist)
        {
            bestDist = distToGorefiend;
            priorityTarget = unit;
        }
    }

    if (!priorityTarget)
        return false;

    if (bot->GetExactDist2d(priorityTarget) > 11.0f)
    {
        return MoveTo(priorityTarget, 10.0f, MovementPriority::MOVEMENT_FORCED);
    }
    else if (botAI->CanCastSpell("spirit volley", priorityTarget))
    {
        return botAI->CastSpell("spirit volley", priorityTarget);
    }
    else if (botAI->CanCastSpell("spirit chains", priorityTarget))
    {
        return botAI->CastSpell("spirit chains", priorityTarget);
    }
    else if (botAI->CanCastSpell("spirit lance", priorityTarget))
    {
        return botAI->CastSpell("spirit lance", priorityTarget);
    }
    else if (bot->GetExactDist2d(gorefiend) > 5.0f)
    {
        return MoveTo(gorefiend, 5.0f, MovementPriority::MOVEMENT_FORCED);
    }
    else if (botAI->CanCastSpell("spirit strike", gorefiend))
    {
        return botAI->CastSpell("spirit strike", gorefiend);
    }

    return false;
} */

bool TeronGorefiendControlAndDestroyShadowyConstructsAction::Execute(Event event)
{
    LOG_DEBUG("playerbots", "TeronGorefiendControlAndDestroyShadowyConstructsAction: Start");

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
    {
        LOG_DEBUG("playerbots", "No Teron Gorefiend found");
        return false;
    }

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();

    Unit* priorityTarget = nullptr;
    float bestDist = std::numeric_limits<float>::max();

    for (auto guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
        if (unit->GetEntry() != NPC_SHADOWY_CONSTRUCT)
            continue;

        float distToGorefiend = gorefiend->GetExactDist2d(unit);
        if (distToGorefiend < bestDist)
        {
            bestDist = distToGorefiend;
            priorityTarget = unit;
        }
    }

    if (!priorityTarget)
    {
        LOG_DEBUG("playerbots", "No shadowy construct found");
        return false;
    }

    for (auto const& spell : bot->GetSpellMap())
    LOG_DEBUG("playerbots", "Bot has spell: id={}", spell.first);

    float distToTarget = bot->GetExactDist2d(priorityTarget);
    LOG_DEBUG("playerbots", "Priority target GUID: {}, Distance to bot: {}", priorityTarget->GetGUID().ToString(), distToTarget);

    if (distToTarget > 11.0f)
    {
        LOG_DEBUG("playerbots", "Moving to shadowy construct (distance: {})", distToTarget);

        // Diagnostic: check IsMovingAllowed
        if (!MovementAction::IsMovingAllowed(BLACK_TEMPLE_MAP_ID, priorityTarget->GetPositionX(), priorityTarget->GetPositionY(), priorityTarget->GetPositionZ()))
        {
            LOG_DEBUG("playerbots", "MoveTo failed: IsMovingAllowed returned false");
            return false;
        }

        // Diagnostic: check IsDuplicateMove
        if (MovementAction::IsDuplicateMove(BLACK_TEMPLE_MAP_ID, priorityTarget->GetPositionX(), priorityTarget->GetPositionY(), priorityTarget->GetPositionZ()))
        {
            LOG_DEBUG("playerbots", "MoveTo failed: IsDuplicateMove returned true");
            return false;
        }

        // Diagnostic: check IsWaitingForLastMove
        if (MovementAction::IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
        {
            LOG_DEBUG("playerbots", "MoveTo failed: IsWaitingForLastMove returned true");
            return false;
        }

        bool result = MoveTo(priorityTarget, 10.0f, MovementPriority::MOVEMENT_FORCED);
        LOG_DEBUG("playerbots", "MoveTo returned {}", result);
        return result;
    }
    else if (!bot->HasSpellCooldown(SPELL_SPIRIT_VOLLEY))
    {
        LOG_DEBUG("playerbots", "Casting spirit volley on shadowy construct");
        return bot->CastSpell(priorityTarget, SPELL_SPIRIT_VOLLEY);
    }
    else if (!bot->HasSpellCooldown(SPELL_SPIRIT_CHAINS))
    {
        LOG_DEBUG("playerbots", "Casting spirit chains on shadowy construct");
        return bot->CastSpell(priorityTarget, SPELL_SPIRIT_CHAINS);
    }
    else if (!bot->HasSpellCooldown(SPELL_SPIRIT_LANCE))
    {
        LOG_DEBUG("playerbots", "Casting spirit lance on shadowy construct");
        return bot->CastSpell(priorityTarget, SPELL_SPIRIT_LANCE);
    }
    else if (bot->GetExactDist2d(gorefiend) > 5.0f)
    {
        float distToGorefiend = bot->GetExactDist2d(gorefiend);
        LOG_DEBUG("playerbots", "Moving to Teron Gorefiend (distance: {})", distToGorefiend);
        return MoveTo(gorefiend, 5.0f, MovementPriority::MOVEMENT_FORCED);
    }
    else if (!bot->HasSpellCooldown(SPELL_SPIRIT_STRIKE))
    {
        LOG_DEBUG("playerbots", "Casting spirit strike on Teron Gorefiend");
        return bot->CastSpell(gorefiend, SPELL_SPIRIT_STRIKE);
    }

    LOG_DEBUG("playerbots", "No action taken");
    return false;
}

// Gurtogg Bloodboil

bool GurtoggBloodboilMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
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

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", gurtogg))
        return botAI->CastSpell("steady shot", gurtogg);

    return false;
}

bool GurtoggBloodboilTanksPositionBossAction::Execute(Event event)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    if (bot->GetVictim() != gurtogg)
        return Attack(gurtogg);

    Unit* victim = gurtogg->GetVictim();
    if (victim && botAI->IsTank(victim))
    {
        const Position& position = GURTOGG_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool GurtoggBloodboilDisperseRangedAction::Execute(Event event)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    const uint32 minInterval = 1000;
    const float minRange = 10.0f;
    const float maxRange = 20.0f;
    const float rangeBuffer = 0.5f;

    float distToGurtogg = bot->GetExactDist2d(gurtogg);

    if (distToGurtogg < (minRange - rangeBuffer))
    {
        return MoveTo(gurtogg, minRange, MovementPriority::MOVEMENT_FORCED);
    }
    else if (distToGurtogg > (maxRange + rangeBuffer))
    {
        return MoveTo(gurtogg, maxRange, MovementPriority::MOVEMENT_FORCED);
    }
    else if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, 5.0f))
    {
        return FleePosition(Position(nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(),
                                     nearestPlayer->GetPositionZ()), 5.0f, minInterval);
    }

    return false;
}

bool GurtoggBloodboilRangedMoveToAbsorbBloodboilPositionAction::Execute(Event event)
{
    const Position& position = GURTOGG_ABSORB_BLOODBOIL_POSITION;
    /* float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
    if (distToPosition > 2.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    } */
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveInside(BLACK_TEMPLE_MAP_ID, position.GetPositionX(),
               position.GetPositionY(), position.GetPositionZ(),
               3.0f, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool GurtoggBloodboilFelRagedBotMoveToTankPositionAction::Execute(Event event)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    const Position& position = GURTOGG_TANK_POSITION;
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
    if (distToPosition > 2.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(5.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }
    else
        bot->SetFacingTo(position.GetOrientation());

    return false;
}

bool GurtoggBloodboilManagePhaseTimerAction::Execute(Event event)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    const time_t now = std::time(nullptr);
    const uint32 instanceId = gurtogg->GetMap()->GetInstanceId();

    if (gurtogg->GetHealthPct() > 99.8f)
        gurtoggPhaseTimer.insert_or_assign(instanceId, now);
    else if (gurtogg->HasAura(SPELL_BOSS_FEL_RAGE))
        gurtoggPhaseTimer.erase(instanceId);
    else if (!gurtogg->HasAura(SPELL_BOSS_FEL_RAGE))
        gurtoggPhaseTimer.try_emplace(instanceId, now);

    return false;
}

// Reliquary of Souls

bool ReliquaryOfSoulsMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
    if (desire == nullptr && anger == nullptr)
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

    Unit* target = desire ? desire : anger;

    if (target->GetHealthPct() > 95.0f)
    {
        if (botAI->CanCastSpell("misdirection", mainTank))
            return botAI->CastSpell("misdirection", mainTank);

        if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", target))
            return botAI->CastSpell("steady shot", target);
    }

    return false;
}

bool ReliquaryOfSoulsHealersDpsEssenceOfSufferingAction::Execute(Event event)
{
    if (AI_VALUE2(Unit*, "find target", "essence of suffering"))
    {
        if (!botAI->HasStrategy("healer dps", BotState::BOT_STATE_COMBAT))
        {
            botAI->ChangeStrategy("+healer dps", BotState::BOT_STATE_COMBAT);
            return true;
        }
    }
    else if (AI_VALUE2(Unit*, "find target", "reliquary of the lost"))
    {
        if (botAI->HasStrategy("healer dps", BotState::BOT_STATE_COMBAT))
        {
            botAI->ChangeStrategy("-healer dps", BotState::BOT_STATE_COMBAT);
            return true;
        }
    }

    return false;
}

bool ReliquaryOfSoulsMeleeDpsStayAtMaxRangeFromEssenceOfSufferingAction::Execute(Event event)
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering)
        return false;

    float maxMeleeRange = bot->GetMeleeRange(suffering);
    const float meleeRangeBuffer = 0.5f;
    const float tolerance = 0.75f;

    float desiredDist = std::max(2.0f, maxMeleeRange - meleeRangeBuffer);
    float currentDist = bot->GetExactDist2d(suffering);

    if (fabs(currentDist - desiredDist) > tolerance)
    {
        float behindAngle = Position::NormalizeOrientation(suffering->GetOrientation() + M_PI);
        float targetX = suffering->GetPositionX() + desiredDist * std::cos(behindAngle);
        float targetY = suffering->GetPositionY() + desiredDist * std::sin(behindAngle);

        if (bot->GetExactDist2d(targetX, targetY) > tolerance)
        {
            return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, suffering->GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool ReliquaryOfSoulsSpellstealRuneShieldAction::Execute(Event event)
{
    if (Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire"))
    {
        if (botAI->CanCastSpell("spellsteal", desire))
            return botAI->CastSpell("spellsteal", desire);
    }

    return false;
}

bool ReliquaryOfSoulsSpellReflectDeadenAction::Execute(Event event)
{
    if (botAI->CanCastSpell("spell reflection", bot))
        return botAI->CastSpell("spell reflection", bot);

    return false;
}

bool ReliquaryOfSoulsManageDpsTimerAction::Execute(Event event)
{
    Unit* reliquary = AI_VALUE2(Unit*, "find target", "reliquary of the lost");
    if (!reliquary)
        return false;

    const time_t now = std::time(nullptr);

    if (Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering"))
    {
        if (suffering && suffering->GetHealthPct() > 99.8f)
            reliquaryDpsWaitTimer.insert_or_assign(reliquary->GetMap()->GetInstanceId(), now);
    }
    else if (Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire"))
    {
        if (desire && desire->GetHealthPct() > 99.8f)
            reliquaryDpsWaitTimer.insert_or_assign(reliquary->GetMap()->GetInstanceId(), now);
    }
    else if (Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger"))
    {
        if (anger && anger->GetHealthPct() > 99.8f)
            reliquaryDpsWaitTimer.insert_or_assign(reliquary->GetMap()->GetInstanceId(), now);
    }

    return false;
}

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
