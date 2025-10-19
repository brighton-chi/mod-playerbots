#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"

using namespace KarazhanHelpers;

// Attumen the Huntsman

bool AttumenTheHuntsmanMainTankAction::Execute(Event event)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (midnight && !attumenMounted)
    {
        MarkTargetWithSkull(bot, midnight);
        SetRtiTarget(botAI, "skull", midnight);

        if (bot->GetVictim() != midnight)
            return Attack(midnight);
    }

    if (attumenMounted)
    {
        MarkTargetWithSkull(bot, attumenMounted);

        if (bot->GetVictim() != attumenMounted)
            return Attack(attumenMounted);

        if (!bot->IsWithinMeleeRange(attumenMounted))
            return MoveTo(bot->GetMapId(), attumenMounted->GetPositionX(), attumenMounted->GetPositionY(),
                          attumenMounted->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Midnight is the phase 1 target; get Attumen out of the way so he doesn't cleave bots
bool AttumenTheHuntsmanSplitBossesAction::Execute(Event event)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);
    if (!midnight || !attumen)
        return false;

    MarkTargetWithSquare(bot, attumen);
    SetRtiTarget(botAI, "square", attumen);

    if (bot->GetVictim() != attumen)
        return Attack(attumen);

    if (attumen->GetVictim() == bot && midnight->GetVictim() != bot)
    {
        const float minDistance = 6.0f;
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);
        if (nearestPlayer)
            return MoveAway(nearestPlayer, 8.0f, false);
    }

    return false;
}

// Stack 1-5 yards behind mounted Attumen (inside minimum range of Berserker Charge)
bool AttumenTheHuntsmanStackBehindAction::Execute(Event event)
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted)
        return false;

    SetRtiTarget(botAI, "skull", attumenMounted);

    const float distance = 3.0f;
    float orientation = attumenMounted->GetOrientation() + M_PI;
    float x = attumenMounted->GetPositionX();
    float y = attumenMounted->GetPositionY();
    float rx = x + cos(orientation) * distance;
    float ry = y + sin(orientation) * distance;

    if (bot->GetExactDist2d(rx, ry) > 2.0f)
        return MoveTo(bot->GetMapId(), rx, ry, bot->GetPositionZ(), false, false, false, false, 
                      MovementPriority::MOVEMENT_FORCED, true, false);

    return false;
}

// Reset timer for AttumenTheHuntsmanWaitForDPSMultiplier
bool AttumenTheHuntsmanManageTimerAction::Execute(Event event)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!midnight || !attumenMounted)
        return false;

    uint32 mapId = midnight ? midnight->GetMapId() : attumenMounted->GetMapId();

    if (midnight && !attumenMounted && attumenDPSWaitTimer.count(mapId))
        attumenDPSWaitTimer.erase(mapId);

    if (attumenMounted && !attumenDPSWaitTimer.count(mapId))
        attumenDPSWaitTimer[mapId] = time(nullptr);

    return false;
}

// Moroes

// Mark targets with skull in the recommended kill order
bool MoroesMarkTargetAction::Execute(Event event)
{
    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");
    Unit* target = GetFirstAliveUnit({dorothea, catriona, keira, rafe, robin, crispin});

    if (target)
        MarkTargetWithSkull(bot, target);

    return false;
}

// Maiden of Virtue

