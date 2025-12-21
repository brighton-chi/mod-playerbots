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

// Supremus

// Shade of Akama

// Teron Gorefiend

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
