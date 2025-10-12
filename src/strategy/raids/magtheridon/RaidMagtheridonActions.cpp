#include "RaidMagtheridonActions.h"
#include "RaidMagtheridonHelpers.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Playerbots.h"

using namespace MagtheridonHelpers;

bool MagtheridonHellfireChannelerMainTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    if (channelerSquare && channelerSquare->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(squareIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerSquare->GetGUID())
            group->SetTargetIcon(squareIcon, bot->GetGUID(), channelerSquare->GetGUID());
    }
    if (channelerStar && channelerStar->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(starIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerStar->GetGUID())
            group->SetTargetIcon(starIcon, bot->GetGUID(), channelerStar->GetGUID());
    }
    if (channelerCircle && channelerCircle->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(circleIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerCircle->GetGUID())
            group->SetTargetIcon(circleIcon, bot->GetGUID(), channelerCircle->GetGUID());
    }

    if ((!channelerSquare || !channelerSquare->IsAlive()) &&
        (!channelerStar   || !channelerStar->IsAlive()) &&
        (!channelerCircle || !channelerCircle->IsAlive()))
    {
        const TankSpot& spot = MagtheridonTankSpots::WaitingForMagtheridon;
        if (!bot->IsWithinDist2d(spot.x, spot.y, 2.0f))
        {
            return MoveTo(bot->GetMapId(), spot.x, spot.y, spot.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        bot->SetFacingTo(spot.orientation);
        return true;
    }

    Creature* currentTarget = nullptr;
    std::string rtiName;
    if (channelerSquare && channelerSquare->IsAlive())
    {
        currentTarget = channelerSquare;
        rtiName = "square";
    }
    else if (channelerStar && channelerStar->IsAlive())
    {
        currentTarget = channelerStar;
        rtiName = "star";
    }
    else if (channelerCircle && channelerCircle->IsAlive())
    {
        currentTarget = channelerCircle;
        rtiName = "circle";
    }

    if (currentTarget &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
         botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != currentTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(currentTarget);
    }

    if (currentTarget && bot->GetVictim() != currentTarget)
    {
        Attack(currentTarget);

        if (!bot->IsWithinMeleeRange(currentTarget))
        {
            return MoveTo(currentTarget->GetMapId(), currentTarget->GetPositionX(), 
                          currentTarget->GetPositionY(), currentTarget->GetPositionZ());
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerNWChannelerTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channelerDiamond->GetGUID())
    {
        group->SetTargetIcon(diamondIcon, bot->GetGUID(), channelerDiamond->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != channelerDiamond)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerDiamond);
    }

    if (bot->GetVictim() != channelerDiamond)
    {
        Attack(channelerDiamond);
        
        if (!bot->IsWithinMeleeRange(channelerDiamond))
        {
            return MoveTo(channelerDiamond->GetMapId(), channelerDiamond->GetPositionX(),
                          channelerDiamond->GetPositionY(), channelerDiamond->GetPositionZ());
        }
    }

    if (channelerDiamond->GetVictim() == bot)
    {
        const TankSpot& spot = MagtheridonTankSpots::NWChanneler;
        const float maxStep = 5.0f;
        const float positionLeeway = 3.0f;

        float dX = spot.x - bot->GetPositionX();
        float dY = spot.y - bot->GetPositionY();
        float distanceToSpot = bot->GetExactDist2d(spot.x, spot.y);
        if (distanceToSpot > positionLeeway)
        {
            float step = std::min(maxStep, distanceToSpot);
            float moveX = bot->GetPositionX() + (dX / distanceToSpot) * step;
            float moveY = bot->GetPositionY() + (dY / distanceToSpot) * step;
            const float moveZ = spot.z;
            {
                return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, 
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerNEChannelerTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    ObjectGuid currentIconGuid = group->GetTargetIcon(triangleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channelerTriangle->GetGUID())
    {
        group->SetTargetIcon(triangleIcon, bot->GetGUID(), channelerTriangle->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "triangle" ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != channelerTriangle)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerTriangle);
    }

    if (bot->GetVictim() != channelerTriangle)
    {
        Attack(channelerTriangle);

        if (!bot->IsWithinMeleeRange(channelerTriangle))
        {
            return MoveTo(channelerTriangle->GetMapId(), channelerTriangle->GetPositionX(),
                          channelerTriangle->GetPositionY(), channelerTriangle->GetPositionZ());
        }
    }

    if (channelerTriangle->GetVictim())
    {
        const TankSpot& spot = MagtheridonTankSpots::NEChanneler;
        const float maxStep = 5.0f;
        const float positionLeeway = 3.0f;

        float dX = spot.x - bot->GetPositionX();
        float dY = spot.y - bot->GetPositionY();
        float distanceToSpot = bot->GetExactDist2d(spot.x, spot.y);
        if (distanceToSpot > positionLeeway)
        {
            float step = std::min(maxStep, distanceToSpot);
            float moveX = bot->GetPositionX() + (dX / distanceToSpot) * step;
            float moveY = bot->GetPositionY() + (dY / distanceToSpot) * step;
            const float moveZ = spot.z;
            {
                return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, 
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerMisdirectionAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }
    
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

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    bool actionTaken = false;
    switch (hunterIndex)
    {
        case 0:
            if (mainTank && channelerStar && channelerStar->IsAlive() && channelerStar->GetVictim() != mainTank)
            {
                if (botAI->CanCastSpell("misdirection", mainTank))
                    botAI->CastSpell("misdirection", mainTank);

                if (!bot->HasAura(MISDIRECTION))
                    return actionTaken;

                if (botAI->CanCastSpell("steady shot", channelerStar))
                {
                    botAI->CastSpell("steady shot", channelerStar);
                    actionTaken = true;
                }
            }
            break;

        case 1:
            if (mainTank && channelerCircle && channelerCircle->IsAlive() && channelerCircle->GetVictim() != mainTank)
            {
                if (botAI->CanCastSpell("misdirection", mainTank))
                    botAI->CastSpell("misdirection", mainTank);

                if (!bot->HasAura(MISDIRECTION))
                    return actionTaken;

                if (botAI->CanCastSpell("steady shot", channelerCircle))
                {
                    botAI->CastSpell("steady shot", channelerCircle);
                    actionTaken = true;
                }
            }
            break;

        default:
            break;
    }

    return actionTaken;
}

bool MagtheridonHellfireChannelerDPSPriorityAction::Execute(Event event)
{
    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    if (channelerSquare && channelerSquare->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        if (rtiValue != "square" || rtiTarget != channelerSquare)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerSquare);
        }
        if (bot->GetVictim() != channelerSquare)
        {
            Attack(channelerSquare);
        }

        return false;
    }

    if (channelerStar && channelerStar->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        if (rtiValue != "star" || rtiTarget != channelerStar)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerStar);
        }
        if (bot->GetVictim() != channelerStar)
        {
            Attack(channelerStar);
        }

        return false;
    }

    if (channelerCircle && channelerCircle->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "circle" || rtiTarget != channelerCircle)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerCircle);
        }
        if (bot->GetVictim() != channelerCircle)
        {
            Attack(channelerCircle);
        }

        return false;
    }

    if (channelerDiamond && channelerDiamond->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "diamond" || rtiTarget != channelerDiamond)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerDiamond);
        }
        if (bot->GetVictim() != channelerDiamond)
        {
            Attack(channelerDiamond);
        }

        return false;
    }

    if (channelerTriangle && channelerTriangle->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "triangle" || rtiTarget != channelerTriangle)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerTriangle);
        }
        if (bot->GetVictim() != channelerTriangle)
        {
            Attack(channelerTriangle);
        }

        return false;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->IsAlive() && 
        !magtheridon->HasAura(SHADOW_CAGE) &&
        (!channelerSquare || !channelerSquare->IsAlive()) &&
        (!channelerStar || !channelerStar->IsAlive()) &&
        (!channelerCircle || !channelerCircle->IsAlive()) &&
        (!channelerDiamond || !channelerDiamond->IsAlive()) &&
        (!channelerTriangle || !channelerTriangle->IsAlive()))
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "cross" || rtiTarget != magtheridon)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(magtheridon);
        }
        if (bot->GetVictim() != magtheridon)
        {
            Attack(magtheridon);
        }
    }

    return false;
}