// Tank the boss in the center of the room
// Move to healers after Repentenace to break the stun
bool MaidenOfVirtueMainTankMovementAction::Execute(Event event)
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    Unit* healer = nullptr;
    if (!maiden)
        return false;

    if (bot->GetVictim() != maiden)
        return Attack(maiden);

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsHeal(member) || 
                !member->HasAura(SPELL_REPENTANCE))
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
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    const float maxDistance = 2.0f;
    float distanceToBossPosition = maiden->GetExactDist2d(KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION);
    if (distanceToBossPosition > maxDistance)
    {
        float dX = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionX() - maiden->GetPositionX();
        float dY = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionY() - maiden->GetPositionY();
        float mX = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionX() + (dX / distanceToBossPosition) * maxDistance;
        float mY = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionY() + (dY / distanceToBossPosition) * maxDistance;
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), mX, mY, bot->GetPositionZ(), false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Spread out ranged DPS between the pillars
bool MaidenOfVirtuePositionRangedAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const uint8 maxIndex = 7;
    uint8 index = 0;

    const GuidVector members = AI_VALUE(GuidVector, "group members");

    for (const auto& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);
        if (!member || !botAI->IsRanged(member->ToPlayer()))
            continue;

        if (member == bot)
            break;

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
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(bot->GetMapId(), KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[index].GetPositionX(),
                      KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[index].GetPositionY(), bot->GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// The Big Bad Wolf

// Tank the boss at the front left corner of the stage
bool BigBadWolfPositionBossAction::Execute(Event event)
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    if (!wolf)
        return false;

    if (bot->GetVictim() != wolf)
        return Attack(wolf);

    if (wolf->GetVictim() == bot)
    {
        const float maxDistance = 2.0f;
        float distanceToBossPosition = wolf->GetExactDist2d(KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION);

        if (distanceToBossPosition > maxDistance)
        {
            float dX = KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION.GetPositionX() - wolf->GetPositionX();
            float dY = KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION.GetPositionY() - wolf->GetPositionY();
            float mX = KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION.GetPositionX() + (dX / distanceToBossPosition) * maxDistance;
            float mY = KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION.GetPositionY() + (dY / distanceToBossPosition) * maxDistance;
            float moveDistance = bot->GetExactDist2d(mX, mY);
            if (moveDistance < 0.5f)
                return false;

            return MoveTo(bot->GetMapId(), mX, mY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        float orientation = atan2(wolf->GetPositionY() - bot->GetPositionY(), 
                            wolf->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }
    else if (!bot->IsWithinMeleeRange(wolf))
        return MoveTo(bot->GetMapId(), wolf->GetPositionX(), wolf->GetPositionY(), wolf->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);

    return false;
}

// Run away, little girl, run away
bool BigBadWolfRunAwayAction::Execute(Event event)
{
    ObjectGuid botGuid = bot->GetGUID();
    const float threshold = 1.0f;
    uint8 index = bigBadWolfRunIndex.count(botGuid) ? bigBadWolfRunIndex[botGuid] : 0;
    Position target = KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[index];

    while (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) < threshold)
    {
        index = (index + 1) % 4;
        target = KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[index];
    }

    bigBadWolfRunIndex[botGuid] = index;
    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);

    return MoveTo(bot->GetMapId(), target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Romulo and Julianne

// Keep the couple within 10% HP of each other
bool RomuloAndJulianneMarkTargetAction::Execute(Event event)
{
    Unit* target = nullptr;
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");
    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    if (!romulo || !julianne)
        return false;

    const uint8 maxPctDifference = 10;
    if (julianne->GetHealthPct() + maxPctDifference < romulo->GetHealthPct() || julianne->GetHealthPct() < 1.0f)
        target = romulo;
    else if (romulo->GetHealthPct() + maxPctDifference < julianne->GetHealthPct() || romulo->GetHealthPct() < 1.0f)
        target = julianne;

    MarkTargetWithSkull(bot, target);

    return false;
}

// The Wizard of Oz

// Mark targets with skull in the recommended kill order
bool WizardOfOzMarkTargetAction::Execute(Event event)
{
    Unit* dorothee = AI_VALUE2(Unit*, "find target", "dorothee");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");
    Unit* target = GetFirstAliveUnit({dorothee, tito, roar, strawman, tinhead, crone});
    if (target)
        MarkTargetWithSkull(bot, target);

    return false;
}

// Mages spam Scorch on Strawman to disorient him
bool WizardOfOzScorchStrawmanAction::Execute(Event event)
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    if (strawman && botAI->CanCastSpell("scorch", strawman))
        return botAI->CastSpell("scorch", strawman);

    return false;
}

// The Curator

// Prioritize destroying Astral Flares
bool TheCuratorMarkAstralFlareAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "astral flare");
    if (!target)
        return false;

    MarkTargetWithSkull(bot, target);

    return false;
}

// Tank the boss in the center of the hallway near the Guardian's Library
// Main tank and Assist tank will attack the boss; others will focus on Astral Flares
bool TheCuratorPositionBossAction::Execute(Event event)
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return false;

    if (bot->GetVictim() != curator)
        return Attack(curator);

    if (curator->GetVictim() == bot)
    {
        const float maxDistance = 3.0f;
        float distanceToBossPosition = curator->GetExactDist2d(KARAZHAN_THE_CURATOR_BOSS_POSITION);
        
        if (distanceToBossPosition > maxDistance)
        {
            float dX = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionX() - curator->GetPositionX();
            float dY = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionY() - curator->GetPositionY();
            float mX = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionX() + (dX / distanceToBossPosition) * maxDistance;
            float mY = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionY() + (dY / distanceToBossPosition) * maxDistance;
            return MoveTo(bot->GetMapId(), mX, mY, bot->GetPositionZ(), false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Spread out ranged DPS to avoid Arcing Sear damage
bool TheCuratorSpreadRangedAction::Execute(Event event)
{
    const float minDistance = 5.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);

    if (nearestPlayer)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return FleePosition(nearestPlayer->GetPosition(), minDistance);
    }

    return false;
}

// Terestian Illhoof

// Prioritize (1) Demon Chains, (2) Kil'rek, (3) Illhoof
bool TerestianIllhoofMarkTargetAction::Execute(Event event)
{
    Unit* illhoof = AI_VALUE2(Unit*, "find target", "terestian illhoof");
    if (!illhoof)
        return false;

    Unit* target = GetFirstAliveUnitByEntry(botAI, NPC_DEMON_CHAINS);
    if (!target || !target->IsAlive()) 
    {
        target = GetFirstAliveUnitByEntry(botAI, NPC_KILREK);
        if (!target || !target->IsAlive()) 
            target = illhoof;
    }
    MarkTargetWithSkull(bot, target);

    return false;
}

// Shade of Aran

// Run to the edge of the room to avoid Arcane Explosion
bool ShadeOfAranArcaneExplosionRunAwayAction::Execute(Event event)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    const float safeDistance = 20.0f;
    float distance = bot->GetDistance2d(aran);
    if (distance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveAway(aran, safeDistance - distance);
    }

    return false;
}

// I will not move when Flame Wreath is cast or the raid blows up
bool ShadeOfAranFlameWreathStopMovementAction::Execute(Event event)
{
    AI_VALUE(LastMovement&, "last movement").Set(nullptr);
    bot->GetMotionMaster()->Clear();
    if (bot->isMoving())
        bot->StopMoving();

    return true;
}

// Mark Conjured Elementals with skull so DPS can burn them down
bool ShadeOfAranMarkConjuredElementalAction::Execute(Event event)
{
    Unit* target = GetFirstAliveUnitByEntry(botAI, NPC_CONJURED_ELEMENTAL);

    if (target)
        MarkTargetWithSkull(bot, target);

    return false;
}


