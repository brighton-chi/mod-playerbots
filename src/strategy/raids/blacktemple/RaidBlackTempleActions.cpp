#include "RaidBlackTempleActions.h"
#include "RaidBlackTempleHelpers.h"
#include "RaidBlackTempleIllidanBossAI.h"
#include "Playerbots.h"

using namespace BlackTempleHelpers;

// General

bool BlackTempleEraseTimersAndTrackersAction::Execute(Event event)
{
    const ObjectGuid guid = bot->GetGUID();
    const uint32 instanceId = bot->GetMap()->GetInstanceId();

    bool erased = false;
    if (!AI_VALUE2(Unit*, "find target", "supremus"))
    {
        if (supremusPhaseTimer.erase(instanceId))
            erased = true;
    }
    else if (!AI_VALUE2(Unit*, "find target", "teron gorefiend"))
    {
        if (gorefiendRangedPositions.erase(guid))
            erased = true;
    }
    else if (!AI_VALUE2(Unit*, "find target", "gurtogg bloodboil"))
    {
        if (gurtoggPhaseTimer.erase(instanceId))
            erased = true;
    }
    else if (!AI_VALUE2(Unit*, "find target", "reliquary of the lost"))
    {
        if (reliquaryDpsWaitTimer.erase(instanceId))
            erased = true;
    }
    else if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
    {
        if (shahrazTankStep.erase(guid))
            erased = true;
    }
    else if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
    {
        if (gathiosTankStep.erase(guid))
            erased = true;
    }
    else if (!AI_VALUE2(Unit*, "find target", "illidan stormrage"))
    {
        if (illidanDpsWaitTimer.erase(instanceId))
            erased = true;
        if (westFlameGuid.erase(instanceId))
            erased = true;
        if (eastFlameGuid.erase(instanceId))
            erased = true;
        if (flameTankWaypointIndex.erase(guid))
            erased = true;
    }

    return erased;
}

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
        if (distToPosition > 3.0f)
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
        // 4. Interact with the spine to remove it, with a random delay
        uint32 delay = urand(1000, 2000); // 1 to 2 seconds
        botAI->AddTimedEvent(
            [this, spineGo]() {
                if (spineGo && bot)
                    spineGo->Use(bot);
            },
            delay);
        botAI->SetNextCheckDelay(delay + 50);
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

    if (Item* spine = bot->GetItemByEntry(ITEM_NAJENTUS_SPINE))
    {
        uint32 delay = urand(1000, 2000); // 1 to 2 seconds
        botAI->AddTimedEvent(
            [this, spine, najentus]() {
                if (spine && najentus)
                    botAI->ImbueItem(spine, najentus);
            },
            delay);
        botAI->SetNextCheckDelay(delay + 50);
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
        else if (!firstAssistTank && botAI->IsAssistTankOfIndex(member, 0, true))
            firstAssistTank = member;
        else if (!secondAssistTank && botAI->IsAssistTankOfIndex(member, 1, true))
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
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();

            if (!member || member == bot || !member->IsAlive())
                continue;

            const float safeDistance = 8.0f;
            const uint32 minInterval = 1000;
            if (bot->GetExactDist2d(member) < safeDistance)
            {
                return FleePosition(Position(member->GetPositionX(), member->GetPositionY(),
                                             member->GetPositionZ()), safeDistance, minInterval);
            }
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
    auto const& volcanos = GetAllSupremusVolcanos();
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

std::vector<Unit*> SupremusMoveAwayFromVolcanosAction::GetAllSupremusVolcanos()
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

    supremusPhaseTimer.try_emplace(
        supremus->GetMap()->GetInstanceId(), std::time(nullptr));

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

    MarkTargetWithSkull(bot, gorefiend);

    if (bot->GetVictim() != gorefiend)
        return Attack(gorefiend);

    if (gorefiend->GetVictim() == bot && bot->IsWithinMeleeRange(gorefiend))
    {
        const Position& position = GOREFIEND_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 3.0f)
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

// Assume positions in arc at the edge of the balcony (farthest from Constructs)
bool TeronGorefiendPositionRangedOnBalconyAction::Execute(Event event)
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

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

bool TeronGorefiendMoveToCornerToDieAction::Execute(Event event)
{
    const Position& position = GOREFIEND_DIE_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TeronGorefiendControlAndDestroyShadowyConstructsAction::Execute(Event event)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    Unit* spirit = bot->GetCharm();
    if (!spirit)
        return false;

    auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();

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

    if (priorityTarget)
    {
        if (spirit->GetDistance2d(priorityTarget) > 11.0f)
        {
            float moveX = priorityTarget->GetPositionX();
            float moveY = priorityTarget->GetPositionY();
            float moveZ = priorityTarget->GetPositionZ();
            spirit->GetMotionMaster()->MovePoint(0, moveX, moveY, moveZ);
            return true;
        }
        else
        {
            if (!spirit->HasSpellCooldown(SPELL_SPIRIT_VOLLEY))
            {
                spirit->CastSpell(priorityTarget, SPELL_SPIRIT_VOLLEY, true);
                spirit->AddSpellCooldown(SPELL_SPIRIT_VOLLEY, 0, 15000);
                return true;
            }
            else if (!spirit->HasSpellCooldown(SPELL_SPIRIT_CHAINS))
            {
                spirit->CastSpell(priorityTarget, SPELL_SPIRIT_CHAINS, true);
                spirit->AddSpellCooldown(SPELL_SPIRIT_CHAINS, 0, 15000);
                return true;
            }
            else if (!spirit->HasSpellCooldown(SPELL_SPIRIT_LANCE))
            {
                spirit->CastSpell(priorityTarget, SPELL_SPIRIT_LANCE, true);
                spirit->AddSpellCooldown(SPELL_SPIRIT_LANCE, 0, 1000);
                return true;
            }
        }
    }

    if (spirit->GetDistance2d(gorefiend) > 5.0f)
    {
        float moveX = gorefiend->GetPositionX();
        float moveY = gorefiend->GetPositionY();
        float moveZ = gorefiend->GetPositionZ();
        spirit->GetMotionMaster()->MovePoint(0, moveX, moveY, moveZ);
        return true;
    }
    else
    {
        spirit->CastSpell(gorefiend, SPELL_SPIRIT_STRIKE, true);
        spirit->AddSpellCooldown(SPELL_SPIRIT_STRIKE, 0, 1000);
        return true;
    }

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
    Player* playerVictim = victim ? victim->ToPlayer() : nullptr;
    if (playerVictim && botAI->IsTank(playerVictim))
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

    const float minRange = 10.0f;
    const float maxRange = 20.0f;
    const float rangeBuffer = 0.5f;
    const uint32 minInterval = 1000;
    const float safeDistance = 5.0f;

    float distToGurtogg = bot->GetExactDist2d(gurtogg);

    if (distToGurtogg < (minRange - rangeBuffer))
    {
        return MoveTo(gurtogg, minRange, MovementPriority::MOVEMENT_FORCED);
    }
    else if (distToGurtogg > (maxRange + rangeBuffer))
    {
        return MoveTo(gurtogg, maxRange, MovementPriority::MOVEMENT_FORCED);
    }
    else if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
    {
        return FleePosition(Position(nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(),
                                     nearestPlayer->GetPositionZ()), safeDistance, minInterval);
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
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    } */
    const float distToPosition = 3.0f;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > distToPosition)
    {
        return MoveInside(BLACK_TEMPLE_MAP_ID, position.GetPositionX(),
               position.GetPositionY(), position.GetPositionZ(),
               distToPosition, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* enragedPlayer = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(SPELL_PLAYER_FEL_RAGE))
        {
            enragedPlayer = member;
            break;
        }
    }

    if (enragedPlayer)
    {
        const float safeDistance = 20.0f;
        if (bot->GetExactDist2d(enragedPlayer) < safeDistance)
        {
            const uint32 minInterval = 0;
            return FleePosition(Position(enragedPlayer->GetPositionX(), enragedPlayer->GetPositionY(),
                                         enragedPlayer->GetPositionZ()), safeDistance, minInterval);
        }
    }

    return false;
}

