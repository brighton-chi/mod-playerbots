#include "RaidBlackTempleActions.h"
#include "RaidBlackTempleHelpers.h"
#include "RaidBlackTempleIllidanBossAI.h"
#include "AiFactory.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace BlackTempleHelpers;

// General

bool BlackTempleEraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    const ObjectGuid guid = bot->GetGUID();
    const uint32 instanceId = bot->GetMap()->GetInstanceId();

    bool erased = false;
    if (!AI_VALUE2(Unit*, "find target", "supremus") &&
        supremusPhaseTimer.erase(instanceId) > 0)
    {
        erased = true;
    }
    if (!AI_VALUE2(Unit*, "find target", "gurtogg bloodboil") &&
        gurtoggPhaseTimer.erase(instanceId) > 0)
    {
        erased = true;
    }
    if (!AI_VALUE2(Unit*, "find target", "mother shahraz") &&
        shahrazTankStep.erase(guid) > 0)
    {
        erased = true;
    }
    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
    {
        if (councilDpsWaitTimer.erase(instanceId) > 0)
            erased = true;
        if (gathiosTankStep.erase(guid) > 0)
            erased = true;
        if (zerevorHealStep.erase(guid) > 0)
            erased = true;
    }
    if (!AI_VALUE2(Unit*, "find target", "illidan stormrage") &&
        !AI_VALUE2(Unit*, "find target", "flames of azzinoth"))
    {
        if (illidanBossDpsWaitTimer.erase(instanceId) > 0)
            erased = true;
        if (westFlameGuid.erase(instanceId) > 0)
            erased = true;
        if (eastFlameGuid.erase(instanceId) > 0)
            erased = true;
        if (flameTankWaypointIndex.erase(guid) > 0)
            erased = true;
    }

    return erased;
}

// High Warlord Naj'entus

bool HighWarlordNajentusMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", najentus))
        return botAI->CastSpell("steady shot", najentus);

    return false;
}

bool HighWarlordNajentusTanksPositionBossAction::Execute(Event /*event*/)
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

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                   false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool HighWarlordNajentusDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    constexpr uint32 minInterval = 1000;

    constexpr float safeDistFromBoss = 10.0f;
    if (bot->GetExactDist2d(najentus) < safeDistFromBoss &&
        FleePosition(najentus->GetPosition(), safeDistFromBoss, minInterval))
        return true;

    constexpr float safeDistFromPlayer = 7.0f;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

    return false;
}

bool HighWarlordNajentusRemoveImpalingSpineAction::Execute(Event /*event*/)
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
    auto gos = AI_VALUE(GuidVector, "nearest game objects");
    GameObject* spineGo = nullptr;
    for (ObjectGuid const& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (go && go->isSpawned() && go->GetEntry() == GO_NAJENTUS_SPINE)
        {
            spineGo = go;
            break;
        }
    }
    if (!spineGo)
        return false;

    // 3. Move to the spine if not close enough
    if (bot->GetExactDist2d(spineGo) > 3.0f)
    {
        uint32 delay = urand(2000, 3000); // 1 to 2 seconds
        botAI->AddTimedEvent(
            [this, spineGo]() {
                if (spineGo && bot)
                    MoveTo(BLACK_TEMPLE_MAP_ID, spineGo->GetPositionX(), spineGo->GetPositionY(),
                           bot->GetPositionZ(), false, false, false, false,
                           MovementPriority::MOVEMENT_FORCED, true, false);
            },
            delay);

        botAI->SetNextCheckDelay(delay + 50);
        return true;
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

bool HighWarlordNajentusThrowImpalingSpineAction::Execute(Event /*event*/)
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

        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(),
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

bool SupremusMisdirectBossToMainTankAction::Execute(Event /*event*/)
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

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* firstAssistTank = GetGroupFirstAssistTank(botAI, bot);
    Player* secondAssistTank = GetGroupSecondAssistTank(botAI, bot);

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

bool SupremusDisperseRangedAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 8.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);

    return false;
}

bool SupremusKiteBossAction::Execute(Event /*event*/)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    float currentDistance = bot->GetDistance2d(supremus);
    constexpr float safeDistance = 25.0f;
    if (currentDistance < safeDistance)
        return MoveAway(supremus, safeDistance - currentDistance);

    return false;
}

bool SupremusMoveAwayFromVolcanosAction::Execute(Event /*event*/)
{
    auto const& volcanos = GetAllSupremusVolcanos();
    if (volcanos.empty())
        return false;

    constexpr float hazardRadius = 14.0f;
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

    constexpr float maxRadius = 40.0f;
    Position safestPos = FindSafestNearbyPosition(volcanos, maxRadius, hazardRadius);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(BLACK_TEMPLE_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  bot->GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

Position SupremusMoveAwayFromVolcanosAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& volcanos, float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float minDistance = 2.0f;
    constexpr float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float distance = minDistance;
         distance <= maxRadius; distance += distanceStep)
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
    constexpr uint8 numChecks = 10;
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
    constexpr float searchRadius = 40.0f;

    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(creatureList, NPC_SUPREMUS_VOLCANO, searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            volcanos.push_back(creature);
    }

    return volcanos;
}

bool SupremusManagePhaseTimerAction::Execute(Event /*event*/)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    supremusPhaseTimer.try_emplace(
        supremus->GetMap()->GetInstanceId(), std::time(nullptr));

    return false;
}

// Teron Gorefiend

bool TeronGorefiendMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", gorefiend))
        return botAI->CastSpell("steady shot", gorefiend);

    return false;
}

bool TeronGorefiendTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    MarkTargetWithSkull(bot, gorefiend);

    if (bot->GetVictim() != gorefiend)
        return Attack(gorefiend);

    if (gorefiend->GetVictim() == bot && bot->IsWithinMeleeRange(gorefiend))
    {
        const Position& position = GOREFIEND_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                   position.GetPositionY());
        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Assume positions in arc at the edge of the balcony (farthest from Constructs)