// Spread out ranged to avoid Blizzard
bool ShadeOfAranSpreadRangedAction::Execute(Event event)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    const float maxBossDistance = 12.0f;
    float bossDistance = bot->GetExactDist2d(aran);
    if (bossDistance > maxBossDistance)
    {
        float dX = bot->GetPositionX() - aran->GetPositionX();
        float dY = bot->GetPositionY() - aran->GetPositionY();
        float length = std::sqrt(dX * dX + dY * dY);
        dX /= length;
        dY /= length;
        float tX = aran->GetPositionX() + dX * maxBossDistance;
        float tY = aran->GetPositionY() + dY * maxBossDistance;
        return MoveTo(bot->GetMapId(), tX, tY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    const float minDistance = 5.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);
    if (nearestPlayer)
        return FleePosition(nearestPlayer->GetPosition(), minDistance);

    return false;
}

// Netherspite

// One tank bot per phase will dance in and out of the red beam (5 seconds in, 5 seconds out)
// Tank bots will ignore void zones--their positioning is too important
bool NetherspiteBlockRedBeamAction::Execute(Event event)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);
    if (!netherspite || !redPortal)
        return false;

    static std::map<ObjectGuid, bool> wasBlockingRedBeam;
    ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    bool isBlockingNow = (bot == redBlocker);
    bool wasBlocking = wasBlockingRedBeam[botGuid];

    Position beamPos = GetPositionOnBeam(netherspite, redPortal, 18.0f);

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> ph;
            ph["%player"] = bot->GetName();
            std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
                "netherspite_beam_blocking_red", "%player is moving to block the red beam!", ph);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        wasBlockingRedBeam[botGuid] = true;

        const uint8 intervalSecs = 5;
        if (time(nullptr) - redBeamMoveTimer[botGuid] >= intervalSecs)
        {
            lastBeamMoveSideways[botGuid] = !lastBeamMoveSideways[botGuid];
            redBeamMoveTimer[botGuid] = time(nullptr);
        }
        if (!lastBeamMoveSideways[botGuid]) 
            return MoveTo(bot->GetMapId(), beamPos.GetPositionX(), beamPos.GetPositionY(), beamPos.GetPositionZ(), 
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        else 
        {
            float bx = netherspite->GetPositionX();
            float by = netherspite->GetPositionY();
            float px = redPortal->GetPositionX();
            float py = redPortal->GetPositionY();
            float dx = px - bx;
            float dy = py - by;
            float length = sqrt(dx*dx + dy*dy);
            if (length == 0.0f)
                return false;

            dx /= length;
            dy /= length;
            float perpDx = -dy;
            float perpDy = dx;
            float sideX = beamPos.GetPositionX() + perpDx * 3.0f;
            float sideY = beamPos.GetPositionY() + perpDy * 3.0f;
            float sideZ = beamPos.GetPositionZ();

            return MoveTo(bot->GetMapId(), sideX, sideY, sideZ, false, false, false, false, 
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    wasBlockingRedBeam[botGuid] = false;
    return false;
}

// Two non-Rogue/Warrior DPS bots will block the blue beam for each phase (swap at 25 debuff stacks)
// When avoiding void zones, blocking bots will move along the beam to continue blocking
bool NetherspiteBlockBlueBeamAction::Execute(Event event)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);
    if (!netherspite || !bluePortal)
        return false;

    static std::map<ObjectGuid, bool> wasBlockingBlueBeam;
    ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    bool isBlockingNow = (bot == blueBlocker);
    bool wasBlocking = wasBlockingBlueBeam[botGuid];

    if (wasBlocking && !isBlockingNow)
    {
        std::map<std::string, std::string> ph;
        ph["%player"] = bot->GetName();
        std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
            "netherspite_beam_leaving_blue", "%player is leaving the blue beam--next blocker up!", ph);
        bot->Yell(text, LANG_UNIVERSAL);
        wasBlockingBlueBeam[botGuid] = false;
    }

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> ph;
            ph["%player"] = bot->GetName();
            std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
                "netherspite_beam_blocking_blue", "%player is moving to block the blue beam!", ph);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        wasBlockingBlueBeam[botGuid] = true;

        float idealDistance = botAI->IsRanged(bot) ? 25.0f : 18.0f;
        std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);
        float bx = netherspite->GetPositionX();
        float by = netherspite->GetPositionY();
        float bz = netherspite->GetPositionZ();
        float px = bluePortal->GetPositionX();
        float py = bluePortal->GetPositionY();
        float dx = px - bx;
        float dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        if (length == 0.0f)
            return false;

        dx /= length;
        dy /= length;
        float bestDist = 150.0f;
        Position bestPos;
        bool found = false;

        for (float dist = 18.0f; dist <= 30.0f; dist += 0.5f) 
        {
            float candidateX = bx + dx * dist;
            float candidateY = by + dy * dist;
            float candidateZ = bz;
            bool outsideAllVoidZones = true;

            float minSearchDist = botAI->IsRanged(bot) ? 20.0f : 15.0f;
            float maxSearchDist = botAI->IsRanged(bot) ? 30.0f : 25.0f;

            for (Unit* voidZone : voidZones) 
            {
                float voidZoneDist = sqrt(pow(candidateX - voidZone->GetPositionX(), 2) + 
                                          pow(candidateY - voidZone->GetPositionY(), 2));
                if (voidZoneDist < 4.0f) 
                {
                    outsideAllVoidZones = false;
                    break;
                }
            }
            if (!outsideAllVoidZones)
                continue;

            float distToIdeal = fabs(dist - idealDistance);
            if (!found || distToIdeal < bestDist) 
            {
                bestDist = distToIdeal;
                bestPos = Position(candidateX, candidateY, candidateZ);
                found = true;
            }
        }

        if (found) 
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }

    wasBlockingBlueBeam[botGuid] = false;
    return false;
}