bool GurtoggBloodboilManagePhaseTimerAction::Execute(Event event)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    const time_t now = std::time(nullptr);
    const uint32 instanceId = gurtogg->GetMap()->GetInstanceId();

    if (gurtogg->HasAura(SPELL_BOSS_FEL_RAGE))
    {
        if (gurtoggPhaseTimer.count(instanceId))
        {
            gurtoggPhaseTimer.erase(instanceId);
            return true;
        }
        return false;
    }
    else
    {
        auto [it, inserted] = reliquaryDpsWaitTimer.try_emplace(instanceId, now);
        return inserted;
    }
}

// Reliquary of Souls

bool ReliquaryOfSoulsMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
    if (!desire && !anger)
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

    float desiredDist = bot->GetMeleeRange(suffering) - 0.5f;
    const float tolerance = 0.25f;
    if (fabs(bot->GetExactDist2d(suffering) - desiredDist) > tolerance)
    {
        float behindAngle = Position::NormalizeOrientation(suffering->GetOrientation() + M_PI);
        float targetX = suffering->GetPositionX() + desiredDist * std::cos(behindAngle);
        float targetY = suffering->GetPositionY() + desiredDist * std::sin(behindAngle);

        if (bot->GetExactDist2d(targetX, targetY) > tolerance)
        {
            return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(), false,
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

    const uint32 instanceId = reliquary->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");

    if (!suffering && !desire && !anger)
    {
        if (reliquaryDpsWaitTimer.count(instanceId))
        {
            reliquaryDpsWaitTimer.erase(instanceId);
            return true;
        }
        return false;
    }
    else
    {
        auto [it, inserted] = reliquaryDpsWaitTimer.try_emplace(instanceId, now);
        return inserted;
    }
}

// Mother Shahraz

bool MotherShahrazMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz)
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

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", shahraz))
        return botAI->CastSpell("steady shot", shahraz);

    return false;
}

bool MotherShahrazTanksPositionBossAction::Execute(Event event)
{
    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz)
        return false;

    if (bot->GetVictim() != shahraz)
        return Attack(shahraz);

    Unit* victim = shahraz->GetVictim();
    Player* playerVictim = victim ? victim->ToPlayer() : nullptr;
    if (playerVictim && botAI->IsTank(playerVictim))
    {
        const ObjectGuid guid = bot->GetGUID();
        uint8 step = shahrazTankStep.count(guid) ? shahrazTankStep[guid] : 0;

        const Position tankPositions[2] =
        {
            SHAHRAZ_TRANSITION_POSITION,
            SHAHRAZ_TANK_POSITION
        };
        const float maxDistance = 0.5f;
        const Position& position = tankPositions[step];
        float distanceToTarget = bot->GetExactDist2d(position);

        if ((distanceToTarget > maxDistance) && bot->IsWithinMeleeRange(shahraz))
        {
            bool backwards = (shahraz->GetVictim() == bot);
            MovementPriority priority = backwards ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;
            return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                          false, false, false, false, priority, true, backwards);
        }

        if (step == 0 && distanceToTarget <= maxDistance)
            shahrazTankStep[guid] = 1;

        if (step == 1 && distanceToTarget <= maxDistance)
        {
            float orientation = atan2(shahraz->GetPositionY() - bot->GetPositionY(),
                                      shahraz->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
    }

    return false;
}