bool TeronGorefiendPositionRangedOnBalconyAction::Execute(Event /*event*/)
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
        if (!member || !botAI->IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    size_t count = rangedMembers.size();
    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t botIndex = (findIt != rangedMembers.end()) ?
        std::distance(rangedMembers.begin(), findIt) : 0;

    constexpr float arcSpan = 2.0f * M_PI / 5.0f;
    constexpr float arcCenter = 6.279f;
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    constexpr float radius = 12.0f;
    float angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float targetX = GOREFIEND_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
    float targetY = GOREFIEND_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

    float distToPosition = bot->GetExactDist2d(targetX, targetY);
    if (distToPosition > 2.0f)
    {
        float dX = targetX - bot->GetPositionX();
        float dY = targetY - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TeronGorefiendAvoidShadowOfDeathAction::Execute(Event /*event*/)
{
    botAI->Reset();

    static const std::array<const char*, 4> abilities =
    {
        "divine shield", "feign death", "ice block", "vanish"
    };

    for (const char* spellName : abilities)
    {
        if (botAI->CanCastSpell(spellName, bot))
            return botAI->CastSpell(spellName, bot);
    }

    return false;
}

bool TeronGorefiendMoveToCornerToDieAction::Execute(Event /*event*/)
{
    const Position& position = GOREFIEND_DIE_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      bot->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TeronGorefiendControlAndDestroyShadowyConstructsAction::Execute(Event /*event*/)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    Unit* spirit = bot->GetCharm();
    if (!spirit)
        return false;

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    Unit* priorityTarget = nullptr;
    uint32 highestHp = std::numeric_limits<uint32>::min();

    /* for (auto guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_SHADOWY_CONSTRUCT)
            continue;

        uint32 hp = unit->GetHealth();
        if (hp > highestHp)
        {
            highestHp = hp;
            priorityTarget = unit;
        }
    } */
    float closestToGorefiend = std::numeric_limits<float>::max();

    for (auto guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_SHADOWY_CONSTRUCT)
            continue;

        uint32 hp = unit->GetHealth();
        float distToGorefiend = gorefiend->GetExactDist2d(unit);

        if (hp > highestHp)
        {
            highestHp = hp;
            priorityTarget = unit;
            closestToGorefiend = distToGorefiend;
        }
        else if ((hp == highestHp) && (distToGorefiend < closestToGorefiend))
        {
            priorityTarget = unit;
            closestToGorefiend = distToGorefiend;
        }
    }

    if (priorityTarget)
    {
        float distToTarget = spirit->GetExactDist2d(priorityTarget);
        float desiredDist = 10.0f;
        if (distToTarget > desiredDist)
        {
            float moveDist = distToTarget - desiredDist + 2.0f;
            float dX = priorityTarget->GetPositionX() - spirit->GetPositionX();
            float dY = priorityTarget->GetPositionY() - spirit->GetPositionY();
            float moveX = spirit->GetPositionX() + (dX / distToTarget) * moveDist;
            float moveY = spirit->GetPositionY() + (dY / distToTarget) * moveDist;

            spirit->GetMotionMaster()->MovePoint(
                0, moveX, moveY, spirit->GetPositionZ());
            return true;
        }

        // Due to the charmed creature not observing GCDs, this sequence is intended
        // to try to create an authentic rotation (having a sequence with just one of
        // each ability results in Chains breaking immediately upon cast, even if
        // Volley is scheduled to occur before Chains (maybe due to projectile travel time?)
        if (!spirit->HasSpellCooldown(SPELL_SPIRIT_CHAINS) &&
            priorityTarget->GetHealthPct() == 100.0f)
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
        else if (!spirit->HasSpellCooldown(SPELL_SPIRIT_VOLLEY) &&
                 !priorityTarget->HasAura(SPELL_SPIRIT_CHAINS))
        {
            spirit->CastSpell(priorityTarget, SPELL_SPIRIT_VOLLEY, true);
            spirit->AddSpellCooldown(SPELL_SPIRIT_VOLLEY, 0, 15000);
            return true;
        }
    }
    else
    {
        float distToGorefiend = spirit->GetExactDist2d(gorefiend);
        float targetDist = 5.0f;
        if (distToGorefiend > targetDist)
        {
            float moveDist = distToGorefiend - targetDist;
            float dX = gorefiend->GetPositionX() - spirit->GetPositionX();
            float dY = gorefiend->GetPositionY() - spirit->GetPositionY();
            float moveX = spirit->GetPositionX() + (dX / distToGorefiend) * moveDist;
            float moveY = spirit->GetPositionY() + (dY / distToGorefiend) * moveDist;

            spirit->GetMotionMaster()->MovePoint(0, moveX, moveY, spirit->GetPositionZ());
            return true;
        }
        else if (!spirit->HasSpellCooldown(SPELL_SPIRIT_STRIKE))
        {
            spirit->CastSpell(gorefiend, SPELL_SPIRIT_STRIKE, true);
            spirit->AddSpellCooldown(SPELL_SPIRIT_STRIKE, 0, 1000);
            return true;
        }
    }

    return false;
}

// Gurtogg Bloodboil

bool GurtoggBloodboilMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", gurtogg))
        return botAI->CastSpell("steady shot", gurtogg);

    return false;
}