// Two healer bots will block the green beam for each phase (swap at 25 debuff stacks)
// OR one rogue or DPS warrior bot will block the green beam for an entire phase (if they begin the phase as the blocker)
// When avoiding void zones, blocking bots will move along the beam to continue blocking
bool NetherspiteBlockGreenBeamAction::Execute(Event event)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);
    if (!netherspite || !greenPortal)
        return false;

    static std::map<ObjectGuid, bool> wasBlockingGreenBeam;
    ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    bool isBlockingNow = (bot == greenBlocker);
    bool wasBlocking = wasBlockingGreenBeam[botGuid];

    if (wasBlocking && !isBlockingNow)
    {
        std::map<std::string, std::string> ph;
        ph["%player"] = bot->GetName();
        std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
            "netherspite_beam_leaving_green", "%player is leaving the green beam--next blocker up!", ph);
        bot->Yell(text, LANG_UNIVERSAL);
        wasBlockingGreenBeam[botGuid] = false;
    }

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> ph;
            ph["%player"] = bot->GetName();
            std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
                "netherspite_beam_blocking_green", "%player is moving to block the green beam!", ph);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        wasBlockingGreenBeam[botGuid] = true;

        std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);
        float bx = netherspite->GetPositionX();
        float by = netherspite->GetPositionY();
        float bz = netherspite->GetPositionZ();
        float px = greenPortal->GetPositionX();
        float py = greenPortal->GetPositionY();
        float dx = px - bx;
        float dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        if (length == 0.0f)
            return false;

        dx /= length;
        dy /= length;
        float bestDist = 150.0f;
        Position bestPos;
        bool found = false;

        for (float dist = 18.0f; dist <= 30.0f; dist += 0.5f) 
        {
            float candidateX = bx + dx * dist;
            float candidateY = by + dy * dist;
            float candidateZ = bz;
            bool outsideAllVoidZones = true;

            for (Unit* voidZone : voidZones)
            {
                float voidZoneDist = sqrt(pow(candidateX - voidZone->GetPositionX(), 2) + 
                                          pow(candidateY - voidZone->GetPositionY(), 2));
                if (voidZoneDist < 4.0f) 
                {
                    outsideAllVoidZones = false;
                    break;
                }
            }
            if (!outsideAllVoidZones)
                continue;

            float distToIdeal = fabs(dist - 18.0f);
            if (!found || distToIdeal < bestDist)
            {
                bestDist = distToIdeal;
                bestPos = Position(candidateX, candidateY, candidateZ);
                found = true;
            }
        }

        if (found) 
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(), 
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }

    wasBlockingGreenBeam[botGuid] = false;
    return false;
}

// All bots not currently blocking a beam will avoid beams and void zones
bool NetherspiteAvoidBeamAndVoidZoneAction::Execute(Event event)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);
    bool nearVoidZone = false;
    for (Unit* vz : voidZones)
    {
        if (bot->GetExactDist2d(vz) < 4.0f)
        {
            nearVoidZone = true;
            break;
        }
    }

    struct BeamAvoid { Unit* portal; float minDist, maxDist; };
    std::vector<BeamAvoid> beams;
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);

    if (redPortal) 
    {
        float bx = netherspite->GetPositionX(), by = netherspite->GetPositionY();
        float px = redPortal->GetPositionX(), py = redPortal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        beams.push_back({redPortal, 0.0f, length});
    }

    if (bluePortal) 
    {
        float bx = netherspite->GetPositionX(), by = netherspite->GetPositionY();
        float px = bluePortal->GetPositionX(), py = bluePortal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        beams.push_back({bluePortal, 0.0f, length});
    }

    if (greenPortal) 
    {
        float bx = netherspite->GetPositionX(), by = netherspite->GetPositionY();
        float px = greenPortal->GetPositionX(), py = greenPortal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        beams.push_back({greenPortal, 0.0f, length});
    }
    bool nearBeam = false;

    for (const auto& beam : beams)
    {
        float bx = netherspite->GetPositionX(), by = netherspite->GetPositionY();
        float px = beam.portal->GetPositionX(), py = beam.portal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        if (length == 0.0f)
            continue;

        dx /= length; dy /= length;
        float botdx = bot->GetPositionX() - bx, botdy = bot->GetPositionY() - by;
        float t = (botdx * dx + botdy * dy);
        float beamX = bx + dx * t, beamY = by + dy * t;
        float distToBeam = sqrt(pow(bot->GetPositionX() - beamX, 2) + pow(bot->GetPositionY() - beamY, 2));
        if (distToBeam < 5.0f && t > beam.minDist && t < beam.maxDist)
        {
            nearBeam = true;
            break;
        }
    }
    if (!nearVoidZone && !nearBeam)
        return false;

    const float minMoveDist = 2.0f, maxSearchDist = 30.0f, stepAngle = M_PI/18.0f, stepDist = 0.5f;
    float netherspiteZ = netherspite->GetPositionZ();
    Position bestCandidate;
    float bestDist = 0.0f;
    bool found = false;
    for (float angle = 0; angle < 2 * M_PI; angle += stepAngle)
    {
        for (float dist = 2.0f; dist <= maxSearchDist; dist += stepDist)
        {
            float cx = bot->GetPositionX() + cos(angle) * dist;
            float cy = bot->GetPositionY() + sin(angle) * dist;
            float cz = netherspiteZ;
            if (std::any_of(voidZones.begin(), voidZones.end(), [&](Unit* vz){ 
                return Position(cx, cy, cz).GetExactDist2d(vz) < 4.0f; }))
                continue;

            bool tooCloseToBeam = false;
            for (const auto& beam : beams)
            {
                float bx = netherspite->GetPositionX(), by = netherspite->GetPositionY();
                float px = beam.portal->GetPositionX(), py = beam.portal->GetPositionY();
                float dx = px - bx, dy = py - by;
                float length = sqrt(dx*dx + dy*dy);
                if (length == 0.0f) 
                    continue;

                dx /= length; dy /= length;
                float botdx = cx - bx, botdy = cy - by;
                float t = (botdx * dx + botdy * dy);
                float beamX = bx + dx * t, beamY = by + dy * t;
                float distToBeam = sqrt(pow(cx - beamX, 2) + pow(cy - beamY, 2));
                if (distToBeam < 5.0f && t > beam.minDist && t < beam.maxDist)
                {
                    tooCloseToBeam = true;
                    break;
                }
            }
            if (tooCloseToBeam)
                continue;

            float moveDist = sqrt(pow(cx - bot->GetPositionX(), 2) + pow(cy - bot->GetPositionY(), 2));
            if (moveDist < minMoveDist)
                continue;

            if (!found || moveDist < bestDist) 
            {
                bestCandidate = Position(cx, cy, cz);
                bestDist = moveDist;
                found = true;
            }
        }
    }

    if (found && IsSafePosition(bestCandidate.GetPositionX(), 
        bestCandidate.GetPositionY(), bestCandidate.GetPositionZ(), voidZones, 4.0f))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(bot->GetMapId(), bestCandidate.GetPositionX(), bestCandidate.GetPositionY(), 
                      bestCandidate.GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }
    
    return false;
}

