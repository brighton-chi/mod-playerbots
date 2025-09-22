#include "RaidMagsLairHelpers.h"
#include "PlayerbotAI.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "Timer.h"

bool IsSouthTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
    {
        return false;
    }
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            return member == bot;
        }
    }
    return false;
}

bool IsWestTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
    {
        return false;
    }
    int tankIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            if (tankIndex == 1)
            {
                return member == bot;
            }
            ++tankIndex;
        }
    }
    return false;
}

bool IsEastTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
    {
        return false;
    }
    int tankIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            if (tankIndex == 2)
            {
                return member == bot;
            }
            ++tankIndex;
        }
    }
    return false;
}

bool IsSouthWarlock(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_WARLOCK)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_WARLOCK)
            return member == bot;
    }
    return false;
}

bool IsWestWarlock(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_WARLOCK)
        return false;

    int warlockIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_WARLOCK)
        {
            if (warlockIndex == 1)
                return member == bot;
            ++warlockIndex;
        }
    }
    return false;
}

bool IsEastWarlock(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_WARLOCK)
        return false;

    int warlockIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_WARLOCK)
        {
            if (warlockIndex == 2)
                return member == bot;
            ++warlockIndex;
        }
    }
    return false;
}

bool IsWestHealer(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsHeal(bot, true))
        return false;

    Player* firstPaladinHealer = nullptr;
    Player* firstOtherHealer = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (!botAI->IsHeal(member, true)) continue;

        if (member->getClass() == CLASS_PALADIN && !firstPaladinHealer)
            firstPaladinHealer = member;
        else if (member->getClass() != CLASS_PALADIN && !firstOtherHealer)
            firstOtherHealer = member;
    }

    if (firstPaladinHealer)
        return bot == firstPaladinHealer;
    if (firstOtherHealer)
        return bot == firstOtherHealer;
    return false;
}

bool IsEastHealer(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsHeal(bot, true))
        return false;

    Player* firstPaladinHealer = nullptr;
    Player* secondPaladinHealer = nullptr;
    Player* firstOtherHealer = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (!botAI->IsHeal(member, true)) continue;

        if (member->getClass() == CLASS_PALADIN)
        {
            if (!firstPaladinHealer)
                firstPaladinHealer = member;
            else if (!secondPaladinHealer)
                secondPaladinHealer = member;
        }
        else if (!firstOtherHealer)
        {
            firstOtherHealer = member;
        }
    }

    if (secondPaladinHealer)
        return bot == secondPaladinHealer;
    if (!firstPaladinHealer && firstOtherHealer)
        return bot == firstOtherHealer;
    return false;
}

bool IsWestHunter(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_HUNTER)
            return member == bot;
    }
    return false;
}

bool IsEastHunter(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
        return false;

    int hunterIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_HUNTER)
        {
            if (hunterIndex == 1)
                return member == bot;
            ++hunterIndex;
        }
    }
    return false;
}

const TankSpot MagtheridonTankSpot = { -7.956f, 45.401f, -0.409f, 4.290f };

const CubePosition manticronCubes[5] = 
{
    { -8.0f, 18.0f, -0.4f },
    { 8.0f, 18.0f, -0.4f },
    { 15.0f, 36.0f, -0.4f },
    { 0.0f, 45.0f, -0.4f },
    { -15.0f, 36.0f, -0.4f }
};

std::map<ObjectGuid, int> cubeAssignments;
std::map<ObjectGuid, time_t> cubeTimers;

void AssignRangedDpsToCubes(Group* group, PlayerbotAI* botAI)
{
    int cubeIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref && cubeIndex < 5; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (!botAI->IsRangedDps(member, true))
            continue;

        cubeAssignments[member->GetGUID()] = cubeIndex++;
    }
}