// This doesn't actually work for bots--they don't obey the collision of the statue...
// But it's still cool to get them to go to the right place!
bool MotherShahrazPositionRangedUnderStatueAction::Execute(Event event)
{
    const Position& position = SHAHRAZ_RANGED_POSITION;
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
    if (distToPosition > 1.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool MotherShahrazRunAwayToBreakFatalAttractionAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> attractedPlayers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(SPELL_FATAL_ATTRACTION))
            attractedPlayers.push_back(member);
    }

    if (attractedPlayers.size() < 2)
        return false;

    float centerX = 0.0f, centerY = 0.0f;
    for (Player* member : attractedPlayers)
    {
        centerX += member->GetPositionX();
        centerY += member->GetPositionY();
    }
    centerX /= attractedPlayers.size();
    centerY /= attractedPlayers.size();

    std::sort(attractedPlayers.begin(), attractedPlayers.end(),
        [](Player* firstPlayer, Player* secondPlayer)
        {
            return firstPlayer->GetGUID().GetCounter() < secondPlayer->GetGUID().GetCounter();
        });

    auto botIt = std::find(attractedPlayers.begin(), attractedPlayers.end(), bot);
    if (botIt == attractedPlayers.end())
        return false;

    size_t botIndex = std::distance(attractedPlayers.begin(), botIt);
    float spreadAngle = 2.0f * M_PI * botIndex / attractedPlayers.size();

    float maxSpreadDistance = 35.0f;
    float distanceStep = 1.0f;
    float lastValidX = bot->GetPositionX();
    float lastValidY = bot->GetPositionY();
    float lastValidZ = bot->GetPositionZ();

    for (float currentDistance = distanceStep;
         currentDistance <= maxSpreadDistance;
         currentDistance += distanceStep)
    {
        float testX = centerX + std::cos(spreadAngle) * currentDistance;
        float testY = centerY + std::sin(spreadAngle) * currentDistance;
        float testZ = bot->GetPositionZ();

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), testX, testY, testZ))
        {
            break;
        }
        lastValidX = testX;
        lastValidY = testY;
        lastValidZ = testZ;
    }

    return MoveTo(BLACK_TEMPLE_MAP_ID, lastValidX, lastValidY, lastValidZ, false,
                  false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Illidari Council

bool IllidariCouncilMisdirectBossesToTanksAction::Execute(Event event)
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

        if (hunters.size() >= 4)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* councilTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
        tankTarget = GetZerevorMageTank(botAI, bot);
    }
    else if (hunterIndex == 1)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "lady malande");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && GET_PLAYERBOT_AI(member)->IsAssistTankOfIndex(member, 0, false))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 2)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && GET_PLAYERBOT_AI(member)->IsMainTank(member))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 3)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "veras darkshadow");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && GET_PLAYERBOT_AI(member)->IsAssistTankOfIndex(member, 1, false))
            {
                tankTarget = member;
                break;
            }
        }
    }

    if (!councilTarget || !tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", councilTarget))
        return botAI->CastSpell("steady shot", councilTarget);

    return false;
}