bool NetherspiteBanishPhaseAvoidVoidZoneAction::Execute(Event event)
{
    std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);

    for (Unit* vz : voidZones)
    {
        if (vz->GetEntry() == NPC_VOID_ZONE && bot->GetExactDist2d(vz) < 4.0f)
            return FleePosition(vz->GetPosition(), 4.0f);
    }

    return false;
}

bool NetherspiteManageTimersAction::Execute(Event event)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    uint32 mapId = netherspite->GetMapId();
    ObjectGuid botGuid = bot->GetGUID();

    if (netherspite->HasAura(SPELL_NETHERSPITE_BANISHED) ||
        (netherspite->GetHealth() == netherspite->GetMaxHealth() &&
         !netherspite->HasAura(SPELL_GREEN_BEAM_HEAL)))
    {
        if (IsMapIDTimerManager(bot) && netherspiteDPSWaitTimer.count(mapId))
            netherspiteDPSWaitTimer.erase(mapId);

        if (botAI->IsTank(bot) && redBeamMoveTimer.count(botGuid))
        {
            redBeamMoveTimer.erase(botGuid);
            lastBeamMoveSideways.erase(botGuid);
        }
    }

    if (!netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
    {
        if (IsMapIDTimerManager(bot) && !netherspiteDPSWaitTimer.count(mapId))
            netherspiteDPSWaitTimer[mapId] = time(nullptr);

        if (botAI->IsTank(bot) && bot->HasAura(SPELL_RED_BEAM_DEBUFF) && 
            !redBeamMoveTimer.count(botGuid))
        {
            redBeamMoveTimer[botGuid] = time(nullptr);
            lastBeamMoveSideways[botGuid] = false;
        }
    }

    return false;
}