bool GurtoggBloodboilTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    if (bot->GetVictim() != gurtogg)
        return Attack(gurtogg);

    Unit* victim = gurtogg->GetVictim();
    Player* playerVictim = victim ? victim->ToPlayer() : nullptr;
    if (playerVictim && botAI->IsTank(playerVictim) && bot->IsWithinMeleeRange(gurtogg))
    {
        const Position& position = GURTOGG_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                   position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool GurtoggBloodboilRotateRangedGroupsAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    auto groups = GetGurtoggRangedRotationGroups(bot);
    int activeGroup = GetGurtoggActiveRotationGroup(gurtogg);

    bool inActiveGroup = false;
    if (activeGroup >= 0 && activeGroup < groups.size())
    {
        const auto& group = groups[activeGroup];
        inActiveGroup = std::find(group.begin(), group.end(), bot) != group.end();
    }

    const Position& nearPosition = GURTOGG_RANGED_POSITION;
    const Position& farPosition = GURTOGG_SOAKER_POSITION;
    constexpr float distFromPos = 2.0f;

    if (inActiveGroup && bot->GetExactDist2d(farPosition) > distFromPos)
    {
        return MoveInside(BLACK_TEMPLE_MAP_ID, farPosition.GetPositionX(),
                          farPosition.GetPositionY(), bot->GetPositionZ(),
                          distFromPos, MovementPriority::MOVEMENT_FORCED);
    }
    else if (!inActiveGroup && bot->GetExactDist2d(nearPosition) > distFromPos)
    {
        return MoveInside(BLACK_TEMPLE_MAP_ID, nearPosition.GetPositionX(),
                          nearPosition.GetPositionY(), bot->GetPositionZ(),
                          distFromPos, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction::Execute(Event /*event*/)
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

    constexpr float safeDistance = 20.0f;
    constexpr uint32 minInterval = 0;
    if (enragedPlayer && bot->GetExactDist2d(enragedPlayer) < safeDistance)
        return FleePosition(enragedPlayer->GetPosition(), safeDistance, minInterval);

    return false;
}

bool GurtoggBloodboilManagePhaseTimerAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    const time_t now = std::time(nullptr);
    const uint32 instanceId = gurtogg->GetMap()->GetInstanceId();

    if (gurtogg->HasAura(SPELL_BOSS_FEL_RAGE))
    {
        return gurtoggPhaseTimer.erase(instanceId) > 0;
    }
    else
    {
        auto [it, inserted] = gurtoggPhaseTimer.try_emplace(instanceId, now);
        return inserted;
    }
}

// Reliquary of Souls

bool ReliquaryOfSoulsMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
    if (!desire && !anger)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
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

bool ReliquaryOfSoulsHealersDpsEssenceOfSufferingAction::Execute(Event /*event*/)
{
    if (AI_VALUE2(Unit*, "find target", "essence of suffering"))
    {
        if (!botAI->HasStrategy("healer dps", BotState::BOT_STATE_COMBAT))
        {
            botAI->ChangeStrategy("+healer dps", BotState::BOT_STATE_COMBAT);
            return true;
        }
    }
    else
    {
        if (botAI->HasStrategy("healer dps", BotState::BOT_STATE_COMBAT))
        {
            botAI->ChangeStrategy("-healer dps", BotState::BOT_STATE_COMBAT);
            return true;
        }
    }

    return false;
}

bool ReliquaryOfSoulsAdjustDistanceFromEssenceOfSufferingAction::Execute(Event /*event*/)
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering)
        return false;

    if (botAI->IsTank(bot))
        return TanksMoveToMinimumRange(suffering);
    else if (botAI->IsMelee(bot) && bot->GetVictim() != suffering)
        return MeleeDpsStayAtMaximumRange(suffering);
    else if (botAI->IsRanged(bot))
        return RangedMoveAwayFromBoss(suffering);

    return false;
}

bool ReliquaryOfSoulsAdjustDistanceFromEssenceOfSufferingAction::TanksMoveToMinimumRange(Unit* suffering)
{
    float distanceToBoss = bot->GetExactDist2d(suffering);
    if (distanceToBoss > 2.0f)
    {
        float dX = suffering->GetPositionX() - bot->GetPositionX();
        float dY = suffering->GetPositionY() - bot->GetPositionY();
        float targetX = bot->GetPositionX() + (dX / distanceToBoss);
        float targetY = bot->GetPositionY() + (dY / distanceToBoss);

        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool ReliquaryOfSoulsAdjustDistanceFromEssenceOfSufferingAction::MeleeDpsStayAtMaximumRange(Unit* suffering)
{
    float desiredDist = bot->GetMeleeRange(suffering);
    float behindAngle = Position::NormalizeOrientation(suffering->GetOrientation() + M_PI);
    float targetX = suffering->GetPositionX() + desiredDist * std::cos(behindAngle);
    float targetY = suffering->GetPositionY() + desiredDist * std::sin(behindAngle);

    if (bot->GetExactDist2d(targetX, targetY) > 0.25f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool ReliquaryOfSoulsAdjustDistanceFromEssenceOfSufferingAction::RangedMoveAwayFromBoss(Unit* suffering)
{
    constexpr float safeDistance = 15.0f;
    constexpr uint32 minInterval = 1000;
    if (bot->GetExactDist2d(suffering) < safeDistance)
        return FleePosition(suffering->GetPosition(), safeDistance, minInterval);

    return false;
}

bool ReliquaryOfSoulsSpellstealRuneShieldAction::Execute(Event /*event*/)
{
    if (Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire"))
    {
        if (botAI->CanCastSpell("spellsteal", desire))
            return botAI->CastSpell("spellsteal", desire);
    }

    return false;
}

bool ReliquaryOfSoulsSpellReflectDeadenAction::Execute(Event /*event*/)
{
    if (botAI->CanCastSpell("spell reflection", bot))
        return botAI->CastSpell("spell reflection", bot);

    return false;
}

// Mother Shahraz

bool MotherShahrazMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", shahraz))
        return botAI->CastSpell("steady shot", shahraz);

    return false;
}

bool MotherShahrazTanksPositionBossUnderPillarAction::Execute(Event /*event*/)
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

        const Position positions[2] =
        {
            SHAHRAZ_TRANSITION_POSITION,
            SHAHRAZ_TANK_POSITION
        };
        constexpr float maxDistance = 0.5f;
        const Position& position = positions[step];
        float distToPosition = bot->GetExactDist2d(position);

        if ((distToPosition > maxDistance) && bot->IsWithinMeleeRange(shahraz))
        {
            bool backwards = (shahraz->GetVictim() == bot);
            return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, backwards);
        }

        if (step == 0 && distToPosition <= maxDistance)
            shahrazTankStep[guid] = 1;

        if (step == 1 && distToPosition <= maxDistance)
        {
            float orientation = atan2(shahraz->GetPositionY() - bot->GetPositionY(),
                                      shahraz->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
    }

    return false;
}

// This doesn't actually matter for bots since they don't take fall damage
// But I still want to simulate actual player behavior
bool MotherShahrazPositionRangedUnderPillarAction::Execute(Event /*event*/)
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

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool MotherShahrazRunAwayToBreakFatalAttractionAction::Execute(Event /*event*/)
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
        [](Player* a, Player* b) { return a->GetGUID().GetCounter() < b->GetGUID().GetCounter(); });

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

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), testX, testY, testZ))
            break;

        lastValidX = testX;
        lastValidY = testY;
        lastValidZ = testZ;
    }

    if (MoveTo(BLACK_TEMPLE_MAP_ID, lastValidX, lastValidY, lastValidZ, false, false,
               false, true, MovementPriority::MOVEMENT_FORCED, true, false))
    {
        return true;
    }
    else
    {
        // Failsafe: try a 5-yard random move if main MoveTo fails
        float angle = frand(0.0f, 2.0f * M_PI);
        constexpr float dist = 5.0f;
        float randX = bot->GetPositionX() + std::cos(angle) * dist;
        float randY = bot->GetPositionY() + std::sin(angle) * dist;
        float randZ = bot->GetPositionZ();
        bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), randX, randY, randZ);

        return MoveTo(BLACK_TEMPLE_MAP_ID, randX, randY, randZ, false, false, false, true,
                    MovementPriority::MOVEMENT_FORCED, true, false);
    }
}

