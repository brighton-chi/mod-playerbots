#include "RaidKarazhanMultipliers.h"
#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "GenericActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

float KarazhanAttumenTheHuntsmanMultiplier::GetValue(Action* action)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* boss = karazhanHelper.GetFirstAliveUnitByEntry(ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!boss)
    {
        return 1.0f;
    }

    if (!botAI->IsMainTank(bot) && boss->GetVictim() != bot &&
        (dynamic_cast<CombatFormationMoveAction*>(action) || 
         dynamic_cast<FleeAction*>(action) || 
         dynamic_cast<CastBlinkBackAction*>(action) || 
         dynamic_cast<CastDisengageAction*>(action)))
    {
        return 0.0f;
    }

    Unit* victim = boss->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    if (!botAI->IsMainTank(bot) && victimPlayer && !botAI->IsMainTank(victimPlayer) && 
        (dynamic_cast<AttackAction*>(action) || (!botAI->IsHeal(victimPlayer) && dynamic_cast<CastSpellAction*>(action))))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KarazhanTheCuratorMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");
    if (!boss)
    {
        return 1.0f;
    }

    if ((botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0)) &&
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KarazhanShadeOfAranMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!boss)
    {
        return 1.0f;
    }

    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(ARCANE_EXPLOSION))
    {
        if (IsChargeAction(action))
        {
            return 0.0f;
        }

        if (dynamic_cast<MovementAction*>(action))
        {
            const float safeDistance = 20.0f;
            if (bot->GetDistance2d(boss) >= safeDistance)
            {
                return 0.0f;
            }
        }
    }

    bool flameWreathActive = boss->HasAura(FLAME_WREATH_CAST);
    if (!flameWreathActive && bot->GetGroup())
    {
        for (GroupReference* itr = bot->GetGroup()->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->HasAura(FLAME_WREATH_AURA))
            {
                flameWreathActive = true;
                break;
            }
        }
    }
    if (flameWreathActive)
    {
        if (dynamic_cast<MovementAction*>(action) || IsChargeAction(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float KarazhanNetherspiteBlueAndGreenBeamMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 0.0f;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    auto [redBlocker /*unused*/, greenBlocker, blueBlocker] = karazhanHelper.GetCurrentBeamBlockers();
    bool isBlocker = (bot == greenBlocker || bot == blueBlocker);
    if (isBlocker)
    {
        Unit* bluePortal = bot->FindNearestCreature(BLUE_PORTAL, 150.0f);
        Unit* greenPortal = bot->FindNearestCreature(GREEN_PORTAL, 150.0f);
        bool inBeam = false;
        for (Unit* portal : {bluePortal, greenPortal}) 
        {
            if (!portal)
            {
                continue;
            }
            float bx = boss->GetPositionX(), by = boss->GetPositionY();
            float px = portal->GetPositionX(), py = portal->GetPositionY();
            float dx = px - bx, dy = py - by;
            float length = sqrt(dx*dx + dy*dy);
            if (length == 0.0f)
            {
                continue;
            }
            dx /= length; dy /= length;
            float botdx = bot->GetPositionX() - bx, botdy = bot->GetPositionY() - by;
            float t = (botdx * dx + botdy * dy);
            float beamX = bx + dx * t, beamY = by + dy * t;
            float distToBeam = sqrt(pow(bot->GetPositionX() - beamX, 2) + pow(bot->GetPositionY() - beamY, 2));
            if (distToBeam < 0.3f && t > 0.0f && t < length) 
            {
                inBeam = true;
                break;
            }
        }
        if (inBeam)
        {
            std::vector<Unit*> voidZones = karazhanHelper.GetAllVoidZones();
            bool inVoidZone = false;
            for (Unit* vz : voidZones) 
            {
                if (bot->GetExactDist2d(vz) < 4.0f)
                {
                    inVoidZone = true;
                    break;
                }
            }
            if (!inVoidZone)
            {
                if (dynamic_cast<MovementAction*>(action) || IsChargeAction(action))
                {
                    return 0.0f;
                }
            }
        }
    }

    return 1.0f;
}

float KarazhanNetherspiteRedBeamMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    if (dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    auto [redBlocker, greenBlocker /*unused*/, blueBlocker /*unused*/] = karazhanHelper.GetCurrentBeamBlockers();
    static std::map<ObjectGuid, uint32> beamMoveTimes;
    static std::map<ObjectGuid, bool> lastBeamMoveSideways;
    ObjectGuid botGuid = bot->GetGUID();
    Unit* redPortal = bot->FindNearestCreature(RED_PORTAL, 150.0f);
    if (bot == redBlocker && boss && redPortal)
    {
        Position blockingPos = karazhanHelper.GetPositionOnBeam(boss, redPortal, 18.0f);
        float bx = boss->GetPositionX();
        float by = boss->GetPositionY();
        float px = redPortal->GetPositionX();
        float py = redPortal->GetPositionY();
        float dx = px - bx;
        float dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        if (length != 0.0f)
        {
            dx /= length;
            dy /= length;
            float perpDx = -dy;
            float perpDy = dx;
            Position sidewaysPos(blockingPos.GetPositionX() + perpDx * 3.0f,
                                 blockingPos.GetPositionY() + perpDy * 3.0f,
                                 blockingPos.GetPositionZ());

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
            Position targetPos = lastBeamMoveSideways[botGuid] ? sidewaysPos : blockingPos;
            float distToTarget = bot->GetExactDist2d(targetPos.GetPositionX(), targetPos.GetPositionY());
            const float positionTolerance = 1.5f;
            if (distToTarget < positionTolerance)
            {
                if (dynamic_cast<MovementAction*>(action) || IsChargeAction(action))
                {
                    return 0.0f;
                }
            }
        }
    }
    
    return 1.0f;
}

float KarazhanNetherspiteWaitForDPSMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    Unit* victim = boss->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    if (!botAI->IsTank(bot) && victimPlayer && !botAI->IsTank(victimPlayer) && 
        !boss->HasAura(NETHERSPITE_BANISHED) &&
        (dynamic_cast<AttackAction*>(action) || (!botAI->IsHeal(victimPlayer) && dynamic_cast<CastSpellAction*>(action))))
    {
        return 0.0f;
    }

    /* bool tankHasRedBeam = false;
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = bot->GetGroup()->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
            {
                continue;
            }
            if (botAI->IsTank(member) && member->HasAura(RED_BEAM_DEBUFF)
            {
                tankHasRedBeam = true;
                break;
            }
        }
    }

    if (!boss->HasAura(NETHERSPITE_BANISHED) && !tankHasRedBeam)
    {
        if (!botAI->IsTank(bot) && dynamic_cast<AttackAction*>(action))
        {
            return 0.0f;
        }
    } */

    return 1.0f;
}

float KarazhanPrinceMalchezaarMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    if (dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsMelee(bot) && bot->HasAura(ENFEEBLE) &&
        !dynamic_cast<KarazhanPrinceMalchezaarNonTankAvoidHazardAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsRanged(bot) && bot->HasAura(ENFEEBLE) &&
        (dynamic_cast<MovementAction*>(action) && !dynamic_cast<KarazhanPrinceMalchezaarNonTankAvoidHazardAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KarazhanNightbaneMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    if (dynamic_cast<CastBlinkBackAction*>(action) || 
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastForceOfNatureAction*>(action) ||
        dynamic_cast<CastFeralSpiritAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastSummonWaterElementalAction*>(action) ||
        dynamic_cast<CastShadowfiendAction*>(action) ||
        dynamic_cast<PetAttackAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }

    Unit* victim = boss->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    if (!boss->IsFlying() && !botAI->IsMainTank(bot) && victimPlayer && !botAI->IsMainTank(victimPlayer) && 
        (dynamic_cast<AttackAction*>(action) || (!botAI->IsHeal(victimPlayer) && dynamic_cast<CastSpellAction*>(action))))
    {
        return 0.0f;
    }

    if ((botAI->IsMainTank(bot) || botAI->IsRanged(bot)) && 
        dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action))
    {
        // Disable for all bots during flight phase
        if (boss->IsFlying())
        {
            return 0.0f;
        }
        // Disable for all bots except non-main tank melee during ground phase
        /* if (!botAI->IsMelee(bot) || botAI->IsMainTank(bot))
        {
            return 0.0f;
        } */
    }

    /* if (botAI->IsMainTank(bot) && boss && boss->GetVictim() == bot && dynamic_cast<MovementAction*>(action))
    {
        const float positionThreshold = 1.0f;
        const float orientationLeeway = 30.0f * M_PI / 180.0f;

        float distanceToTankSpot = bot->GetExactDist2d(KARAZHAN_NIGHTBANE_FINAL_BOSS_POSITION.GetPositionX(),
                                                       KARAZHAN_NIGHTBANE_FINAL_BOSS_POSITION.GetPositionY());

        float desiredOrientation = atan2(boss->GetPositionY() - bot->GetPositionY(),
                                        boss->GetPositionX() - bot->GetPositionX());
        float currentOrientation = bot->GetOrientation();
        float delta = desiredOrientation - currentOrientation;
        while (delta > M_PI) delta -= 2 * M_PI;
        while (delta < -M_PI) delta += 2 * M_PI;
        float orientationDifference = fabs(delta);

        if (distanceToTankSpot < positionThreshold && orientationDifference < orientationLeeway)
        {
            return 0.0f;
        }
    } */

    return 1.0f;
}
