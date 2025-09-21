#include "RaidMagsLairHelpers.h"
#include "PlayerbotAI.h"
#include "Group.h"
#include "Timer.h"

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