bool MagtheridonBurningAbyssalWarlockCCAction::Execute(Event event)
{
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    bool isAlreadyBanishing = false;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == BURNING_ABYSSAL && unit->HasAura(BANISH))
        {
            isAlreadyBanishing = true;
            break;
        }
    }

    bool isAlreadyFearing = false;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == BURNING_ABYSSAL && unit->HasAura(FEAR))
        {
            isAlreadyFearing = true;
            break;
        }
    }

    if (!isAlreadyBanishing)
    {
        for (const auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->GetEntry() == BURNING_ABYSSAL && !unit->HasAura(BANISH))
            {
                if (unit->IsAlive() && botAI->CanCastSpell(BANISH, unit, true))
                {
                    botAI->CastSpell(BANISH, unit);
                    return true;
                }
            }
        }
    }

    if (!isAlreadyFearing)
    {
        for (const auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->GetEntry() == BURNING_ABYSSAL &&
                !unit->HasAura(BANISH) && !unit->HasAura(FEAR))
            {
                if (unit->IsAlive() && botAI->CanCastSpell(FEAR, unit, true))
                {
                    botAI->CastSpell(FEAR, unit);
                    return true;
                }
            }
        }
    }

    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->GetEntry() == HELLFIRE_CHANNELER)
        {
            if (!unit->HasAura(CURSE_OF_TONGUES))
            {
                if (botAI->CanCastSpell(CURSE_OF_TONGUES, unit, true))
                {
                    botAI->CastSpell(CURSE_OF_TONGUES, unit);
                    return true;
                }
            }
        }
    }

    return false;
}