bool PrinceMalchezaarEnfeebledAvoidHazardAction::Execute(Event event)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar || !bot->HasAura(SPELL_ENFEEBLE))
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float minSafeBossDistance = 34.0f;
    const float maxSafeBossDistance = 60.0f;
    const float safeInfernalDistance = 23.0f;
    const float stepSize = 0.5f;
    const uint8 numAngles = 64;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();
    float malchezaarX = malchezaar->GetPositionX();
    float malchezaarY = malchezaar->GetPositionY();
    float malchezaarZ = malchezaar->GetPositionZ();
    float bestMoveDist = std::numeric_limits<float>::max();
    float bestDestX = 0.0f, bestDestY = 0.0f, bestDestZ = bz;
    bool found = false;

    for (int i = 0; i < numAngles; ++i)
    {
        float angle = (2 * M_PI * i) / numAngles;
        float dx = cos(angle);
        float dy = sin(angle);

        for (float dist = minSafeBossDistance; dist <= maxSafeBossDistance; dist += stepSize)
        {
            float x = malchezaarX + dx * dist;
            float y = malchezaarY + dy * dist;
            float destZ = malchezaarZ;
            float destX = x, destY = y, destZ2 = destZ;
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, destX, destY, destZ2, true))
                continue;

            float distFromBoss = sqrt(pow(destX - malchezaarX, 2) + pow(destY - malchezaarY, 2));
            if (distFromBoss < minSafeBossDistance)
                continue;

            bool pathSafe = IsStraightPathSafe(Position(bx, by, bz), Position(destX, destY, destZ2),
                                               infernals, safeInfernalDistance, stepSize);
            float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));

            if (pathSafe && moveDist < bestMoveDist)
            {
                bestMoveDist = moveDist;
                bestDestX = destX;
                bestDestY = destY;
                bestDestZ = destZ2;
                found = true;
            }
        }
    }

    if (found)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(bot->GetMapId(), bestDestX, bestDestY, bestDestZ, false, false, false, false, 
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool PrinceMalchezaarNonTankAvoidInfernalAction::Execute(Event event)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar || bot->HasAura(SPELL_ENFEEBLE))
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float safeInfernalDistance = 23.0f;
    const float stepSize = 0.5f;
    const uint8 numAngles = 64;
    const float maxSafeBossDistance = 60.0f;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();
    float malchezaarX = malchezaar->GetPositionX();
    float malchezaarY = malchezaar->GetPositionY();
    float malchezaarZ = malchezaar->GetPositionZ();

    bool nearInfernal = false;
    for (Unit* infernal : infernals)
    {
        float infernalDist = sqrt(pow(bx - infernal->GetPositionX(), 2) + pow(by - infernal->GetPositionY(), 2));
        if (infernalDist < safeInfernalDistance)
        {
            nearInfernal = true;
            break;
        }
    }

    float bestMoveDist = std::numeric_limits<float>::max();
    float bestDestX = bx, bestDestY = by, bestDestZ = bz;
    bool found = false;

    if (nearInfernal)
    {
        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2 * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);

            for (float dist = stepSize; dist <= maxSafeBossDistance; dist += stepSize)
            {
                float x = malchezaarX + dx * dist;
                float y = malchezaarY + dy * dist;
                float destZ = malchezaarZ;
                float destX = x, destY = y, destZ2 = destZ;
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, malchezaarX, malchezaarY, malchezaarZ, 
                    destX, destY, destZ2, true))
                    continue;

                bool destSafe = true;
                for (Unit* infernal : infernals)
                {
                    float infernalDist = sqrt(pow(destX - infernal->GetPositionX(), 2) + pow(destY - infernal->GetPositionY(), 2));
                    if (infernalDist < safeInfernalDistance)
                    {
                        destSafe = false;
                        break;
                    }
                }
                if (!destSafe)
                    continue;

                float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));

                if (moveDist < bestMoveDist)
                {
                    bestMoveDist = moveDist;
                    bestDestX = destX;
                    bestDestY = destY;
                    bestDestZ = destZ2;
                    found = true;
                }
            }
        }

        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), bestDestX, bestDestY, bestDestZ, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Avoid infernals and run away from the boss when Enfeebled
/* bool PrinceMalchezaarNonTankAvoidHazardAction::Execute(Event event)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float minSafeBossDistance = 34.0f;
    const float maxSafeBossDistance = 60.0f;
    const float safeInfernalDistance = 23.0f;
    const float stepSize = 0.5f;
    const uint8 numAngles = 64;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();
    float malchezaarX = malchezaar->GetPositionX();
    float malchezaarY = malchezaar->GetPositionY();
    float malchezaarZ = malchezaar->GetPositionZ();
    float bestMoveDist = std::numeric_limits<float>::max();
    float bestDestX = 0.0f, bestDestY = 0.0f, bestDestZ = bz;
    bool found = false;

    if (bot->HasAura(SPELL_ENFEEBLE))
    {
        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2 * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);
            for (float dist = minSafeBossDistance; dist <= maxSafeBossDistance; dist += stepSize)
            {
                float x = malchezaarX + dx * dist;
                float y = malchezaarY + dy * dist;
                float destZ = malchezaarZ;
                float destX = x, destY = y, destZ2 = destZ;
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, destX, destY, destZ2, true))
                    continue;

                float distFromBoss = sqrt(pow(destX - malchezaarX, 2) + pow(destY - malchezaarY, 2));
                if (distFromBoss < minSafeBossDistance)
                    continue;

                bool pathSafe = IsStraightPathSafe(Position(bx, by, bz), Position(destX, destY, destZ2),
                                                   infernals, safeInfernalDistance, stepSize);
                float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));
                if (pathSafe && moveDist < bestMoveDist)
                {
                    bestMoveDist = moveDist;
                    bestDestX = destX;
                    bestDestY = destY;
                    bestDestZ = destZ2;
                    found = true;
                }
            }
        }
        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), bestDestX, bestDestY, bestDestZ, false, false, false, false, 
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }

    if (!bot->HasAura(SPELL_ENFEEBLE))
    {
        bool nearInfernal = false;
        for (Unit* infernal : infernals)
        {
            float infernalDist = sqrt(pow(bx - infernal->GetPositionX(), 2) + pow(by - infernal->GetPositionY(), 2));
            if (infernalDist < safeInfernalDistance)
            {
                nearInfernal = true;
                break;
            }
        }
        if (nearInfernal)
        {
            float bestMoveDist = std::numeric_limits<float>::max();
            float bestDestX = bx, bestDestY = by, bestDestZ = bz;
            bool found = false;
            for (int i = 0; i < numAngles; ++i)
            {
                float angle = (2 * M_PI * i) / numAngles;
                float dx = cos(angle);
                float dy = sin(angle);
                for (float dist = stepSize; dist <= maxSafeBossDistance; dist += stepSize)
                {
                    float x = malchezaarX + dx * dist;
                    float y = malchezaarY + dy * dist;
                    float destZ = malchezaarZ;
                    float destX = x, destY = y, destZ2 = destZ;
                    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, malchezaarX, malchezaarY, malchezaarZ, 
                        destX, destY, destZ2, true))
                        continue;

                    bool destSafe = true;
                    for (Unit* infernal : infernals)
                    {
                        float infernalDist = sqrt(pow(destX - infernal->GetPositionX(), 2) + pow(destY - infernal->GetPositionY(), 2));
                        if (infernalDist < safeInfernalDistance)
                        {
                            destSafe = false;
                            break;
                        }
                    }
                    if (!destSafe)
                        continue;

                    float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));
                    if (moveDist < bestMoveDist)
                    {
                        bestMoveDist = moveDist;
                        bestDestX = destX;
                        bestDestY = destY;
                        bestDestZ = destZ2;
                        found = true;
                    }
                }
            }
            if (found)
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(false);
                return MoveTo(bot->GetMapId(), bestDestX, bestDestY, bestDestZ, false, false, false, false, 
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    return false;
} */