// Illidari Council

bool IllidariCouncilMisdirectBossesToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
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
        tankTarget = GetZerevorMageTank(bot);
    }
    else if (hunterIndex == 1)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "lady malande");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetGroupFirstAssistTank(botAI, bot))
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
            if (Player* member = GetGroupMainTank(botAI, bot))
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
            if (Player* member = GetGroupSecondAssistTank(botAI, bot))
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

bool IllidariCouncilMainTankPositionGathiosAction::Execute(Event /*event*/)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return false;

    // Failsafe for if bot falls through the floor, which tends to happen upon the pull
    if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
    {
        bot->TeleportTo(BLACK_TEMPLE_MAP_ID, gathios->GetPositionX(), gathios->GetPositionY(),
                        gathios->GetPositionZ(), bot->GetOrientation());
        return true;
    }

    MarkTargetWithSquare(bot, gathios);
    SetRtiTarget(botAI, "square", gathios);

    if (bot->GetTarget() != gathios->GetGUID())
        return Attack(gathios);

    const ObjectGuid guid = bot->GetGUID();
    uint8 index = gathiosTankStep.count(guid) ? gathiosTankStep[guid] : 0;

    const Position& position = GATHIOS_TANK_POSITIONS[index];

    constexpr float maxDistance = 2.0f;
    float distToPosition = bot->GetExactDist2d(position);

    if (gathios->GetVictim() == bot && bot->IsWithinMeleeRange(gathios))
    {
        if (distToPosition <= maxDistance && HasDangerousCouncilAura(bot))
        {
            index = (index + 1) % 4;
            gathiosTankStep[guid] = index;
            const Position& newPosition = GATHIOS_TANK_POSITIONS[index];
            float newDistToPosition = bot->GetExactDist2d(newPosition);
            if (newDistToPosition > maxDistance)
            {
                float dX = newPosition.GetPositionX() - bot->GetPositionX();
                float dY = newPosition.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, newDistToPosition);
                float moveX = bot->GetPositionX() + (dX / newDistToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / newDistToPosition) * moveDist;

                return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(),
                              false, false, false, true, MovementPriority::MOVEMENT_COMBAT,
                              true, true);
            }
        }
        else if (distToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_COMBAT,
                          true, true);
        }
    }

    return false;
}

bool IllidariCouncilMainTankReflectJudgementOfCommandAction::Execute(Event /*event*/)
{
    if (botAI->CanCastSpell("spell reflection", bot))
        return botAI->CastSpell("spell reflection", bot);

    return false;
}

bool IllidariCouncilFirstAssistTankPositionMalandeAction::Execute(Event /*event*/)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    // Failsafe for if bot falls through the floor, which tends to happen upon the pull
    if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
    {
        bot->TeleportTo(BLACK_TEMPLE_MAP_ID, malande->GetPositionX(), malande->GetPositionY(),
                        malande->GetPositionZ(), bot->GetOrientation());
        return true;
    }

    MarkTargetWithStar(bot, malande);
    SetRtiTarget(botAI, "star", malande);

    if (bot->GetTarget() != malande->GetGUID())
        return Attack(malande);

    if (malande->GetVictim() == bot)
    {
        const Position& position = MALANDE_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 10.0f)
        {
            return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          bot->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool IllidariCouncilSecondAssistTankPositionDarkshadowAction::Execute(Event /*event*/)
{
    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (!darkshadow)
        return false;

    // Failsafe for if bot falls through the floor, which tends to happen upon the pull
    if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
    {
        bot->TeleportTo(BLACK_TEMPLE_MAP_ID, darkshadow->GetPositionX(), darkshadow->GetPositionY(),
                        darkshadow->GetPositionZ(), bot->GetOrientation());
        return true;
    }

    MarkTargetWithCircle(bot, darkshadow);
    SetRtiTarget(botAI, "circle", darkshadow);

    if (bot->GetTarget() != darkshadow->GetGUID())
        return Attack(darkshadow);

    if (darkshadow->GetVictim() == bot)
    {
        Player* mainTank = GetGroupMainTank(botAI, bot);
        if (!mainTank)
            return false;

        float distToPosition =
            bot->GetExactDist2d(mainTank->GetPositionX(), mainTank->GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = mainTank->GetPositionX() - bot->GetPositionX();
            float dY = mainTank->GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            bool backwards = bot->GetExactDist2d(mainTank) < 10.0f;
            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT, true, backwards);
        }
    }

    return false;
}

bool IllidariCouncilMageTankPositionZerevorAction::Execute(Event /*event*/)
{
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor)
        return false;

    if (zerevor->HasAura(SPELL_DAMPEN_MAGIC) && botAI->CanCastSpell("spellsteal", zerevor))
        return botAI->CastSpell("spellsteal", zerevor);

    MarkTargetWithTriangle(bot, zerevor);
    SetRtiTarget(botAI, "triangle", zerevor);

    if (bot->GetTarget() != zerevor->GetGUID())
        return Attack(zerevor);

    if (zerevor->GetVictim() == bot)
    {
        const Position& position = ZEREVOR_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                          true, false);
        }
    }

    return false;
}