bool IllidariCouncilMainTankPositionGathiosAction::Execute(Event event)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return false;

    MarkTargetWithSquare(bot, gathios);
    SetRtiTarget(botAI, "square", gathios);

    if (bot->GetTarget() != gathios->GetGUID())
        return Attack(gathios);

    static const uint32 dangerousAuras[] = { SPELL_CONSECRATION, SPELL_BLIZZARD, SPELL_FLAMESTRIKE };
    bool hasDangerousAura = false;
    for (uint32 aura : dangerousAuras)
    {
        if (bot->HasAura(aura))
        {
            hasDangerousAura = true;
            break;
        }
    }

    const ObjectGuid guid = bot->GetGUID();
    uint8 index = gathiosTankStep.count(guid) ? gathiosTankStep[guid] : 0;

    const Position tankPositions[4] =
    {
        GATHIOS_TANK_POSITION_1,
        GATHIOS_TANK_POSITION_2,
        GATHIOS_TANK_POSITION_3,
        GATHIOS_TANK_POSITION_4
    };
    const Position& position = tankPositions[index];

    const float maxDistance = 2.0f;
    float distanceToTarget = bot->GetExactDist2d(position);

    if (gathios->GetVictim() == bot)
    {
        if (distanceToTarget <= maxDistance && hasDangerousAura)
        {
            index = (index + 1) % 4;
            gathiosTankStep[guid] = index;
            const Position& newPosition = tankPositions[index];
            float newDistanceToTarget = bot->GetExactDist2d(newPosition);
            if (newDistanceToTarget > maxDistance)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, newPosition.GetPositionX(), newPosition.GetPositionY(),
                              newPosition.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
        else if (distanceToTarget > maxDistance)
        {
            return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool IllidariCouncilMainTankReflectJudgementOfCommandAction::Execute(Event event)
{
    if (botAI->CanCastSpell("spell reflection", bot))
        return botAI->CastSpell("spell reflection", bot);

    return false;
}

bool IllidariCouncilFirstAssistTankPositionMalandeAction::Execute(Event event)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    MarkTargetWithStar(bot, malande);
    SetRtiTarget(botAI, "star", malande);

    if (bot->GetTarget() != malande->GetGUID())
        return Attack(malande);

    if (malande->GetVictim() == bot)
    {
        const Position& position = MALANDE_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 5.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool IllidariCouncilSecondAssistTankPositionDarkshadowAction::Execute(Event event)
{
    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (!darkshadow)
        return false;

    MarkTargetWithCircle(bot, darkshadow);
    SetRtiTarget(botAI, "circle", darkshadow);

    if (bot->GetTarget() != darkshadow->GetGUID())
        return Attack(darkshadow);

    if (darkshadow->GetVictim() == bot)
    {
        const Position& position = DARKSHADOW_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool IllidariCouncilMageTankPositionZerevorAction::Execute(Event event)
{
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (zerevor->HasAura(SPELL_DAMPEN_MAGIC) && botAI->CanCastSpell("spellsteal", zerevor))
        return botAI->CastSpell("spellsteal", zerevor);

    MarkTargetWithTriangle(bot, zerevor);
    SetRtiTarget(botAI, "triangle", zerevor);

    if (bot->GetTarget() != zerevor->GetGUID())
        return Attack(zerevor);

    if (zerevor->GetVictim() == bot)
    {
        const Position& position = ZEREVOR_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool IllidariCouncilAssignDpsTargetsAction::Execute(Event event)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    bool shouldAttackMalande = false;

    if (bot->getClass() == CLASS_ROGUE ||
        (bot->getClass() == CLASS_WARRIOR && botAI->IsDps(bot)))
    {
        if (!malande->HasAura(SPELL_BLESSING_OF_PROTECTION))
            shouldAttackMalande = true;
    }
    else if (bot->getClass() == CLASS_SHAMAN && botAI->IsDps(bot))
    {
        if (!malande->HasAura(SPELL_BLESSING_OF_SPELL_WARDING))
            shouldAttackMalande = true;
    }

    if (shouldAttackMalande)
    {
        SetRtiTarget(botAI, "star", malande);

        if (bot->GetTarget() != malande->GetGUID())
            return Attack(malande);
    }
    else
    {
        if (Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        {
            SetRtiTarget(botAI, "square", gathios);

            if (bot->GetTarget() != gathios->GetGUID())
                return Attack(gathios);
        }
    }

    return false;
}

bool IllidariCouncilManageDpsTimerAction::Execute(Event event)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (gathios->GetHealthPct() > 99.9f)
    {
        councilDpsWaitTimer.insert_or_assign(
            gathios->GetMap()->GetInstanceId(), std::time(nullptr));
            return true;
    }

    return false;
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageMisdirectToTankAction::Execute(Event event)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (GetIllidanPhase(illidan) == 2)
    {
        if (TryMisdirectToFlameTanks(group))
            return true;
    }
    else if (GetIllidanPhase(illidan) == 4)
    {
        if (TryMisdirectToWarlockTank(illidan, group))
            return true;
    }

    return false;
}

bool IllidanStormrageMisdirectToTankAction::TryMisdirectToFlameTanks(Group* group)
{
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);

        if (hunters.size() >= 2)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    auto [eastFlame, westFlame] = GetFlamesOfAzzinoth(botAI, bot);
    // If only one flame, do nothing
    if (!eastFlame || !westFlame || eastFlame == westFlame)
        return false;

    Player* firstAssistTank = nullptr;
    Player* secondAssistTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (!firstAssistTank && GET_PLAYERBOT_AI(member)->IsAssistTankOfIndex(member, 0, true))
            firstAssistTank = member;
        else if (!secondAssistTank && GET_PLAYERBOT_AI(member)->IsAssistTankOfIndex(member, 1, true))
            secondAssistTank = member;
    }

    if (!firstAssistTank || !secondAssistTank)
        return false;

    // If only one hunter, assign to second assist tank and east flame
    if (hunters.size() == 1)
    {
        if (eastFlame->GetHealthPct() < 90.0f)
            return false;

        if (botAI->CanCastSpell("misdirection", secondAssistTank))
            return botAI->CastSpell("misdirection", secondAssistTank);

        if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", eastFlame))
            return botAI->CastSpell("steady shot", eastFlame);

        return false;
    }

    // Standard case: two hunters, two tanks, two flames
    Player* tankTarget = nullptr;
    Unit* flame = nullptr;

    if (hunterIndex == 0)
    {
        if (eastFlame->GetHealthPct() < 90.0f)
            return false;
        tankTarget = secondAssistTank;
        flame = eastFlame;
    }
    else if (hunterIndex == 1)
    {
        if (westFlame->GetHealthPct() < 98.0f)
            return false;
        tankTarget = firstAssistTank;
        flame = westFlame;
    }
    else
        return false;

    if (!tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", flame))
        return botAI->CastSpell("steady shot", flame);

    return false;
}

bool IllidanStormrageMisdirectToTankAction::TryMisdirectToWarlockTank(Unit* illidan, Group* group)
{
    Player* warlockTank = GetIllidanWarlockTank(botAI, bot);
    if (!warlockTank)
        return false;

    if (botAI->CanCastSpell("misdirection", warlockTank))
        return botAI->CastSpell("misdirection", warlockTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", illidan))
        return botAI->CastSpell("steady shot", illidan);

    return false;
}

bool IllidanStormrageMainTankMoveAwayFromFlameCrashAction::Execute(Event event)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (bot->GetVictim() != illidan)
        return Attack(illidan);

    if (GetIllidanPhase(illidan) == 5)
    {
        auto const& gos = AI_VALUE(GuidVector, "nearest game objects");
        GameObject* nearestTrap = nullptr;
        float minDist = 30.0f; // Need to test what distance is worth it in terms of traps
        for (ObjectGuid const& guid : gos)
        {
            GameObject* go = botAI->GetGameObject(guid);
            if (!go || !go->isSpawned() || go->GetEntry() != GO_CAGE_TRAP)
                continue;
            float distToTrap = bot->GetExactDist2d(go);
            if (distToTrap < minDist)
            {
                minDist = distToTrap;
                nearestTrap = go;
            }
        }

        if (nearestTrap && illidan->GetVictim() == bot)
        {
            Position target = GetPointBeyondTrap(nearestTrap, 5.0f);
            return MoveTo(BLACK_TEMPLE_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, true);
        }
    }

    auto const& flameCrashes = GetAllFlameCrashes();
    if (flameCrashes.empty())
        return false;

    const float hazardRadius = 13.0f;
    bool inDanger = false;
    for (Unit* flameCrash : flameCrashes)
    {
        if (bot->GetDistance2d(flameCrash) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const float maxRadius = 30.0f;
    Position safestPos = FindSafestNearbyPosition(flameCrashes, maxRadius, hazardRadius);

    return MoveTo(BLACK_TEMPLE_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, true);

    return false;
}

Position IllidanStormrageMainTankMoveAwayFromFlameCrashAction::GetPointBeyondTrap(
    GameObject* nearestTrap, float extraDistance /*= 5.0f*/)
{
    if (!nearestTrap)
        return Position();

    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float trapX = nearestTrap->GetPositionX();
    float trapY = nearestTrap->GetPositionY();

    float distToTrap = nearestTrap->GetExactDist2d(bot);

    if (distToTrap == 0.0f)
        return Position(trapX, trapY, nearestTrap->GetPositionZ());

    // Normalize and extend beyond trap
    float dx = trapX - botX;
    float dy = trapY - botY;
    float targetX = trapX + (dx / distToTrap) * extraDistance;
    float targetY = trapY + (dy / distToTrap) * extraDistance;
    float targetZ = nearestTrap->GetPositionZ();

    return Position(targetX, targetY, targetZ);
}

Position IllidanStormrageMainTankMoveAwayFromFlameCrashAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& flameCrashes, float maxRadius, float hazardRadius)
{
    const float searchStep = M_PI / 16.0f; // finer angle steps
    const float minDistance = 2.0f;
    const float maxDistance = maxRadius;
    const float distanceStep = 1.0f;

    float backwardsAngle = Position::NormalizeOrientation(bot->GetOrientation() + M_PI);

    Position bestPos;
    float bestAngleDiff = M_PI * 2.0f;
    float bestDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return bestPos;

    for (float distance = minDistance; distance <= maxDistance; distance += distanceStep)
    {
        for (float angleOffset = 0.0f; angleOffset < 2 * M_PI; angleOffset += searchStep)
        {
            // Try both directions from straight backwards
            for (int sign = -1; sign <= 1; sign += 2)
            {
                float testAngle = Position::NormalizeOrientation(backwardsAngle + sign * angleOffset);
                float x = bot->GetPositionX() + distance * std::cos(testAngle);
                float y = bot->GetPositionY() + distance * std::sin(testAngle);

                Position testPos(x, y, bot->GetPositionZ());

                if (GetIllidanPhase(illidan) == 1)
                {
                    const Position& invalidPos = ILLIDAN_C_GRATE_POSITION;
                    const float invalidRadius = 20.0f;

                    if (testPos.GetExactDist2d(invalidPos.GetPositionX(), invalidPos.GetPositionY()) < invalidRadius)
                        continue;
                }

                bool isSafe = true;
                for (Unit* flameCrash : flameCrashes)
                {
                    if (flameCrash->GetDistance2d(x, y) < hazardRadius)
                    {
                        isSafe = false;
                        break;
                    }
                }
                if (!isSafe)
                    continue;

                bool pathSafe = IsPathSafeFromFlameCrashes(bot->GetPosition(), testPos, flameCrashes, hazardRadius);

                float angleDiff = std::abs(Position::NormalizeOrientation(testAngle - backwardsAngle));
                if (angleDiff > M_PI)
                    angleDiff = 2 * M_PI - angleDiff;

                // Prefer pathSafe, then smallest angle difference, then closest distance
                if (pathSafe && (!foundSafe || angleDiff < bestAngleDiff ||
                    (angleDiff == bestAngleDiff && distance < bestDistance)))
                {
                    bestPos = testPos;
                    bestAngleDiff = angleDiff;
                    bestDistance = distance;
                    foundSafe = true;
                }
                else if (!foundSafe && angleDiff < bestAngleDiff)
                {
                    bestPos = testPos;
                    bestAngleDiff = angleDiff;
                    bestDistance = distance;
                }
            }
            if (foundSafe)
                break;
        }
        if (foundSafe)
            break;
    }

    return bestPos;
}

bool IllidanStormrageMainTankMoveAwayFromFlameCrashAction::IsPathSafeFromFlameCrashes(const Position& start,
    const Position& end, const std::vector<Unit*>& flameCrashes, float hazardRadius)
{
    const uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* flameCrash : flameCrashes)
        {
            float distToFlameCrash = flameCrash->GetDistance2d(checkX, checkY);
            if (distToFlameCrash < hazardRadius)
                return false;
        }
    }

    return true;
}

std::vector<Unit*> IllidanStormrageMainTankMoveAwayFromFlameCrashAction::GetAllFlameCrashes()
{
    std::vector<Unit*> flameCrashes;
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        const float maxSearchRadius = 40.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_FLAME_CRASH &&
            bot->GetDistance2d(unit) < maxSearchRadius)
            flameCrashes.push_back(unit);
    }

    return flameCrashes;
}

bool IllidanStormrageIsolateBotWithParasiteAction::Execute(Event event)
{
    if (botAI->HasCheat(BotCheatMask::raid))
    {
        if (bot->HasAura(SPELL_PARASITIC_SHADOWFIEND))
            bot->RemoveAura(SPELL_PARASITIC_SHADOWFIEND);

        return false;
    }

    if (botAI->IsMainTank(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) == 2 || GetIllidanPhase(illidan) == 4)
        return false;

    Player* trapper = GetIllidanTrapperHunter(botAI, bot);

    if (GetIllidanPhase(illidan) == 1)
    {
        if (bot->HasAura(SPELL_PARASITIC_SHADOWFIEND))
        {
            Unit* nearestPlayer = nullptr;
            float distToNearest = 12.0f;

            for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || member == bot ||
                    member == trapper)
                    continue;

                float distance = bot->GetExactDist2d(member);
                if (distance < distToNearest)
                {
                    distToNearest = distance;
                    nearestPlayer = member;
                }
            }

            if (nearestPlayer)
            {
                const uint32 minInterval = 0;
                botAI->Reset();
                return FleePosition(Position(nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(),
                                             nearestPlayer->GetPositionZ()), 12.0f, minInterval);
            }
        }
    }

    if (GetIllidanPhase(illidan) == 1 || GetIllidanPhase(illidan) == 3 || GetIllidanPhase(illidan) == 5)
    {
        if (trapper && trapper == bot)
        {
            Player* parasiteTarget = HasParasiticShadowfiend(botAI, bot);
            if (!parasiteTarget)
                return false;

            Aura* parasiteAura = parasiteTarget->GetAura(SPELL_PARASITIC_SHADOWFIEND);

            if (parasiteAura && parasiteAura->GetDuration() < 6000)
            {
                float distToTarget = bot->GetExactDist2d(parasiteTarget);
                if (distToTarget > 5.0f && botAI->CanCastSpell("frost trap", bot))
                {
                    // Only move to the target if trap is ready
                    float dX = parasiteTarget->GetPositionX() - bot->GetPositionX();
                    float dY = parasiteTarget->GetPositionY() - bot->GetPositionY();
                    float moveDist = std::min(7.0f, distToTarget);
                    float moveX = bot->GetPositionX() + (dX / distToTarget) * moveDist;
                    float moveY = bot->GetPositionY() + (dY / distToTarget) * moveDist;

                    bot->AttackStop();
                    bot->InterruptNonMeleeSpells(true);
                    return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, parasiteTarget->GetPositionZ(), false, false,
                                  false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
                }
                else if (botAI->CanCastSpell("frost trap", bot))
                {
                    // Cast trap if close enough and trap is ready
                    return botAI->CastSpell("frost trap", bot);
                }
                else if (parasiteTarget != bot)
                {
                    // Trap is on cooldown, move away from the infected player
                    botAI->Reset();
                    return MoveAway(parasiteTarget, 12.0f);
                }
            }
        }
    }

    return false;
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::Execute(Event event)
{
    auto [eastFlame, westFlame] = GetFlamesOfAzzinoth(botAI, bot);

    // The second assist tank's flame is killed first; this is so that if the tank
    // for the second flame dies after the first flame is down, the dead flame's
    // tank will become the first assist tank and take over the remaining flame
    if (botAI->IsAssistTankOfIndex(bot, 1, true))
    {
        if (eastFlame && westFlame)
        {
            MarkTargetWithStar(bot, eastFlame);
            SetRtiTarget(botAI, "star", eastFlame);

            if (bot->GetVictim() != eastFlame)
                return Attack(eastFlame);

            if (!bot->IsWithinMeleeRange(eastFlame))
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, eastFlame->GetPositionX(), eastFlame->GetPositionY(),
                              eastFlame->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
        else if (!eastFlame && !westFlame)
        {
            const Position& pos = ILLIDAN_E_GLAIVE_WAITING_POSITION;
            if (bot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) > 0.5f)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        // After the first flame dies, its tank waits with all bots other than the second flame's tank
        else if (!eastFlame && westFlame)
        {
            const Position& pos = ILLIDAN_SW_GRATE_POSITION;
            if (bot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) > 0.5f)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        if (westFlame)
        {
            MarkTargetWithCircle(bot, westFlame);
            SetRtiTarget(botAI, "circle", westFlame);

            if (bot->GetVictim() != westFlame)
                return Attack(westFlame);

            if (!bot->IsWithinMeleeRange(westFlame))
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, westFlame->GetPositionX(), westFlame->GetPositionY(),
                              westFlame->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
        else
        {
            const Position& pos = ILLIDAN_W_GLAIVE_WAITING_POSITION;
            if (bot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) > 0.5f)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (RepositionToAvoidEyeBlast(illidan))
        return true;

    if (!eastFlame && !westFlame)
        return false;

    if (RepositionToAvoidBlaze(eastFlame, westFlame))
        return true;

    return false;
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidEyeBlast(Unit* illidan)
{
    EyeBlastDangerArea dangerArea = GetEyeBlastDangerArea(botAI, illidan);

    LOG_DEBUG("playerbots", "EyeBlast DangerArea width: {}", dangerArea.width);

    if (dangerArea.width == 0.0f)
    {
        LOG_DEBUG("playerbots", "EyeBlast DangerArea invalid, skipping avoidance.");
        return false;
    }

    LOG_DEBUG("playerbots", "EyeBlast Bot pos: ({}, {}) | Danger start: ({}, {}) end: ({}, {})",
        bot->GetPositionX(), bot->GetPositionY(),
        dangerArea.start.GetPositionX(), dangerArea.start.GetPositionY(),
        dangerArea.end.GetPositionX(), dangerArea.end.GetPositionY());

    bool inDanger = IsPositionInEyeBlastDangerArea(bot->GetPosition(), dangerArea);
    LOG_DEBUG("playerbots", "EyeBlast Is bot in Eye Blast danger area? {}", inDanger);

    if (inDanger)
    {
        // Find closest point on the line
        float dx = dangerArea.end.GetPositionX() - dangerArea.start.GetPositionX();
        float dy = dangerArea.end.GetPositionY() - dangerArea.start.GetPositionY();
        float length = std::sqrt(dx * dx + dy * dy);

        float px = bot->GetPositionX();
        float py = bot->GetPositionY();
        float sx = dangerArea.start.GetPositionX();
        float sy = dangerArea.start.GetPositionY();

        float projection = ((px - sx) * dx + (py - sy) * dy) / (length * length);
        projection = std::clamp(projection, 0.0f, 1.0f);

        float closestX = sx + projection * dx;
        float closestY = sy + projection * dy;

        float distToLine = bot->GetExactDist2d(closestX, closestY);
        float moveDist = (dangerArea.width - distToLine) + 0.5f;

        LOG_DEBUG("playerbots", "EyeBlast distToLine: {}, moveDist: {}", distToLine, moveDist);

        if (moveDist <= 0.0f)
        {
            LOG_DEBUG("playerbots", "EyeBlast Bot already outside danger area, no move needed.");
            return false;
        }

        // Move directly outward from the line
        float dirX = px - closestX;
        float dirY = py - closestY;
        float dirLength = std::sqrt(dirX * dirX + dirY * dirY);
        if (dirLength == 0.0f)
        {
            // If bot is exactly on the line, pick a default direction (perpendicular)
            dirX = -(dy / length);
            dirY = dx / length;
            dirLength = 1.0f;
        }

        float safeX = px + (dirX / dirLength) * moveDist;
        float safeY = py + (dirY / dirLength) * moveDist;
        float safeZ = bot->GetPositionZ();

        LOG_DEBUG("playerbots", "EyeBlast Moving bot to safe position: ({}, {}, {})", safeX, safeY, safeZ);

        return MoveTo(BLACK_TEMPLE_MAP_ID, safeX, safeY, safeZ,
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidBlaze(Unit* eastFlame, Unit* westFlame)
{
    const Position* waypoints = nullptr;
    size_t numWaypoints = 7;

    // Determine which tank and assign waypoints
    if (botAI->IsAssistTankOfIndex(bot, 1, true))
    {
        if (!eastFlame || eastFlame->GetVictim() != bot)
            return false;
        waypoints = ILLIDAN_E_GLAIVE_TANK_POSITIONS;
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        if (!westFlame || westFlame->GetVictim() != bot)
            return false;
        waypoints = ILLIDAN_W_GLAIVE_TANK_POSITIONS;
    }

    // Track current waypoint index for this bot using the shared map in the helpers namespace
    size_t& waypointIndex = flameTankWaypointIndex[bot->GetGUID()];

    // Check for nearby blaze
    auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_BLAZE && bot->GetDistance2d(unit) <= 9.0f)
        {
            waypointIndex = (waypointIndex + 1) % numWaypoints;
            break;
        }
    }

    // Move to current waypoint
    const Position& target = waypoints[waypointIndex];
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 0.2f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, true);
    }

    return false;
}

// Pets grab aggro right away during Phase 2 and wipe the raid if not put on passive
bool IllidanStormrageControlPetAggressionAction::Execute(Event event)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    if (GetIllidanPhase(illidan) == 2)
    {
        if (pet->GetReactState() != REACT_PASSIVE)
        {
            pet->AttackStop();
            pet->SetReactState(REACT_PASSIVE);
        }
    }
    else
    {
        if (pet->GetReactState() == REACT_PASSIVE)
            pet->SetReactState(REACT_DEFENSIVE);
    }

    return false;
}

bool IllidanStormrageBotsSpreadAboveGrateAction::Execute(Event event)
{
    const Position* gratePositions[2] =
    {
        &ILLIDAN_N_GRATE_POSITION,
        &ILLIDAN_S_GRATE_POSITION
    };

    std::vector<Player*> bots;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && !botAI->IsAssistTankOfIndex(member, 0, true) &&
            !botAI->IsAssistTankOfIndex(member, 1, true))
            bots.push_back(member);
    }

    if (bots.empty())
        return false;

    // Sort for deterministic assignment
    std::sort(bots.begin(), bots.end(),
        [](Player* firstPlayer, Player* secondPlayer) {
            return firstPlayer->GetGUID() < secondPlayer->GetGUID(); });

    auto it = std::find(bots.begin(), bots.end(), bot);
    if (it == bots.end())
        return false;

    size_t botIndex = std::distance(bots.begin(), it);
    size_t groupIndex = botIndex % 2; // Assign to N, S in round-robin

    // If bot has blaze aura and is at its assigned position, move clockwise
    if (bot->HasAura(SPELL_BLAZE))
    {
        const Position& currentPos = *gratePositions[groupIndex];
        if (bot->GetExactDist2d(currentPos.GetPositionX(), currentPos.GetPositionY()) <= 0.2f)
        {
            groupIndex = (groupIndex + 1) % 2; // Move N->S, S->N
        }
    }

    const Position& position = *gratePositions[groupIndex];
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 0.2f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool IllidanStormrageRemoveDarkBarrageAction::Execute(Event event)
{
    botAI->Reset();

    const char* abilities[] = { "divine shield", "ice block", "cloak of shadows" };
    for (const char* spellName : abilities)
    {
        if (botAI->CanCastSpell(spellName, bot))
            return botAI->CastSpell(spellName, bot);
    }

    return false;
}

bool IllidanStormrageDisperseRangedAction::Execute(Event event)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    const uint32 minInterval = 1000;

    // In Phase 4
    Player* warlockTank = GetIllidanWarlockTank(botAI, bot);
    if (GetIllidanPhase(illidan) == 4)
    {
        // Flee from warlock tank if within 21 yards
        if (warlockTank && warlockTank != bot)
        {
            if (bot->GetDistance2d(warlockTank) < 21.0f)
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return FleePosition(Position(warlockTank->GetPositionX(), warlockTank->GetPositionY(),
                                             warlockTank->GetPositionZ()), 21.0f, minInterval);
            }
        }
        // Flee from Illidan if within 16 yards
        else if (bot->GetDistance2d(illidan) < 16.0f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return FleePosition(Position(illidan->GetPositionX(), illidan->GetPositionY(),
                                         illidan->GetPositionZ()), 16.0f, minInterval);
        }
        else if (warlockTank && warlockTank == bot)
            return false;
    }

    // Excluding warlock tank during phase 4, flee if within 6 yards of any other player
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, 6.0f))
    {
        return FleePosition(Position(nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(),
                                     nearestPlayer->GetPositionZ()), 6.0f, minInterval);
    }

    return false;
}

