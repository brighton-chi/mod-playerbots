#include "Playerbots.h"
#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "Timer.h"

bool KarazhanMoroesMarkTargetAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);

    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");
    Unit* target = karazhanHelper.GetFirstAliveUnit({dorothea, catriona, keira, rafe, robin, crispin});

    if (!target)
        return false;

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanMaidenOfVirtuePositionBossAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    if (!boss)
        return false;

    Unit* healer = nullptr;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();

            if (!member || !member->IsAlive() || !botAI->IsHeal(member) || !member->HasAura(SPELL_REPENTANCE))
                continue;

            healer = member;
            break;
        }
    }

    if (healer)
    {
        float angle = healer->GetOrientation();
        float targetX = healer->GetPositionX() + cos(angle) * 6.0f;
        float targetY = healer->GetPositionY() + sin(angle) * 6.0f;
        float targetZ = healer->GetPositionZ();

        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    const float maxDistance = 3.0f;
    const float distanceToBossPosition = boss->GetExactDist2d(KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION);

    if (distanceToBossPosition > maxDistance)
    {
        float dX = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionX() - boss->GetPositionX();
        float dY = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionY() - boss->GetPositionY();

        float mX = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionX() + (dX / distanceToBossPosition) * maxDistance;
        float mY = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionY() + (dY / distanceToBossPosition) * maxDistance;

        return MoveTo(bot->GetMapId(), mX, mY,
                      bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_FORCED, true,
                      false);
    }

    return false;
}

bool KarazhanMaidenOfVirtuePositionBossAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    return boss && botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot;
}

bool KarazhanMaidenOfVirtuePositionRangedAction::Execute(Event /*event*/)
{
    int maxIndex = 7;
    int index = 0;

    const GuidVector members = AI_VALUE(GuidVector, "group members");

    for (const auto& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);

        if (!member || !botAI->IsRanged(member->ToPlayer()) || member->ToPlayer()->IsGameMaster())
            continue;

        if (member == bot)
            break;

        // Reset index to 0 if there are more than 8 ranged players
        if (index >= maxIndex)
        {
            index = 0;
            continue;
        }

        index++;
    }

    float distance = bot->GetExactDist2d(KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[index]);
    const float maxDistance = 2.0f;

    if (distance > maxDistance)
        return MoveTo(bot->GetMapId(), KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[index].GetPositionX(),
                      KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[index].GetPositionY(), bot->GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);

    return false;
}

bool KarazhanMaidenOfVirtuePositionRangedAction::isUseful()
{
    return botAI->IsRanged(bot);
}

bool KarazhanBigBadWolfRunAwayAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    if (!boss)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);

    constexpr float threshold = 1.0f;
    Position target = KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[currentIndex];

    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) < threshold)
    {
        currentIndex = (currentIndex + 1) % 4;
        target = KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[currentIndex];
    }

    return MoveTo(bot->GetMapId(), target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_FORCED);
}

bool KarazhanBigBadWolfRunAwayAction::isUseful()
{
    return bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD);
}

bool KarazhanRomuloJulianneMarkTargetAction::Execute(Event event)
{
    Unit* target = nullptr;
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");
    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");

    if (!romulo || !julianne)
        return false;

    const int maxPctDifference = 10;

    if (julianne->GetHealthPct() + maxPctDifference < romulo->GetHealthPct() || julianne->GetHealthPct() < 1.0f)
        target = romulo;
    else if (romulo->GetHealthPct() + maxPctDifference < julianne->GetHealthPct() || romulo->GetHealthPct() < 1.0f)
        target = julianne;

    if (!target)
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanWizardOfOzMarkTargetAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* dorothee = AI_VALUE2(Unit*, "find target", "dorothee");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");
    Unit* target = karazhanHelper.GetFirstAliveUnit({dorothee, strawman, tinhead, tito, roar, crone});

    if (!target)
        return false;

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanTheCuratorMarkTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "astral flare");

    if (!target)
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanTheCuratorPositionBossAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");

    if (!boss)
        return false;

    const float maxDistance = 3.0f;
    const float distanceToBossPosition = boss->GetExactDist2d(KARAZHAN_THE_CURATOR_BOSS_POSITION);

    if (distanceToBossPosition > maxDistance)
    {
        float dX = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionX() - boss->GetPositionX();
        float dY = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionY() - boss->GetPositionY();

        float mX = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionX() + (dX / distanceToBossPosition) * maxDistance;
        float mY = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionY() + (dY / distanceToBossPosition) * maxDistance;

        return MoveTo(bot->GetMapId(), mX, mY,
                      bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_FORCED, true,
                      false);
    }

    return false;
}

bool KarazhanTheCuratorPositionBossAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");
    return boss && botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot;
}

bool KarazhanTheCuratorSpreadRangedAction::Execute(Event /*event*/)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    const float minDistance = 5.0f;
    Unit* nearestPlayer = karazhanHelper.GetNearestPlayerInRadius(minDistance);

    if (nearestPlayer)
        return FleePosition(nearestPlayer->GetPosition(), minDistance);

    return false;
}

bool KarazhanTheCuratorSpreadRangedAction::isUseful()
{
    return botAI->IsRanged(bot);
}

bool KarazhanTerestianIllhoofMarkTargetAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "terestian illhoof");

    if (!boss)
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* target = karazhanHelper.GetFirstAliveUnitByEntry(NPC_DEMON_CHAINS);

    if (!target)
        target = boss;

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanShadeOfAranArcaneExplosionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");

    if (!boss)
        return false;

    const float safeDistance = 20.0f;
    const float distance = bot->GetDistance2d(boss);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);

    if (distance < safeDistance)
        return MoveAway(boss, safeDistance - distance);

    return true;
}

bool KarazhanShadeOfAranArcaneExplosionAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    return boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION);
}

bool KarazhanShadeOfAranFlameWreathAction::Execute(Event /*event*/)
{


    AI_VALUE(LastMovement&, "last movement").Set(nullptr);
    bot->GetMotionMaster()->Clear();

    if (bot->isMoving())
        bot->StopMoving();

    return false;
}

bool KarazhanShadeOfAranFlameWreathAction::isUseful()
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    return karazhanHelper.IsFlameWreathActive();
}

bool KarazhanShadeOfAranMarkTargetAction::Execute(Event /*event*/)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* target = karazhanHelper.GetFirstAliveUnitByEntry(NPC_CONJURED_ELEMENTAL);

    if (!target)
        return false;

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanShadeOfAranSpreadRangedAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");

    if (!boss)
        return false;

    const float maxBossDistance = 12.0f;
    float bossDistance = bot->GetExactDist2d(boss);

    if (bossDistance > maxBossDistance)
    {
        float dX = bot->GetPositionX() - boss->GetPositionX();
        float dY = bot->GetPositionY() - boss->GetPositionY();
        float length = std::sqrt(dX * dX + dY * dY);

        dX /= length;
        dY /= length;

        float tX = boss->GetPositionX() + dX * maxBossDistance;
        float tY = boss->GetPositionY() + dY * maxBossDistance;

        return MoveTo(bot->GetMapId(), tX, tY, bot->GetPositionZ(), false, false, false, true,
            MovementPriority::MOVEMENT_COMBAT);
    }

    const float minDistance = 5.0f;
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* nearestPlayer = karazhanHelper.GetNearestPlayerInRadius(minDistance);

    if (nearestPlayer)
        return FleePosition(nearestPlayer->GetPosition(), minDistance);

    return false;
}

bool KarazhanShadeOfAranSpreadRangedAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    RaidKarazhanHelpers karazhanHelper(botAI);
    return boss && botAI->IsRanged(bot) && !karazhanHelper.IsFlameWreathActive() && !(boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION));
}