bool PrinceMalchezaarMainTankMovementAction::Execute(Event event)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    if (bot->GetVictim() != malchezaar)
        return Attack(malchezaar);

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float safeInfernalDistance = 30.0f;
    const float stepSize = 0.5f;
    const uint8 numAngles = 64;
    const float maxSampleDist = 60.0f;
    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();

    bool nearInfernal = false;
    for (Unit* infernal : infernals)
    {
        float infernalDist = sqrt(pow(bx - infernal->GetPositionX(), 2) + pow(by - infernal->GetPositionY(), 2));
        if (infernalDist < safeInfernalDistance)
        {
            nearInfernal = true;
            break;
        }
    }

    float bestMoveDist = std::numeric_limits<float>::max();
    float bestDestX = bx, bestDestY = by, bestDestZ = bz;
    bool found = false;

    if (nearInfernal)
    {
        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2 * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);

            for (float dist = stepSize; dist <= maxSampleDist; dist += stepSize)
            {
                float x = bx + dx * dist;
                float y = by + dy * dist;
                float z = bz;
                float destX = x, destY = y, destZ = z;
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, destX, destY, destZ, true))
                    continue;

                bool destSafe = true;
                for (Unit* infernal : infernals)
                {
                    float infernalDist = sqrt(pow(destX - infernal->GetPositionX(), 2) + pow(destY - infernal->GetPositionY(), 2));
                    if (infernalDist < safeInfernalDistance)
                    {
                        destSafe = false;
                        break;
                    }
                }
                if (!destSafe)
                    continue;

                bool pathSafe = IsStraightPathSafe(Position(bx, by, bz), Position(destX, destY, destZ),
                     infernals, safeInfernalDistance, stepSize);
                float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));

                if (pathSafe && moveDist < bestMoveDist)
                {
                    bestMoveDist = moveDist;
                    bestDestX = destX;
                    bestDestY = destY;
                    bestDestZ = destZ;
                    found = true;
                }
            }
        }

        if (!found)
        {
            for (int i = 0; i < numAngles; ++i)
            {
                float angle = (2 * M_PI * i) / numAngles;
                float dx = cos(angle);
                float dy = sin(angle);

                for (float dist = stepSize; dist <= maxSampleDist; dist += stepSize)
                {
                    float x = bx + dx * dist;
                    float y = by + dy * dist;
                    float z = bz;
                    float destX = x, destY = y, destZ = z;
                    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, destX, destY, destZ, true))
                        continue;

                    bool destSafe = true;
                    for (Unit* infernal : infernals)
                    {
                        float infernalDist = sqrt(pow(destX - infernal->GetPositionX(), 2) + pow(destY - infernal->GetPositionY(), 2));
                        if (infernalDist < safeInfernalDistance)
                        {
                            destSafe = false;
                            break;
                        }
                    }

                    float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));
                    if (destSafe && moveDist < bestMoveDist)
                    {
                        bestMoveDist = moveDist;
                        bestDestX = destX;
                        bestDestY = destY;
                        bestDestZ = destZ;
                        found = true;
                    }
                }
            }
        }

        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), bestDestX, bestDestY, bestDestZ, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// The tank position is near the urn; it's best to pull from Northwest of the urn
// The tank moves Nightbane into position in two steps to try to get Nightbane to face sideways to the raid
bool NightbaneGroundPhasePositionBossAction::Execute(Event event)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    MarkTargetWithSkull(bot, nightbane);

    if (bot->GetVictim() != nightbane)
        return Attack(nightbane);

    ObjectGuid botGuid = bot->GetGUID();
    uint8 step = nightbaneTankStep.count(botGuid) ? nightbaneTankStep[botGuid] : 0;

    if (nightbane->GetVictim() == bot)
    {
        const Position tankPositions[2] = 
        {
            KARAZHAN_NIGHTBANE_TRANSITION_BOSS_POSITION,
            KARAZHAN_NIGHTBANE_FINAL_BOSS_POSITION
        };
        const Position& targetPos = tankPositions[step];
        const float maxDistance = 0.5f;
        float distanceToTarget = bot->GetExactDist2d(targetPos);

        if (distanceToTarget > maxDistance)
            return MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, true);

        if (step == 0 && distanceToTarget <= maxDistance)
            nightbaneTankStep[botGuid] = 1;

        float orientation = atan2(nightbane->GetPositionY() - bot->GetPositionY(),
                                  nightbane->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }
    else if (!bot->IsWithinMeleeRange(nightbane))
        return MoveTo(nightbane->GetMapId(), nightbane->GetPositionX(), nightbane->GetPositionY(), 
                      nightbane->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);

    return false;
}