bool IllidanStormragePositionMeleeAction::Execute(Event event)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (GetIllidanPhase(illidan) == 4)
    {
        if (StayAwayFromDemonBoss(illidan))
            return true;
    }
    else if (GetIllidanPhase(illidan) != 2)
    {
        if (botAI->IsMainTank(bot))
            return false;

        if (PositionBehindBoss(illidan))
            return true;
    }

    return false;
}

bool IllidanStormragePositionMeleeAction::StayAwayFromDemonBoss(Unit* illidan)
{
    float currentDistance = bot->GetDistance2d(illidan);
    float safeDistance = 30.0f;

    if (currentDistance < safeDistance)
    {
        botAI->Reset();
        return MoveAway(illidan, safeDistance - currentDistance);
    }

    return false;
}

bool IllidanStormragePositionMeleeAction::PositionBehindBoss(Unit* illidan)
{
    if (illidan->GetVictim() == bot)
        return false;

    const float behindAngle = M_PI;
    const float angleOffset = M_PI / 2.25f; // 80 degrees

    std::vector<Player*> melee;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMelee(member) && !botAI->IsMainTank(member))
            melee.push_back(member);
    }

    if (melee.empty())
        return false;

    // Sort for deterministic assignment
    std::sort(melee.begin(), melee.end(),
        [](Player* firstPlayer, Player* secondPlayer) {
            return firstPlayer->GetGUID() < secondPlayer->GetGUID(); });

    auto it = std::find(melee.begin(), melee.end(), bot);
    if (it == melee.end())
        return false;

    size_t botIndex = std::distance(melee.begin(), it);
    size_t groupIndex = botIndex % 2; // 0: left, 1: right

    float assignedAngle;
    if (groupIndex == 0)
        assignedAngle = Position::NormalizeOrientation(illidan->GetOrientation() + M_PI + angleOffset); // left
    else if (groupIndex == 1)
        assignedAngle = Position::NormalizeOrientation(illidan->GetOrientation() + M_PI - angleOffset); // right

    const float desiredDist = 3.5f;
    const float tolerance = 0.2f;

    float targetX = illidan->GetPositionX() + desiredDist * std::cos(assignedAngle);
    float targetY = illidan->GetPositionY() + desiredDist * std::sin(assignedAngle);

    if (fabs(bot->GetExactDist2d(targetX, targetY) - desiredDist) > tolerance)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, illidan->GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IllidanStormrageWarlockTankHandleDemonBossAction::Execute(Event event)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (GetIllidanPhase(illidan) == 4)
    {
        if (!botAI->HasStrategy("tank", BotState::BOT_STATE_COMBAT))
            botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);

        MarkTargetWithDiamond(bot, illidan);
        SetRtiTarget(botAI, "diamond", illidan);

        if (bot->GetTarget() != illidan->GetGUID())
            return Attack(illidan);

        if (bot->GetDistance2d(illidan) < 16.0f)
        {
            uint32 minInterval = 0;
            return FleePosition(Position(illidan->GetPositionX(), illidan->GetPositionY(),
                                         illidan->GetPositionZ()), 16.0f, 0);
        }
    }
    else
    {
        if (botAI->HasStrategy("tank", BotState::BOT_STATE_COMBAT))
            botAI->ChangeStrategy("-tank", BotState::BOT_STATE_COMBAT);
    }

    return false;
}