bool MagtheridonPositionBossAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    Group* group = bot->GetGroup();
    ObjectGuid currentIconGuid = group->GetTargetIcon(crossIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != magtheridon->GetGUID())
    {
        group->SetTargetIcon(crossIcon, bot->GetGUID(), magtheridon->GetGUID());
    }

    Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
    std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
    if (rtiValue != "cross" || rtiTarget != magtheridon)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(magtheridon);
    }

    if (bot->GetVictim() != magtheridon)
    {
        Attack(magtheridon);
    }

    if (magtheridon->GetVictim() == bot)
    {
        const TankSpot& spot = MagtheridonTankSpots::Magtheridon;
        const float maxStep = 5.0f;
        const float positionLeeway = 3.0f;

        float dX = spot.x - bot->GetPositionX();
        float dY = spot.y - bot->GetPositionY();
        float distanceToSpot = bot->GetExactDist2d(spot.x, spot.y);
        if (distanceToSpot > positionLeeway)
        {
            float step = std::min(maxStep, distanceToSpot);
            float moveX = bot->GetPositionX() + (dX / distanceToSpot) * step;
            float moveY = bot->GetPositionY() + (dY / distanceToSpot) * step;
            const float moveZ = spot.z;
            return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        float orientation = atan2(magtheridon->GetPositionY() - bot->GetPositionY(),
                                  magtheridon->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }
    else if (!bot->IsWithinMeleeRange(magtheridon))
    {
        return MoveTo(magtheridon->GetMapId(), magtheridon->GetPositionX(), 
                      magtheridon->GetPositionY(), magtheridon->GetPositionZ());
    }

    return false;
}

bool MagtheridonSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->HasAura(SHADOW_CAGE))
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const float spreadCenterX = -15.585;
    const float spreadCenterY = 1.761f;

    Group* group = bot->GetGroup();
    std::vector<Player*> raidMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        raidMembers.push_back(member);
    }

    if (initialPositions.find(bot->GetGUID()) == initialPositions.end())
    {
        size_t botIndex = 0;
        for (size_t i = 0; i < raidMembers.size(); ++i)
        {
            if (raidMembers[i] == bot)
            {
                botIndex = i;
                break;
            }
        }

        const float arcRadians = 2.0f * M_PI;
        const float startAngle = 0.0f;
        float angleStep = raidMembers.size() > 0 ? arcRadians / raidMembers.size() : 0.0f;
        float angle = startAngle + botIndex * angleStep;

        const float minRadius = 1.0f;
        const float maxRadius = 25.0f;
        float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius);

        float targetX = spreadCenterX + radius * cos(angle);
        float targetY = spreadCenterY + radius * sin(angle);
        float targetZ = bot->GetPositionZ();

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, targetZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        float destX = targetPosition.GetPositionX();
        float destY = targetPosition.GetPositionY();
        float destZ = bot->GetPositionZ();
        if (!bot->IsWithinDist2d(destX, destY, 3.0f) && 
            bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
                                                           bot->GetPositionZ(), destX, destY, destZ))
        {
            return MoveTo(bot->GetMapId(), destX, destY, destZ);
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    const float magtheridonRangedRadius = 15.0f;
    const float minSpreadDistance = 5.0f;
    const float movementThreshold = 2.0f;

    Unit* closestMember = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
        {
            continue;
        }
        if (!closestMember || bot->GetExactDist2d(member) < bot->GetExactDist2d(closestMember))
            closestMember = member;
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    float magX = magtheridon->GetPositionX();
    float magY = magtheridon->GetPositionY();
    float distanceToMagtheridon = bot->GetExactDist2d(magX, magY);

    float destX2 = magX;
    float destY2 = magY;
    float destZ2 = bot->GetPositionZ();
    if ((distanceToMagtheridon < magtheridonRangedRadius - 3.0f - movementThreshold) && 
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
                                                       bot->GetPositionZ(), destX2, destY2, destZ2))
    {
        return MoveTo(bot->GetMapId(), destX2, destY2, destZ2);
    }

    return false;
}

bool MagtheridonSpreadHealerAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->HasAura(SHADOW_CAGE))
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const float spreadCenterX = -2.005f;
    const float spreadCenterY = 1.867f;

    Group* group = bot->GetGroup();
    std::vector<Player*> raidMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        raidMembers.push_back(member);
    }

    if (initialPositions.find(bot->GetGUID()) == initialPositions.end())
    {
        size_t botIndex = 0;
        for (size_t i = 0; i < raidMembers.size(); ++i)
        {
            if (raidMembers[i] == bot)
            {
                botIndex = i;
                break;
            }
        }

        const float arcRadians = 2.0f * M_PI;
        const float startAngle = 0.0f;
        float angleStep = raidMembers.size() > 0 ? arcRadians / raidMembers.size() : 0.0f;
        float angle = startAngle + botIndex * angleStep;

        const float minRadius = 1.0f;
        const float maxRadius = 15.0f;
        float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius);

        float targetX = spreadCenterX + radius * cos(angle);
        float targetY = spreadCenterY + radius * sin(angle);
        float targetZ = bot->GetPositionZ();

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, targetZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        float destX = targetPosition.GetPositionX();
        float destY = targetPosition.GetPositionY();
        float destZ = bot->GetPositionZ();
        if (!bot->IsWithinDist2d(destX, destY, 3.0f) && 
            bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
                                                           bot->GetPositionZ(), destX, destY, destZ))
        {
            return MoveTo(bot->GetMapId(), destX, destY, destZ);
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    const float magtheridonHealerRadius = 10.0f;
    const float minSpreadDistance = 5.0f;
    const float movementThreshold = 2.0f;

    Unit* closestMember = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
        {
            continue;
        }
        if (!closestMember || bot->GetExactDist2d(member) < bot->GetExactDist2d(closestMember))
            closestMember = member;
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    float magX = magtheridon->GetPositionX();
    float magY = magtheridon->GetPositionY();
    float distanceToMagtheridon = bot->GetExactDist2d(magX, magY);

    float destX2 = magX;
    float destY2 = magY;
    float destZ2 = bot->GetPositionZ();
    if ((distanceToMagtheridon < magtheridonHealerRadius - 3.0f - movementThreshold) && 
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
                                                       bot->GetPositionZ(), destX2, destY2, destZ2))
    {
        return MoveTo(bot->GetMapId(), destX2, destY2, destZ2);
    }

    return false;
}

bool MagtheridonUseManticronCubeAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    auto it = botToCubeAssignment.find(bot->GetGUID());
    const CubeInfo& cubeInfo = it->second;
    GameObject* cube = botAI->GetGameObject(cubeInfo.guid);
    if (!cube) 
    {
        return false;
    }

    if (bot->HasAura(SHADOW_GRASP) &&
        !(magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
          magtheridon->FindCurrentSpellBySpellId(BLAST_NOVA)))
    {
            uint32 delay = urand(200, 3000);
            botAI->AddTimedEvent(
                [this, cube]
                {
                    botAI->Reset();
                },
                delay);
            botAI->SetNextCheckDelay(delay + 50);
            return true;
    }

    time_t now = time(nullptr);
    time_t lastBlastNova = magtheridonBlastNovaTimer[bot->GetMapId()];
    bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
         magtheridon->FindCurrentSpellBySpellId(BLAST_NOVA);
    if (now - lastBlastNova < 49)
    {
        return false;
    }

    if (!blastNovaActive) 
    {
        const float safeWaitDistance = 8.0f;
        float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

        if (fabs(cubeDist - safeWaitDistance) > 0.5f) 
        {
            for (int i = 0; i < 12; ++i)
            {
                float angle = i * M_PI / 6.0f;
                float targetX = cubeInfo.x + cos(angle) * safeWaitDistance;
                float targetY = cubeInfo.y + sin(angle) * safeWaitDistance;
                float targetZ = bot->GetPositionZ();

                if (IsSafeFromMagtheridonHazards(botAI, bot, targetX, targetY, targetZ))
                {
                    bot->AttackStop();
                    bot->InterruptNonMeleeSpells(false);
                    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, 
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
            }
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
            float fallbackX = cubeInfo.x + cos(angle) * safeWaitDistance;
            float fallbackY = cubeInfo.y + sin(angle) * safeWaitDistance;
            float fallbackZ = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), fallbackX, fallbackY, fallbackZ, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            return true;
        }
    }

    const float interactDistance = 1.0f;
    const float interactDistanceBuffer = 1.0f;
    float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

    if (cubeDist > interactDistance) 
    {
        if (cubeDist <= interactDistance + interactDistanceBuffer) 
        {
            uint32 delay = urand(200, 1900);
            botAI->AddTimedEvent(
                [this, cube]
                {
                    bot->StopMoving();
                    cube->Use(bot);
                },
                delay);
            botAI->SetNextCheckDelay(delay + 50);
            return true;
        }
        
        float angle = atan2(cubeInfo.y - bot->GetPositionY(), cubeInfo.x - bot->GetPositionX());
        float targetX = cubeInfo.x - cos(angle) * interactDistance;
        float targetY = cubeInfo.y - sin(angle) * interactDistance;
        float targetZ = bot->GetPositionZ();
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);

        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, 
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}
