#include "Playerbots.h"
#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "Timer.h"
#include "WarlockActions.h"
#include "AiObjectContext.h"
#include "PlayerbotMgr.h"
#include "PlayerbotAI.h"

#include "Log.h"

namespace {
    static std::map<ObjectGuid, uint32> beamMoveTimes;
    static std::map<ObjectGuid, bool> lastBeamMoveSideways;
}

bool KarazhanAttumenTheHuntsmanStackBehindAction::Execute(Event event)
{
    RaidKarazhanHelpers helper(botAI);
    Unit* boss = helper.GetFirstAliveUnitByEntry(NPC_ATTUMEN_THE_HUNTSMAN);

    float distance = 5.0f;
    float orientation = boss->GetOrientation() + M_PI;
    float x = boss->GetPositionX();
    float y = boss->GetPositionY();
    float z = boss->GetPositionZ();
    float rx = x + cos(orientation) * distance;
    float ry = y + sin(orientation) * distance;
    return MoveTo(bot->GetMapId(), rx, ry, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

bool KarazhanAttumenTheHuntsmanStackBehindAction::isUseful()
{
    RaidKarazhanHelpers helper(botAI);
    Unit* boss = helper.GetFirstAliveUnitByEntry(NPC_ATTUMEN_THE_HUNTSMAN);
    if (!boss)
        return false;
    return !botAI->IsTank(bot);
}

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

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanMaidenOfVirtuePositionBossAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maiden of virtue");

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
    if (!boss)
        return false;
    return botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot;
}

bool KarazhanMaidenOfVirtuePositionRangedAction::Execute(Event event)
{
    int maxIndex = 7;
    int index = 0;

    const GuidVector members = AI_VALUE(GuidVector, "group members");

    for (const auto& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);

    if (!member || !botAI->IsRanged(member->ToPlayer()))
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

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanWizardOfOzFearRoarAction::Execute(Event event)
{
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->getClass() != CLASS_WARLOCK)
            continue;

        PlayerbotAI* warlockAI = sPlayerbotsMgr->GetPlayerbotAI(member);
        if (!warlockAI)
            continue;

        warlockAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(roar);

        CastFearOnCcAction fearAction(warlockAI);
        if (fearAction.isPossible())
        {
            return fearAction.Execute(event);
        }
    }
    return false;
}

bool KarazhanWizardOfOzFearRoarAction::isUseful()
{
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    return roar && roar->IsAlive() && !roar->HasAura(6215);
}

bool KarazhanWizardOfOzScorchStrawmanAction::Execute(Event event)
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const std::vector<uint32> scorchSpellIds = {42859, 42858, 10207};

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->getClass() != CLASS_MAGE)
            continue;

        PlayerbotAI* mageAI = sPlayerbotsMgr->GetPlayerbotAI(member);
        if (!mageAI)
            continue;

        uint32 knownScorchId = 0;
        for (uint32 spellId : scorchSpellIds)
        {
            if (member->HasSpell(spellId))
            {
                knownScorchId = spellId;
                break;
            }
        }
        if (!knownScorchId)
            continue;

        mageAI->CastSpell(knownScorchId, strawman);
    }
    return false;
}

bool KarazhanWizardOfOzScorchStrawmanAction::isUseful()
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    return strawman && strawman->IsAlive();
}

bool KarazhanTheCuratorMarkTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "astral flare");

    if (!target || !target->IsAlive())
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanTheCuratorPositionBossAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");

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
    if (!boss)
        return false;
    return botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot;
}

bool KarazhanTheCuratorSpreadRangedAction::Execute(Event event)
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

    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* target = karazhanHelper.GetFirstAliveUnitByEntry(NPC_DEMON_CHAINS);

    if (!target || !target->IsAlive())
        target = boss;

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanShadeOfAranArcaneExplosionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");

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

bool KarazhanShadeOfAranFlameWreathAction::Execute(Event event)
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

bool KarazhanShadeOfAranMarkTargetAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* target = karazhanHelper.GetFirstAliveUnitByEntry(NPC_CONJURED_ELEMENTAL);

    if (!target || !target->IsAlive())
        return false;

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanShadeOfAranSpreadRangedAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");

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

bool KarazhanNetherspiteBlockRedBeamAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 100.0f);
    if (!boss || !redPortal)
        return false;

    // Find first tank in group without Nether Exhaustion Red
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* eligibleTank = nullptr;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;
        PlayerbotAI* memberAI = sPlayerbotsMgr->GetPlayerbotAI(member);
        if (!memberAI || !memberAI->IsTank(member))
            continue;
        if (member->HasAura(SPELL_NETHER_EXHAUSTION_RED))
            continue;
        eligibleTank = member;
        break;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    Position beamPos = karazhanHelper.GetPositionOnBeam(boss, redPortal, 18.0f);

    if (bot == eligibleTank)
    {
        ObjectGuid botGuid = bot->GetGUID();
        uint32 intervalSecs = 5;
        if (beamMoveTimes[botGuid] == 0)
        {
            beamMoveTimes[botGuid] = time(nullptr);
            lastBeamMoveSideways[botGuid] = false;
        }
        if (time(nullptr) - beamMoveTimes[botGuid] >= intervalSecs)
        {
            lastBeamMoveSideways[botGuid] = !lastBeamMoveSideways[botGuid];
            beamMoveTimes[botGuid] = time(nullptr);
        }
        if (!lastBeamMoveSideways[botGuid]) {
            // Move to beam position
            return MoveTo(bot->GetMapId(), beamPos.GetPositionX(), beamPos.GetPositionY(), beamPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        } else {
            // Move sideways 3 yards (perpendicular to beam)
            float bx = boss->GetPositionX();
            float by = boss->GetPositionY();
            float px = redPortal->GetPositionX();
            float py = redPortal->GetPositionY();
            float dx = px - bx;
            float dy = py - by;
            float length = sqrt(dx*dx + dy*dy);
            if (length == 0.0f)
                return false;
            dx /= length;
            dy /= length;
            // Perpendicular direction
            float perpDx = -dy;
            float perpDy = dx;
            float sideX = beamPos.GetPositionX() + perpDx * 3.0f;
            float sideY = beamPos.GetPositionY() + perpDy * 3.0f;
            float sideZ = beamPos.GetPositionZ();
            return MoveTo(bot->GetMapId(), sideX, sideY, sideZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    return false;
}

bool KarazhanNetherspiteBlockRedBeamAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    ObjectGuid botGuid = bot->GetGUID();
    static std::map<ObjectGuid, bool> lastBossBanishState;
    bool bossIsBanished = boss && boss->HasAura(SPELL_BANISH);

    if (lastBossBanishState[botGuid] != bossIsBanished) {
        // Boss banish state changed
        if (!bossIsBanished) {
            // Banish ended, reset timer/state
            beamMoveTimes[botGuid] = 0;
            lastBeamMoveSideways[botGuid] = false;
        }
        lastBossBanishState[botGuid] = bossIsBanished;
    }
    
    if (bossIsBanished)
        return false;

    return true;
}

/*bool KarazhanNetherspiteBlockBlueBeamAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 100.0f);
    if (!boss || !bluePortal)
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    std::vector<Player*> blueBlockers = karazhanHelper.GetBlueBlockers();
    bool isBlueBlocker = std::find(blueBlockers.begin(), blueBlockers.end(), bot) != blueBlockers.end();
    Player* eligibleBlueBlocker = blueBlockers.empty() ? nullptr : blueBlockers.front();
    Position beamPos = karazhanHelper.GetPositionOnBeam(boss, bluePortal, 18.0f);

    if (bot == eligibleBlueBlocker || (isBlueBlocker && bot->GetExactDist2d(beamPos.GetPositionX(), beamPos.GetPositionY()) < 1.5f))
    {
        // Stay at beam position as long as bot is a blue blocker
        return MoveTo(bot->GetMapId(), beamPos.GetPositionX(), beamPos.GetPositionY(), beamPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    return false;
}

bool KarazhanNetherspiteBlockBlueBeamAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    if (boss->HasAura(SPELL_BANISH))
        return false;

    return true;
}*/

/*bool KarazhanNetherspiteBlockGreenBeamAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 100.0f);
    if (!boss || !greenPortal)
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    std::vector<Player*> greenBlockers = karazhanHelper.GetGreenBlockers();
    bool isGreenBlocker = std::find(greenBlockers.begin(), greenBlockers.end(), bot) != greenBlockers.end();
    Player* eligibleGreenBlocker = greenBlockers.empty() ? nullptr : greenBlockers.front();
    Position beamPos = karazhanHelper.GetPositionOnBeam(boss, greenPortal, 18.0f);

    if (bot == eligibleGreenBlocker || (isGreenBlocker && bot->GetExactDist2d(beamPos.GetPositionX(), beamPos.GetPositionY()) < 1.5f))
    {
        // Stay at beam position as long as bot is a green blocker
        return MoveTo(bot->GetMapId(), beamPos.GetPositionX(), beamPos.GetPositionY(), beamPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    return false;
}

bool KarazhanNetherspiteBlockGreenBeamAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    if (boss->HasAura(SPELL_BANISH))
        return false;

    return true;
}*/

/* bool KarazhanNetherspiteAvoidBeamAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss)
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    auto [redBlocker, greenBlocker, blueBlocker] = karazhanHelper.GetCurrentBeamBlockers();

    // If this bot is a current blocker, do nothing
    if (bot == redBlocker || bot == greenBlocker || bot == blueBlocker)
        return false;

    // Assign beam by role
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    auto [redBlocker, greenBlocker, blueBlocker] = karazhanHelper.GetCurrentBeamBlockers();

    // If this bot is a current blocker, do nothing
    if (bot == redBlocker || bot == greenBlocker || bot == blueBlocker)
        return false;

    // Assign beam by role
    Unit* portal = nullptr;
    float minDistance = 5.0f;
    float maxDistance = 10.0f;
    PlayerbotAI* memberAI = botAI;
    bool isTank = memberAI->IsTank(bot);
    bool isHealer = memberAI->IsHeal(bot);
    bool isRogue = bot->getClass() == CLASS_ROGUE;
    bool isDpsWarrior = bot->getClass() == CLASS_WARRIOR && !isTank;
    bool isDps = memberAI->IsDps(bot);

    if (isTank)
        portal = bot->FindNearestCreature(NPC_RED_PORTAL, 100.0f);
    else if (isHealer || isRogue || isDpsWarrior)
        portal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 100.0f);
    else if (isDps)
        portal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 100.0f);

    if (!portal)
        return false;

    Position avoidPos = karazhanHelper.GetAvoidBeamPosition(boss, portal, minDistance, maxDistance);
    return MoveTo(bot->GetMapId(), avoidPos.GetPositionX(), avoidPos.GetPositionY(), avoidPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool KarazhanNetherspiteAvoidBeamAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    if (boss->HasAura(SPELL_BANISH))
        return false;

    return true;
} */

/* bool KarazhanNetherspiteAvoidVoidZoneAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    if (!boss)
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* voidzone = karazhanHelper.GetNearestUnitByEntryWithinRadius(NPC_VOID_ZONE, 1.5f);

    if (!voidzone)
        return false;

    // Need to add movement logic and ensure it does not break beams. Also to add isUseful function.

    return false;
} */

/* bool KarazhanNetherspiteAvoidVoidZoneAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    if (!boss)
        return false;

    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* voidzone = karazhanHelper.GetNearestUnitByEntryWithinRadius(NPC_VOID_ZONE, 1.5f);

    if (!voidzone)
        return false;

    // Need to add movement logic and ensure it does not break beams. Also to add IsUseful function.

    return false;
} */

bool KarazhanPrinceMalchezaarAvoidInfernalAction::Execute(Event event)
{
    RaidKarazhanHelpers helper(botAI);
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    std::vector<Unit*> infernals = helper.GetSpawnedInfernals();
    const float safeInfernalDistance = 20.0f;
    const float safeInfernalTankingDistance = 25.0f;

    float safeDistance = botAI->IsTank(bot) && botAI->HasAggro(boss) ? safeInfernalTankingDistance : safeInfernalDistance;

    for (Unit* infernal : infernals)
    {
        float distance = bot->GetDistance2d(infernal);
        if (distance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveAway(infernal, safeDistance - distance);
        }
    }
    return false;
}

bool KarazhanPrinceMalchezaarRunAwayFromShadowNovaAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!boss)
        return false;

    RaidKarazhanHelpers helper(botAI);
    std::vector<Unit*> infernals = helper.GetSpawnedInfernals();

    const float safeBossDistance = 30.0f;
    const float safeInfernalDistance = 20.0f;
    float currentBossDistance = bot->GetDistance2d(boss);

    if (currentBossDistance < safeBossDistance)
    {
        // Try to find a safe straight line path and destination
        const float stepSize = 2.0f; // yards per sample
        const int numAngles = 16;
        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2 * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);

            bool pathIsSafe = true;
            for (float dist = stepSize; dist <= safeBossDistance; dist += stepSize)
            {
                float x = bot->GetPositionX() + dx * dist;
                float y = bot->GetPositionY() + dy * dist;
                for (Unit* infernal : infernals)
                {
                    float infernalDist = sqrt(pow(x - infernal->GetPositionX(), 2) + pow(y - infernal->GetPositionY(), 2));
                    if (infernalDist < safeInfernalDistance)
                    {
                        pathIsSafe = false;
                        break;
                    }
                }
                if (!pathIsSafe)
                    break;
            }
            if (pathIsSafe)
            {
                float destX = bot->GetPositionX() + dx * (safeBossDistance - currentBossDistance);
                float destY = bot->GetPositionY() + dy * (safeBossDistance - currentBossDistance);
                float destZ = bot->GetPositionZ();
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(false);
                return MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

bool KarazhanPrinceMalchezaarRunAwayFromShadowNovaAction::isUseful()
{
    return bot->HasAura(SPELL_ENFEEBLE);
}