bool IllidanStormrageDpsPrioritizeAddsAction::Execute(Event event)
{
    auto [eastFlame, westFlame] = GetFlamesOfAzzinoth(botAI, bot);

    if (Unit* shadowDemon = GetFirstAliveUnitByEntry(botAI, NPC_SHADOW_DEMON))
    {
        if (IsInstanceTimerManager(botAI, bot))
            MarkTargetWithSquare(bot, shadowDemon);

        SetRtiTarget(botAI, "square", shadowDemon);

        if (bot->GetTarget() != shadowDemon->GetGUID())
            return Attack(shadowDemon);
    }
    else if (Unit* shadowfiend = AI_VALUE2(Unit*, "find target", "parasitic shadowfiend"))
    {
        if (bot->getClass() == CLASS_MAGE && bot->GetExactDist2d(shadowfiend) < 10.0f)
        {
            if (botAI->CanCastSpell("frost nova", bot))
                return botAI->CastSpell("frost nova", bot);
        }

        if (IsInstanceTimerManager(botAI, bot))
            MarkTargetWithTriangle(bot, shadowfiend);

        if (botAI->IsRanged(bot))
        {
            SetRtiTarget(botAI, "triangle", shadowfiend);

            if (bot->GetTarget() != shadowfiend->GetGUID())
                return Attack(shadowfiend);
        }
    }
    else if (eastFlame)
    {
        SetRtiTarget(botAI, "star", eastFlame);

        if (bot->GetTarget() != eastFlame->GetGUID())
            return Attack(eastFlame);
    }
    else if (westFlame)
    {
        SetRtiTarget(botAI, "circle", westFlame);

        if (bot->GetTarget() != westFlame->GetGUID())
            return Attack(westFlame);
    }

    return false;
}