/*bool KarazhanNetherspiteSoakBeamsAction::Execute(Event /*event*//*)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    if (!boss)
        return false;

    if (boss->HasAura(SPELL_BANISH))
        return false;

    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 100.0f);
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 100.0f);
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 100.0f);

    if (!redPortal || !greenPortal || !bluePortal)
        return false;

    Unit* redSoaker = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();

            if (!member || !member->IsAlive() || member->IsGameMaster() || !botAI->IsTank(member))
                continue;

            if (member->HasAura(SPELL_RED_DEBUFF))
                continue;

            redSoaker = member;
            break;
        }
    }

    Unit* blueSoaker = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();

            if (!member || !member->IsAlive() || member->IsGameMaster() || !botAI->IsDps(member))
                continue;

            if (member->HasAura(SPELL_BLUE_DEBUFF) || (member->GetAura(SPELL_BLUE_BUFF) && member->GetAura(SPELL_BLUE_BUFF)->GetStackAmount() > 30))
                continue;

            blueSoaker = member;
            break;
        }
    }

    Unit* greenSoaker = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();

            if (!member || !member->IsAlive() || member->IsGameMaster() || member == redSoaker || member == blueSoaker)
                continue;

            if (botAI->IsTank(member) && botAI->HasAggro(boss) && boss->GetVictim() == member)
                continue;

            if (member->HasAura(SPELL_GREEN_DEBUFF) || (member->GetAura(SPELL_GREEN_BUFF) && member->GetAura(SPELL_GREEN_BUFF)->GetStackAmount() > 30))
                continue;

            greenSoaker = member;
            break;
        }
    }

    RaidKarazhanHelpers karazhanHelper(botAI);

    if (redSoaker && redSoaker == bot)
    {
        uint32 now = getMSTime();
        bool isBetween = karazhanHelper.IsBetween(boss, redPortal);

        if (redSoaker->GetAura(SPELL_RED_BUFF) && redSoaker->GetAura(SPELL_RED_BUFF)->GetDuration() < 10 * IN_MILLISECONDS)
            lastMoveTime = now;

        if (now - lastMoveTime < 5000 || !redSoaker->GetAura(SPELL_RED_BUFF))
        {
            if (!isBetween)
            {
                Position linePosition = karazhanHelper.GetPositionBetween(boss, redPortal);

                return MoveTo(bot->GetMapId(), linePosition.GetPositionX(), linePosition.GetPositionY(),
                              linePosition.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_FORCED);
            }
        }
        else
        {
            if (isBetween)
            {
                Position stepAwayPosition = karazhanHelper.GetPositionToRotateAroundTarget(boss);

                return MoveTo(bot->GetMapId(), stepAwayPosition.GetPositionX(), stepAwayPosition.GetPositionY(),
                              stepAwayPosition.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_FORCED);
            }
        }

        Position stepPosition = karazhanHelper.GetPositionToMoveCloserToTarget(redPortal);

        if (bot->GetExactDist2d(stepPosition) > 1.0f)
            return MoveTo(bot->GetMapId(), stepPosition.GetPositionX(), stepPosition.GetPositionY(),
                          stepPosition.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    else if ((greenSoaker && greenSoaker == bot) || (blueSoaker && blueSoaker == bot))
    {
        Unit* portal = (greenSoaker == bot) ? greenPortal : bluePortal;

        if (!karazhanHelper.IsBetween(boss, portal))
        {
            Position linePosition = karazhanHelper.GetPositionBetween(boss, portal);

            return MoveTo(bot->GetMapId(), linePosition.GetPositionX(), linePosition.GetPositionY(),
                            linePosition.GetPositionZ(), false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED);
        }
    }
    else
    {
        if (karazhanHelper.IsBetween(boss, redPortal) || karazhanHelper.IsBetween(boss, greenPortal) || karazhanHelper.IsBetween(boss, bluePortal))
        {
            Position stepAwayPosition = karazhanHelper.GetPositionToRotateAroundTarget(boss);

            return MoveTo(bot->GetMapId(), stepAwayPosition.GetPositionX(), stepAwayPosition.GetPositionY(),
                          stepAwayPosition.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool KarazhanNetherspiteAvoidVoidZoneAction::Execute(Event /*event*//*)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    if (!boss)
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* target = karazhanHelper.GetNearestUnitByEntryWithinRadius(NPC_VOIDZONE, 1.5f);

    if (!target)
        return false;

    return false;
}*/