bool IllidariCouncilPositionMageTankHealerAction::Execute(Event /*event*/)
{
    Player* mageTank = GetZerevorMageTank(bot);
    if (!mageTank)
        return false;

    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor || zerevor->GetVictim() != mageTank)
        return false;

    const ObjectGuid guid = bot->GetGUID();
    uint8 index = zerevorHealStep.count(guid) ? zerevorHealStep[guid] : 0;

    const Position& position = ZEREVOR_HEALER_POSITIONS[index];

    constexpr float maxDistance = 1.0f;
    float distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= maxDistance && HasDangerousCouncilAura(bot))
    {
        index = (index + 1) % 2;
        zerevorHealStep[guid] = index;
        const Position& newPosition = ZEREVOR_HEALER_POSITIONS[index];
        float newDistToPosition = bot->GetExactDist2d(newPosition);
        if (newDistToPosition > maxDistance)
        {
            return MoveTo(BLACK_TEMPLE_MAP_ID, newPosition.GetPositionX(), newPosition.GetPositionY(),
                          bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    else if (distToPosition > maxDistance)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(),
                      false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool IllidariCouncilCommandPetsToAttackGathiosAction::Execute(Event /*event*/)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return false;

    Pet* pet = bot->GetPet();
    if (pet && pet->IsAlive() && pet->GetVictim() != gathios)
    {
        pet->ClearUnitState(UNIT_STATE_FOLLOW);
        pet->AttackStop();
        pet->SetTarget(gathios->GetGUID());

        if (pet->GetCharmInfo())
        {
            pet->GetCharmInfo()->SetIsCommandAttack(true);
            pet->GetCharmInfo()->SetIsAtStay(false);
            pet->GetCharmInfo()->SetIsFollowing(false);
            pet->GetCharmInfo()->SetIsCommandFollow(false);
            pet->GetCharmInfo()->SetIsReturning(false);

            pet->AI()->AttackStart(gathios);
            return true;
        }
    }

    return false;
}

bool IllidariCouncilAssignDpsTargetsAction::Execute(Event /*event*/)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    bool shouldAttackMalande = false;
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (zerevor && zerevor->GetExactDist2d(malande) < 15.0f)
        shouldAttackMalande = false;
    else if (bot->getClass() == CLASS_ROGUE ||
             (bot->getClass() == CLASS_WARRIOR && botAI->IsDps(bot)))
        shouldAttackMalande = !malande->HasAura(SPELL_BLESSING_OF_PROTECTION);
    else if (bot->getClass() == CLASS_SHAMAN && botAI->IsDps(bot))
        shouldAttackMalande = !malande->HasAura(SPELL_BLESSING_OF_SPELL_WARDING);

    if (shouldAttackMalande)
    {
        SetRtiTarget(botAI, "star", malande);

        if (bot->GetTarget() != malande->GetGUID())
            return Attack(malande);
    }
    else if (Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
             darkshadow && !darkshadow->HasAura(SPELL_VANISH))
    {
        SetRtiTarget(botAI, "circle", darkshadow);

        if (bot->GetTarget() != darkshadow->GetGUID())
            return Attack(darkshadow);
    }
    else if (Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
    {
        SetRtiTarget(botAI, "square", gathios);

        if (bot->GetTarget() != gathios->GetGUID())
            return Attack(gathios);
    }

    return false;
}

bool IllidariCouncilManageDpsTimerAction::Execute(Event /*event*/)
{
    if (Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
    {
        return councilDpsWaitTimer.try_emplace(
            gathios->GetMap()->GetInstanceId(), std::time(nullptr)).second;
    }

    return false;
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageMisdirectToTankAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (GetIllidanPhase(illidan) == 2 && TryMisdirectToFlameTanks(group))
        return true;
    else if (GetIllidanPhase(illidan) == 4 && TryMisdirectToWarlockTank(illidan))
        return true;

    return false;
}

bool IllidanStormrageMisdirectToTankAction::TryMisdirectToFlameTanks(Group* group)
{
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
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

    Player* firstAssistTank = GetGroupFirstAssistTank(botAI, bot);
    Player* secondAssistTank = GetGroupSecondAssistTank(botAI, bot);
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
        tankTarget = secondAssistTank;
        flame = eastFlame;
    }
    else if (hunterIndex == 1)
    {
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

bool IllidanStormrageMisdirectToTankAction::TryMisdirectToWarlockTank(Unit* illidan)
{
    Player* warlockTank = GetIllidanWarlockTank(bot);
    if (!warlockTank)
        return false;

    if (botAI->CanCastSpell("misdirection", warlockTank))
        return botAI->CastSpell("misdirection", warlockTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", illidan))
        return botAI->CastSpell("steady shot", illidan);

    return false;
}

bool IllidanStormrageMainTankMoveAwayFromFlameCrashAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (bot->GetVictim() != illidan)
        return Attack(illidan);

    if (GetIllidanPhase(illidan) == 5) // I don't think this works right now
    {
        auto const& gos = AI_VALUE(GuidVector, "nearest game objects");

        // Log details about all nearest game objects
        std::ostringstream oss;
        oss << "Nearest game objects for bot " << bot->GetName() << ":";
        for (ObjectGuid const& guid : gos)
        {
            GameObject* go = botAI->GetGameObject(guid);
            if (!go)
            {
                oss << "\n  [guid=" << guid.ToString() << "] (nullptr)";
                continue;
            }
            oss << "\n  [guid=" << guid.ToString()
                << ", entry=" << go->GetEntry()
                << ", spawned=" << go->isSpawned()
                << ", dist=" << bot->GetExactDist2d(go) << "]";
        }
        LOG_DEBUG("playerbots", "{}", oss.str());
        // End logging

        GameObject* nearestTrap = nullptr;
        float maxDist = 30.0f; // Need to test what distance is worth it in terms of traps
        for (ObjectGuid const& guid : gos)
        {
            GameObject* go = botAI->GetGameObject(guid);
            if (!go || !go->isSpawned() || go->GetEntry() != GO_CAGE_TRAP)
                continue;
            float distToTrap = bot->GetExactDist2d(go);
            if (distToTrap < maxDist)
            {
                maxDist = distToTrap;
                nearestTrap = go;
            }
        }

        if (nearestTrap && illidan->GetVictim() == bot)
        {
            Position target = GetPointBeyondTrap(nearestTrap, 5.0f);
            return MoveTo(BLACK_TEMPLE_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                          target.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED, true, true);
        }
    }

    auto const& flameCrashes = GetAllFlameCrashes(bot);
    if (flameCrashes.empty())
        return false;

    constexpr float hazardRadius = 12.0f;
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

    constexpr float maxRadius = 30.0f;
    Position safestPos = FindSafestNearbyPosition(flameCrashes, maxRadius, hazardRadius);

    return MoveTo(BLACK_TEMPLE_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  bot->GetPositionZ(), false, false, false, true,
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
        return Position(trapX, trapY, bot->GetPositionZ());

    // Normalize and extend beyond trap
    float dx = trapX - botX;
    float dy = trapY - botY;
    float targetX = trapX + (dx / distToTrap) * extraDistance;
    float targetY = trapY + (dy / distToTrap) * extraDistance;
    float targetZ = bot->GetPositionZ();

    return Position(targetX, targetY, targetZ);
}

Position IllidanStormrageMainTankMoveAwayFromFlameCrashAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& flameCrashes, float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI / 16.0f;
    constexpr float minDistance = 2.0f;
    constexpr float distanceStep = 1.0f;

    float backwardsAngle = Position::NormalizeOrientation(bot->GetOrientation() + M_PI);

    Position bestPos;
    float bestAngleDiff = M_PI * 2.0f;
    float bestDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return bestPos;

    for (float distance = minDistance; distance <= maxRadius; distance += distanceStep)
    {
        for (float angleOffset = 0.0f; angleOffset < 2 * M_PI; angleOffset += searchStep)
        {
            for (int sign = -1; sign <= 1; sign += 2)
            {
                float testAngle =
                    Position::NormalizeOrientation(backwardsAngle + sign * angleOffset);
                float x = bot->GetPositionX() + distance * std::cos(testAngle);
                float y = bot->GetPositionY() + distance * std::sin(testAngle);

                Position testPos(x, y, bot->GetPositionZ());

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

                bool pathSafe = IsPathSafeFromFlameCrashes(bot->GetPosition(), testPos,
                                                           flameCrashes, hazardRadius);

                float angleDiff = std::abs(Position::NormalizeOrientation(
                                           testAngle - backwardsAngle));
                if (angleDiff > M_PI)
                    angleDiff = 2 * M_PI - angleDiff;

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

bool IllidanStormrageMainTankMoveAwayFromFlameCrashAction::IsPathSafeFromFlameCrashes(
    const Position& start, const Position& end, const std::vector<Unit*>& flameCrashes,
    float hazardRadius)
{
    constexpr uint8 numChecks = 10;
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

bool IllidanStormrageIsolateBotWithParasiteAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 10.0f;
    constexpr uint32 minInterval = 0;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);
    }

    return false;
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::Execute(Event /*event*/)
{
    auto [eastFlame, westFlame] = GetFlamesOfAzzinoth(botAI, bot);

    // The second assist tank's flame is killed first; this is so that if the tank
    // for the second flame dies after the first flame is down, the dead flame's
    // tank will become the first assist tank and take over the remaining flame
    if (botAI->IsAssistTankOfIndex(bot, 1, true))
    {
        if (eastFlame && westFlame)
        {
            if (bot->GetVictim() != eastFlame)
                return Attack(eastFlame);

            if (eastFlame->GetVictim() != bot)
            {
                if (!bot->IsWithinMeleeRange(eastFlame))
                {
                    return MoveTo(BLACK_TEMPLE_MAP_ID, eastFlame->GetPositionX(),
                                  eastFlame->GetPositionY(), bot->GetPositionZ(),
                                  false, false, false, false,
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
                return false;
            }
        }
        else if (!eastFlame && !westFlame)
        {
            const Position& pos = ILLIDAN_E_GLAIVE_WAITING_POSITION;
            if (bot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) > 0.5f)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, pos.GetPositionX(), pos.GetPositionY(),
                              pos.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        // After the first flame dies, its tank waits with all bots other than the second flame's tank
        else if (!eastFlame && westFlame)
        {
            const Position& pos = ILLIDAN_S_GRATE_POSITION;
            if (bot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) > 0.5f)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, pos.GetPositionX(), pos.GetPositionY(),
                              pos.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        if (westFlame)
        {
            if (bot->GetVictim() != westFlame)
                return Attack(westFlame);

            if (westFlame->GetVictim() != bot)
            {
                if (!bot->IsWithinMeleeRange(westFlame))
                {
                    return MoveTo(BLACK_TEMPLE_MAP_ID, westFlame->GetPositionX(), bot->GetPositionY(),
                                  bot->GetPositionZ(), false, false, false, false,
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
                return false;
            }
        }
        else
        {
            const Position& pos = ILLIDAN_W_GLAIVE_WAITING_POSITION;
            if (bot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) > 0.5f)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, pos.GetPositionX(), pos.GetPositionY(),
                              bot->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    EyeBlastDangerArea dangerArea = GetEyeBlastDangerArea(botAI, bot, illidan);
    if (dangerArea.width > 0.0f)
        return RepositionToAvoidEyeBlast(illidan, dangerArea);
    else
        return RepositionToAvoidBlaze(eastFlame, westFlame);

    return false;
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidEyeBlast(
    Unit* illidan, const EyeBlastDangerArea& dangerArea)
{
    if (IsPositionInEyeBlastDangerArea(bot->GetPosition(), dangerArea))
    {
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

        if (moveDist <= 0.0f)
            return false;

        float dirX = px - closestX;
        float dirY = py - closestY;
        float dirLength = std::sqrt(dirX * dirX + dirY * dirY);
        if (dirLength == 0.0f)
        {
            dirX = -(dy / length);
            dirY = dx / length;
            dirLength = 1.0f;
        }

        float safeX = px + (dirX / dirLength) * moveDist;
        float safeY = py + (dirY / dirLength) * moveDist;
        float safeZ = bot->GetPositionZ();

        constexpr float minGrateDistance = 10.0f;
        bool tooCloseToNorthGrate =
            Position(safeX, safeY, safeZ).GetExactDist2d(ILLIDAN_N_GRATE_POSITION) < minGrateDistance;
        bool tooCloseToSouthGrate =
            Position(safeX, safeY, safeZ).GetExactDist2d(ILLIDAN_S_GRATE_POSITION) < minGrateDistance;

        if (tooCloseToNorthGrate || tooCloseToSouthGrate)
            return false;

        return MoveTo(BLACK_TEMPLE_MAP_ID, safeX, safeY, safeZ,
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidBlaze(
    Unit* eastFlame, Unit* westFlame)
{
    const std::array<Position, 7>* waypoints = nullptr;
    constexpr size_t numWaypoints = 7;

    if (botAI->IsAssistTankOfIndex(bot, 1, true))
    {
        if (!eastFlame || eastFlame->GetVictim() != bot || !bot->IsWithinMeleeRange(eastFlame))
            return false;
        waypoints = &E_GLAIVE_TANK_POSITIONS;
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        if (!westFlame || westFlame->GetVictim() != bot || !bot->IsWithinMeleeRange(westFlame))
            return false;
        waypoints = &W_GLAIVE_TANK_POSITIONS;
    }

    size_t& waypointIndex = flameTankWaypointIndex[bot->GetGUID()];
    const Position& target = (*waypoints)[waypointIndex];

    // Check for nearby blaze and increment only if bot is at current waypoint
    auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible triggers")->Get();
    bool blazeNearby = false;
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_BLAZE && bot->GetDistance2d(unit) <= 8.0f)
        {
            blazeNearby = true;
            break;
        }
    }

    float distToPosition = bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY());
    // Move to next waypoint if blaze is nearby and bot is at current waypoint
    if (blazeNearby && distToPosition <= 0.2f)
    {
        waypointIndex = (waypointIndex + 1) % numWaypoints;
        const Position& newTarget = (*waypoints)[waypointIndex];
        float distToNewPosition = bot->GetExactDist2d(newTarget.GetPositionX(), newTarget.GetPositionY());

        if (distToNewPosition > 0.2f)
        {
            float dX = newTarget.GetPositionX() - bot->GetPositionX();
            float dY = newTarget.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToNewPosition);
            float moveX = bot->GetPositionX() + (dX / distToNewPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToNewPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, newTarget.GetPositionX(), newTarget.GetPositionY(),
                          bot->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }
    // Move to current waypoint if no blaze nearby
    else if (distToPosition > 0.2f)
    {
        float dX = target.GetPositionX() - bot->GetPositionX();
        float dY = target.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(5.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                      bot->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

// Pets grab aggro right away during Phase 2 and wipe the raid if not put on passive
// Just like players, pets cannot melee Illidan during Phase 4
bool IllidanStormrageControlPetAggressionAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    if ((GetIllidanPhase(illidan) == 2 || GetIllidanPhase(illidan) == 4) &&
        pet->GetReactState() != REACT_PASSIVE)
    {
        pet->AttackStop();
        pet->SetReactState(REACT_PASSIVE);
    }
    else if (pet->GetReactState() == REACT_PASSIVE)
    {
        pet->SetReactState(REACT_DEFENSIVE);
    }

    return false;
}

bool IllidanStormragePositionAboveGrateAction::Execute(Event /*event*/)
{
    const std::array<Position, 2>& gratePositions = GRATE_POSITIONS;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> bots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && !botAI->IsAssistTankOfIndex(member, 0, true) &&
            !botAI->IsAssistTankOfIndex(member, 1, true))
            bots.push_back(member);
    }

    if (bots.empty())
        return false;

    std::sort(bots.begin(), bots.end(),
        [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    auto it = std::find(bots.begin(), bots.end(), bot);
    if (it == bots.end())
        return false;

    size_t botIndex = std::distance(bots.begin(), it);
    uint8 index = botIndex % 2; // 0 = north, 1 = south

    const Position& position = gratePositions[index];
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 0.2f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool IllidanStormrageRemoveDarkBarrageAction::Execute(Event /*event*/)
{
    static const std::array<const char*, 3> abilities =
    {
        "divine shield", "ice block", "cloak of shadows"
    };

    for (const char* spellName : abilities)
    {
        if (botAI->CanCastSpell(spellName, bot))
            return botAI->CastSpell(spellName, bot);
    }

    return false;
}

bool IllidanStormrageMoveAwayFromLandingPointAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    float currentDistance = bot->GetExactDist2d(illidan);
    constexpr float safeDistance = 20.0f;
    if (currentDistance < safeDistance)
        return MoveAway(illidan, safeDistance - currentDistance);

    return false;
}

// NOTE: Illidan's bounding radius is 0.459f, and combatreach is 7.5f
bool IllidanStormrageDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (GetIllidanPhase(illidan) == 3 || GetIllidanPhase(illidan) == 5)
        return FanOutBehindInHumanPhase(illidan, group);
    else if (GetIllidanPhase(illidan) == 4)
        return SpreadInCircleInDemonPhase(illidan, group);

    return false;
}

bool IllidanStormrageDisperseRangedAction::FanOutBehindInHumanPhase(
    Unit* illidan, Group* group)
{
    if (illidan->GetPositionZ() > ILLIDAN_FLOOR_Z_THRESHOLD)
        return false;

    auto const& flameCrashes = GetAllFlameCrashes(bot);

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    constexpr float arcSpan = M_PI; // 180 degrees
    float arcCenter = illidan->GetOrientation() + M_PI;
    float arcStart = arcCenter - arcSpan / 2.0f;

    float radius = botAI->IsHeal(bot) ? 18.0f : 25.0f;
    auto& bots = botAI->IsHeal(bot) ? healers : rangedDps;
    size_t count = bots.size();
    auto findIt = std::find(bots.begin(), bots.end(), bot);
    size_t botIndex = (findIt != bots.end()) ? std::distance(bots.begin(), findIt) : 0;

    // Try to find a safe position for this bot
    float angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float targetX = illidan->GetPositionX() + radius * std::cos(angle);
    float targetY = illidan->GetPositionY() + radius * std::sin(angle);

    bool safe = true;
    for (Unit* flameCrash : flameCrashes)
    {
        if (flameCrash->GetDistance2d(targetX, targetY) < 12.0f)
        {
            safe = false;
            break;
        }
    }

    if (!safe)
        return false;

    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT,
                      true, false);
    }

    return false;
}

bool IllidanStormrageDisperseRangedAction::SpreadInCircleInDemonPhase(
    Unit* illidan, Group* group)
{
    Player* warlockTank = GetIllidanWarlockTank(bot);
    if (!warlockTank)
    {
        constexpr float safeDistFromBoss = 23.0f;
        if (bot->GetExactDist2d(illidan) < safeDistFromBoss)
        {
            constexpr uint32 minInterval = 0;
            if (FleePosition(illidan->GetPosition(), safeDistFromBoss, minInterval))
                return true;
        }

        constexpr float safeDistFromPlayer = 6.0f;
        constexpr uint32 minInterval = 1000;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

        return false;
    }

    std::vector<Player*> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        rangedBots.push_back(member);
    }

    if (rangedBots.empty())
        return false;

    size_t count = rangedBots.size();
    auto findIt = std::find(rangedBots.begin(), rangedBots.end(), bot);
    size_t botIndex = (findIt != rangedBots.end()) ? std::distance(rangedBots.begin(), findIt) : 0;

    float dx = warlockTank->GetPositionX() - illidan->GetPositionX();
    float dy = warlockTank->GetPositionY() - illidan->GetPositionY();
    float warlockAngle = std::atan2(dy, dx);

    constexpr float forbiddenArc = (2.0f / 3.0f) * M_PI; // 120 degrees
    constexpr float allowedArc = (4.0f / 3.0f) * M_PI;   // 240 degrees

    float arcStart = Position::NormalizeOrientation(warlockAngle + forbiddenArc / 2.0f);
    constexpr float radius = 25.0f;

    float angle = (count == 1) ? Position::NormalizeOrientation(arcStart + allowedArc / 2.0f) :
        Position::NormalizeOrientation(
            arcStart + allowedArc * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float targetX = illidan->GetPositionX() + radius * std::cos(angle);
    float targetY = illidan->GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Melee cannot attack Demon Form Illidan
bool IllidanStormrageMeleeGoSomewhereToNotDieAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    float currentDistFromBoss = bot->GetExactDist2d(illidan);
    constexpr float safeDistFromBoss = 35.0f;
    if (currentDistFromBoss < safeDistFromBoss)
        MoveAway(illidan, safeDistFromBoss - currentDistFromBoss);

    if (Player* warlockTank = GetIllidanWarlockTank(bot))
    {
        float currentDistFromTank = bot->GetExactDist2d(warlockTank);
        constexpr float safeDistFromTank = 25.0f;
        if (currentDistFromTank < safeDistFromTank)
            MoveAway(warlockTank, safeDistFromTank - currentDistFromTank);
    }

    constexpr float safeDistFromPlayer = 6.0f;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        MoveAway(nearestPlayer, safeDistFromPlayer - bot->GetDistance2d(nearestPlayer));

    return true;
}

bool IllidanStormrageWarlockTankHandleDemonBossAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    constexpr float safeDistFromBoss = 25.0f;
    constexpr uint32 minInterval = 0;
    if (bot->GetExactDist2d(illidan) < safeDistFromBoss &&
        FleePosition(illidan->GetPosition(), safeDistFromBoss, minInterval))
        return true;

    if (botAI->CanCastSpell("searing pain", illidan))
        return botAI->CastSpell("searing pain", illidan);

    return false;
}

bool IllidanStormrageDpsPrioritizeAddsAction::Execute(Event /*event*/)
{
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets")->Get();

    Unit* shadowfiend = nullptr;
    Unit* shadowDemon = nullptr;
    Unit* illidan = nullptr;

    // 1. Find the closest of each specific add type
    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        switch (unit->GetEntry())
        {
            case NPC_SHADOW_DEMON:
                if (!shadowDemon || bot->GetExactDist2d(unit) < bot->GetExactDist2d(shadowDemon))
                    shadowDemon = unit;
                break;

            case NPC_PARASITIC_SHADOWFIEND:
                if (!shadowfiend || bot->GetExactDist2d(unit) < bot->GetExactDist2d(shadowfiend))
                    shadowfiend = unit;
                break;

            case NPC_ILLIDAN_STORMRAGE:
                illidan = unit;
                break;

            default:
                break;
        }
    }

    auto [eastFlame, westFlame] = GetFlamesOfAzzinoth(botAI, bot);
    Player* warlockTank = GetIllidanWarlockTank(bot);

    // 2. Build the priority list based on Phase and Role
    std::vector<Unit*> targets;

    int phase = GetIllidanPhase(illidan);
    if (phase == 2)
    {
        if (botAI->IsAssistTankOfIndex(bot, 1, true))
            targets = { eastFlame, westFlame };
        else if (botAI->IsAssistTankOfIndex(bot, 0, true))
            targets = { westFlame, eastFlame };
        else if (botAI->IsRanged(bot))
            targets = { shadowfiend, eastFlame, westFlame };
        else
            targets = { eastFlame, westFlame };
    }
    else if (phase == 4)
    {
        if (botAI->IsRanged(bot) && bot != warlockTank)
            targets = { /*shadowDemon,*/ shadowfiend, illidan };
        else if (bot == warlockTank)
            targets = { illidan };
        else
            targets = { /*shadowDemon*/ };
    }
    else if (phase == 1 || phase == 3 || phase == 5)
    {
        if (botAI->IsRanged(bot))
            targets = { shadowfiend, illidan };
        else
            targets = { illidan };
    }

    // 3. Select the highest priority valid target
    Unit* target = nullptr;
    for (Unit* candidate : targets)
    {
        if (candidate && candidate->IsAlive())
        {
            target = candidate;
            break;
        }
    }

    // 4. Attack
    if (target && bot->GetTarget() != target->GetGUID())
        return Attack(target);

    return false;
}

bool IllidanStormrageManageDpsTimerAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    const time_t now = std::time(nullptr);
    const uint32 instanceId = illidan->GetMap()->GetInstanceId();

    bool updated = false;
    if ((GetIllidanPhase(illidan) == 2 &&
         AI_VALUE2(Unit*, "find target", "flame of azzinoth")) ||
         GetIllidanPhase(illidan) == 5)
    {
        if (illidanBossDpsWaitTimer.erase(instanceId) > 0)
            updated = true;
    }
    else if (GetIllidanPhase(illidan) == 1 || GetIllidanPhase(illidan) == 3 ||
             GetIllidanPhase(illidan) == 4)
    {
        if (illidanBossDpsWaitTimer.try_emplace(instanceId, now).second)
            updated = true;
        if (illidanFlameDpsWaitTimer.erase(instanceId) > 0)
            updated = true;
    }
    else if (GetIllidanPhase(illidan) == 2 &&
             !AI_VALUE2(Unit*, "find target", "flame of azzinoth"))
    {
        if (illidanFlameDpsWaitTimer.insert_or_assign(instanceId, now).second)
            updated = true;
    }

    return updated;
}

bool IllidanStormrageDestroyHazardsCheatAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int phase = GetIllidanPhase(illidan);
    constexpr float searchRadius = 100.0f;
    std::list<Creature*> hazards;

    if (phase == 2)
    {
        bot->GetCreatureListWithEntryInGrid(hazards, NPC_PARASITIC_SHADOWFIEND, searchRadius);
        bot->GetCreatureListWithEntryInGrid(hazards, NPC_FLAME_CRASH, searchRadius);
    }
    else if (phase == 4)
    {
        bot->GetCreatureListWithEntryInGrid(hazards, NPC_SHADOW_DEMON, searchRadius);
        bot->GetCreatureListWithEntryInGrid(hazards, NPC_FLAME_CRASH, searchRadius);
    }
    else if (phase == 0)
    {
        bot->GetCreatureListWithEntryInGrid(hazards, NPC_DEMON_FIRE, searchRadius);
        bot->GetCreatureListWithEntryInGrid(hazards, NPC_BLAZE, searchRadius);
    }

    bool destroyed = false;

    for (Creature* creature : hazards)
    {
        if (creature && creature->IsAlive())
        {
            creature->Kill(bot, creature);
            destroyed = true;
        }
    }

    return destroyed;
}