bool IllidanStormrageManageDpsTimerAction::Execute(Event event)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    const time_t now = std::time(nullptr);
    const uint32 instanceId = illidan->GetMap()->GetInstanceId();

    if (GetIllidanPhase(illidan) == 3 || GetIllidanPhase(illidan) == 5)
    {
        if (illidanDpsWaitTimer.erase(instanceId))
            return true;
    }
    else if (GetIllidanPhase(illidan) == 1 || GetIllidanPhase(illidan) == 4)
    {
        if (illidanDpsWaitTimer.try_emplace(instanceId, now).second)
            return true;
    }
    else if (GetIllidanPhase(illidan) == 2 &&
             AI_VALUE2(Unit*, "find target", "flame of azzinoth"))
    {
        if (illidanDpsWaitTimer.try_emplace(instanceId, now).second)
            return true;
    }

    return false;
}

bool IllidanStormrageDestroyHazardsCheatAction::Execute(Event event)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    bool destroyed = false;
    if (Unit* shadowfiend = AI_VALUE2(Unit*, "find target", "parasitic shadowfiend"))
    {
        shadowfiend->Kill(bot, shadowfiend);
        destroyed = true;
    }
    if (GetIllidanPhase(illidan) == 2 || GetIllidanPhase(illidan) == 4)
    {
        auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == NPC_FLAME_CRASH)
            {
                unit->Kill(bot, unit);
                destroyed = true;
            }
        }
    }
    else
    {
        Unit* flame = AI_VALUE2(Unit*, "find target", "flame of azzinoth");
        if (GetIllidanPhase(illidan) == 0 ||
            GetIllidanPhase(illidan) == 3 ||
            (GetIllidanPhase(illidan) == 2 && flame == nullptr))
        {
            auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
            for (auto const& guid : npcs)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (unit && (unit->GetEntry() == NPC_DEMON_FIRE || unit->GetEntry() == NPC_BLAZE))
                {
                    unit->Kill(bot, unit);
                    destroyed = true;
                }
            }
        }
    }

    return destroyed;
}
