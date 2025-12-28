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

    if (najentus->GetVictim() == bot)
    {
        const Position& position = NAJENTUS_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

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

bool SupremusMainTankPositionBossAction::Execute(Event event)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    if (bot->GetVictim() != supremus)
        return Attack(supremus);

    if (supremus->GetVictim() == bot)
    {
        const Position& position = SUPREMUS_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool SupremusSpreadRangedInArcAction::Execute(Event event)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    if (supremus->GetHealth() == supremus->GetMaxHealth())
        supremusRangedPositions.clear();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            continue;
        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    auto it = supremusRangedPositions.find(guid);
    if (it == supremusRangedPositions.end())
    {
        size_t count = healers.size() + rangedDps.size();
        size_t botIndex = 0;
        float radius = 0.0f;
        float angle = 0.0f;

        const float arcSpan = 2.0f * M_PI / 3.0f; // 120 degrees
        const float arcCenter = 0.165f;
        const float arcStart = arcCenter - arcSpan / 2.0f;

        if (botAI->IsHeal(bot))
        {
            auto findIt = std::find(healers.begin(), healers.end(), bot);
            botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 30.0f;
            count = healers.size();
        }
        else
        {
            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            radius = 25.0f;
            count = rangedDps.size();
        }

        angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        float targetX = SUPREMUS_TANK_POSITION.GetPositionX() + radius * std::sin(angle);
        float targetY = SUPREMUS_TANK_POSITION.GetPositionY() + radius * std::cos(angle);
        float targetZ = bot->GetMap()->GetHeight(targetX, targetY, SUPREMUS_TANK_POSITION.GetPositionZ());

        supremusRangedPositions.try_emplace(guid, Position(targetX, targetY, targetZ));
        it = supremusRangedPositions.find(guid);
    }

    if (it == supremusRangedPositions.end())
        return false;

    const Position& target = it->second;
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 2.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                      target.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool SupremusDisperseRangedAction::Execute(Event event)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    // Flee if within 5 yards of any other player
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();

            if (!member || member == bot || !member->IsAlive())
                continue;

            if (bot->GetExactDist2d(member) < 5.0f)
                return FleePosition(Position(member->GetPositionX(), member->GetPositionY(),
                                    member->GetPositionZ()), 5.5f, 1000);
        }
    }

    return false;
}

bool SupremusKiteBossAction::Execute(Event event)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    float currentDistance = bot->GetExactDist2d(supremus);
    const float safeDistance = 20.0f;
    if (currentDistance < safeDistance)
    {
        botAI->Reset();
        return MoveAway(supremus, safeDistance - currentDistance);
    }

    return false;
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

// Teron Gorefiend

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