// Ranged bots rotate between 3 positions to avoid standing in Charred Earth, which lasts for
// 30s and has a minimum cooldown of 18s (so there can be 3 active at once)
// Ranged positions are North of the urn, near the door to the tower
bool NightbaneGroundPhaseRotateRangedPositionsAction::Execute(Event event)
{
    ObjectGuid botGuid = bot->GetGUID();
    uint8 index = nightbaneRangedStep.count(botGuid) ? nightbaneRangedStep[botGuid] : 0;

    const Position rangedPositions[3] = 
    {
        KARAZHAN_NIGHTBANE_RANGED_POSITION_1,
        KARAZHAN_NIGHTBANE_RANGED_POSITION_2,
        KARAZHAN_NIGHTBANE_RANGED_POSITION_3
    };
    const Position& targetPos = rangedPositions[index];
    const float maxDistance = 2.0f;
    float distanceToTarget = bot->GetExactDist2d(targetPos);

    if (distanceToTarget <= maxDistance && 
        bot->HasAura(SPELL_CHARRED_EARTH) && !bot->HasAura(SPELL_BELLOWING_ROAR))
    {
        index = (index + 1) % 3;
        nightbaneRangedStep[botGuid] = index;
        const Position& newTargetPos = rangedPositions[index];
        float newDistanceToTarget = bot->GetExactDist2d(newTargetPos);
        if (newDistanceToTarget > maxDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), newTargetPos.GetPositionX(), newTargetPos.GetPositionY(), newTargetPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        return false;
    }

    if (distanceToTarget > maxDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool NightbaneCastFearWardOnMainTankAction::Execute(Event event)
{
    Player* mainTank = nullptr;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

// Put pets on passive during the flight phase so they don't try to chase Nightbane off the map
bool NightbaneControlPetAggressionAction::Execute(Event event)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    Pet* pet = bot->GetPet();
    if (!nightbane || !pet)
        return false;

    if (!nightbane->IsFlying() && pet->GetReactState() == REACT_PASSIVE)
        pet->SetReactState(REACT_DEFENSIVE);

    if (nightbane->IsFlying() && pet->GetReactState() != REACT_PASSIVE)
    {
        pet->AttackStop();
        pet->SetReactState(REACT_PASSIVE);
    }

    return false;
}

// 1. Stack at the "Flight Stack Position" near Nightbane so he doesn't use Fireball Barrage
// 2. Once Rain of Bones hits, the whole party moves to a new stack position
// This action lasts for the first 35 seconds of the flight phase, after which Nightbane
// transitions to land--the player will need to lead the bots over near the ground phase position
bool NightbaneFlightPhaseMovementAction::Execute(Event event)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || !nightbane->IsFlying())
        return false;

    MarkTargetWithMoon(bot, nightbane);

    Unit* botTarget = botAI->GetUnit(bot->GetTarget());
    if (botTarget && botTarget == nightbane)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
    }

    ObjectGuid botGuid = bot->GetGUID();
    bool hasRainOfBones = bot->HasAura(SPELL_RAIN_OF_BONES);

    if (hasRainOfBones)
        nightbaneRainOfBonesHit[botGuid] = true;

    float destX, destY, destZ;
    if (nightbaneRainOfBonesHit[botGuid])
    {
        destX = KARAZHAN_NIGHTBANE_RAIN_OF_BONES_POSITION.GetPositionX();
        destY = KARAZHAN_NIGHTBANE_RAIN_OF_BONES_POSITION.GetPositionY();
        destZ = KARAZHAN_NIGHTBANE_RAIN_OF_BONES_POSITION.GetPositionZ();
    }
    else
    {
        destX = KARAZHAN_NIGHTBANE_FLIGHT_STACK_POSITION.GetPositionX();
        destY = KARAZHAN_NIGHTBANE_FLIGHT_STACK_POSITION.GetPositionY();
        destZ = KARAZHAN_NIGHTBANE_FLIGHT_STACK_POSITION.GetPositionZ();
    }

    if (bot->GetExactDist2d(destX, destY) > 2.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool NightbaneManageTimersAndTrackersAction::Execute(Event event)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    uint32 mapId = nightbane->GetMapId();
    ObjectGuid botGuid = bot->GetGUID();

    // Erase DPS wait timer and tank and ranged position tracking on encounter reset or flight
    if (nightbane->IsFlying() || nightbane->GetHealth() == nightbane->GetMaxHealth())
    {
        if (botAI->IsMainTank(bot) && nightbaneTankStep.count(botGuid))
            nightbaneTankStep.erase(botGuid);

        if (botAI->IsRanged(bot) && nightbaneRangedStep.count(botGuid))
            nightbaneRangedStep.erase(botGuid);

        if (IsMapIDTimerManager(bot) && nightbaneDPSWaitTimer.count(mapId))
            nightbaneDPSWaitTimer.erase(mapId);
    }

    // Erase flight phase timer and Rain of Bones tracker on ground phase and start DPS wait timer
    if (!nightbane->IsFlying())
    {
        if (IsMapIDTimerManager(bot) && nightbaneFlightPhaseStartTimer.count(mapId))
            nightbaneFlightPhaseStartTimer.erase(mapId);

        if (nightbaneRainOfBonesHit.count(botGuid))
            nightbaneRainOfBonesHit.erase(botGuid);

        if (IsMapIDTimerManager(bot) && !nightbaneDPSWaitTimer.count(mapId))
            nightbaneDPSWaitTimer[mapId] = time(nullptr);
    }

    // Start flight phase timer at beginning of flight phase
    if (nightbane->IsFlying() && IsMapIDTimerManager(bot) &&
        !nightbaneFlightPhaseStartTimer.count(mapId))
        nightbaneFlightPhaseStartTimer[mapId] = time(nullptr);

    return false;
}
