#include "RaidGruulsLairActions.h"
#include "RaidGruulsLairHelpers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "UnitAI.h"

#include "Log.h"

bool HighKingMaulgarMaulgarTankAction::Execute(Event event)
{
    if (!IsFirstTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar || !maulgar->IsAlive())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] Maulgar not found or not alive");
        return false;
    }

    LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] Maulgar found, GUID: {}", maulgar->GetGUID().ToString().c_str());

    ObjectGuid currentIconGuid = group->GetTargetIcon(squareIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != maulgar->GetGUID())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] Bot GUID: {}, Maulgar GUID: {}", bot->GetGUID().ToString().c_str(), maulgar->GetGUID().ToString().c_str());
        LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] Group state: {}", group ? "Valid" : "Invalid");

        group->SetTargetIcon(squareIcon, bot->GetGUID(), maulgar->GetGUID());

        LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] Successfully set square icon for Maulgar");
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "square" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != maulgar)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] {} is updating RTI to square and setting target to Maulgar", bot->GetName().c_str());
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(maulgar);
    }

    // Check if bot has no target or if target is not Maulgar
    if (bot->GetVictim() != maulgar)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] {} is switching target to Maulgar", bot->GetName().c_str());
        Attack(maulgar);
        
        // If we've just switched targets and aren't in melee range, move to basic melee range first
        if (!bot->IsWithinMeleeRange(maulgar))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] {} is moving into initial melee range of Maulgar", bot->GetName().c_str());
            return MoveTo(maulgar->GetMapId(), maulgar->GetPositionX(), maulgar->GetPositionY(), maulgar->GetPositionZ());
        }
    }

    // If we're tanking, prioritize positioning over simply being in melee range
    if (maulgar->GetVictim() == bot)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] {} is tanking Maulgar", bot->GetName().c_str());
        const TankSpot& spot = GruulsLairTankSpots::Maulgar;
        const float maxDistance = 3.0f;
        float distanceToMaulgar = maulgar->GetExactDist2d(spot.x, spot.y);
        
        // If boss is not near the designated spot, move to drag the boss there
        if (distanceToMaulgar > maxDistance)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] {} is moving to designated tank spot", bot->GetName().c_str());
            float dX = spot.x - maulgar->GetPositionX();
            float dY = spot.y - maulgar->GetPositionY();
            float moveX = spot.x + (dX / distanceToMaulgar) * maxDistance;
            float moveY = spot.y + (dY / distanceToMaulgar) * maxDistance;
            
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
        
        // Boss is in position, maintain facing
        float orientation = atan2(maulgar->GetPositionY() - bot->GetPositionY(), maulgar->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return false;
    }
    // We're not tanking yet, but should be in melee range to generate threat
    else if (!bot->IsWithinMeleeRange(maulgar))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMaulgarTankAction] {} is moving into melee range of Maulgar to gain threat", bot->GetName().c_str());
        return MoveTo(maulgar->GetMapId(), maulgar->GetPositionX(), maulgar->GetPositionY(), maulgar->GetPositionZ());
    }

    return false;
}

bool HighKingMaulgarOlmTankAction::Execute(Event event)
{
    if (!IsSecondTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (!olm || !olm->IsAlive())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] Olm not found or not alive");
        return false;
    }

    LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] Olm found, GUID: {}", olm->GetGUID().ToString().c_str());

    ObjectGuid currentIconGuid = group->GetTargetIcon(circleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != olm->GetGUID())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] Bot GUID: {}, Olm GUID: {}", bot->GetGUID().ToString().c_str(), olm->GetGUID().ToString().c_str());
        LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] Group state: {}", group ? "Valid" : "Invalid");

        group->SetTargetIcon(circleIcon, bot->GetGUID(), olm->GetGUID());

        LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] Successfully set circle icon for Olm");
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "circle" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != olm)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] {} is updating RTI to circle and setting target to Olm", bot->GetName().c_str());
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(olm);
    }

    // Check if bot has no target or if target is not Olm
    if (bot->GetVictim() != olm)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] {} is switching target to Olm", bot->GetName().c_str());
        Attack(olm);
        
        // If we've just switched targets and aren't in melee range, move to basic melee range first
        if (!bot->IsWithinMeleeRange(olm))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] {} is moving into initial melee range of Olm", bot->GetName().c_str());
            return MoveTo(olm->GetMapId(), olm->GetPositionX(), olm->GetPositionY(), olm->GetPositionZ());
        }
    }

    // If we're tanking, prioritize positioning over simply being in melee range
    if (olm->GetVictim() == bot)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] {} is tanking Olm", bot->GetName().c_str());
        const TankSpot& spot = GruulsLairTankSpots::Maulgar;
        const float maxDistance = 3.0f;
        float distanceToOlm = olm->GetExactDist2d(spot.x, spot.y);
        
        // If boss is not near the designated spot, move to drag the boss there
        if (distanceToOlm > maxDistance)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] {} is moving to designated tank spot", bot->GetName().c_str());
            float dX = spot.x - olm->GetPositionX();
            float dY = spot.y - olm->GetPositionY();
            float moveX = spot.x + (dX / distanceToOlm) * maxDistance;
            float moveY = spot.y + (dY / distanceToOlm) * maxDistance;
            
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
        
        // Boss is in position, maintain facing
        float orientation = atan2(olm->GetPositionY() - bot->GetPositionY(), olm->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return false;
    }
    // We're not tanking yet, but should be in melee range to generate threat
    else if (!bot->IsWithinMeleeRange(olm))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarOlmTankAction] {} is moving into melee range of Olm to gain threat", bot->GetName().c_str());
        return MoveTo(olm->GetMapId(), olm->GetPositionX(), olm->GetPositionY(), olm->GetPositionZ());
    }

    return false;
}

bool HighKingMaulgarBlindeyeTankAction::Execute(Event event)
{
    if (!IsThirdTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (!blindeye || !blindeye->IsAlive())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] Blindeye not found or not alive");
        return false;
    }

    LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] Blindeye found, GUID: {}", blindeye->GetGUID().ToString().c_str());

    ObjectGuid currentIconGuid = group->GetTargetIcon(starIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != blindeye->GetGUID())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] Bot GUID: {}, Blindeye GUID: {}", bot->GetGUID().ToString().c_str(), blindeye->GetGUID().ToString().c_str());
        LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] Group state: {}", group ? "Valid" : "Invalid");

        group->SetTargetIcon(starIcon, bot->GetGUID(), blindeye->GetGUID());

        LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] Successfully set star icon for Blindeye");
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "star" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != blindeye)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] {} is updating RTI to star and setting target to Blindeye", bot->GetName().c_str());
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(blindeye);
    }

    // Check if bot has no target or if target is not Blindeye
    if (bot->GetVictim() != blindeye)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] {} is switching target to Blindeye", bot->GetName().c_str());
        Attack(blindeye);
        
        // If we've just switched targets and aren't in melee range, move to basic melee range first
        if (!bot->IsWithinMeleeRange(blindeye))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] {} is moving into initial melee range of Blindeye", bot->GetName().c_str());
            return MoveTo(blindeye->GetMapId(), blindeye->GetPositionX(), blindeye->GetPositionY(), blindeye->GetPositionZ());
        }
    }

    // If we're tanking, prioritize positioning over simply being in melee range
    if (blindeye->GetVictim() == bot)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] {} is tanking Blindeye", bot->GetName().c_str());
        const TankSpot& spot = GruulsLairTankSpots::Blindeye;
        const float maxDistance = 3.0f;
        float distanceToBlindeye = blindeye->GetExactDist2d(spot.x, spot.y);
        
        // If boss is not near the designated spot, move to drag the boss there
        if (distanceToBlindeye > maxDistance)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] {} is moving to designated tank spot", bot->GetName().c_str());
            float dX = spot.x - blindeye->GetPositionX();
            float dY = spot.y - blindeye->GetPositionY();
            float moveX = spot.x + (dX / distanceToBlindeye) * maxDistance;
            float moveY = spot.y + (dY / distanceToBlindeye) * maxDistance;
            
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
        
        // Boss is in position, maintain facing
        float orientation = atan2(blindeye->GetPositionY() - bot->GetPositionY(), blindeye->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return false;
    }
    // We're not tanking yet, but should be in melee range to generate threat
    else if (!bot->IsWithinMeleeRange(blindeye))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBlindeyeTankAction] {} is moving into melee range of Blindeye to gain threat", bot->GetName().c_str());
        return MoveTo(blindeye->GetMapId(), blindeye->GetPositionX(), blindeye->GetPositionY(), blindeye->GetPositionZ());
    }

    return false;
}

bool HighKingMaulgarMageTankAction::Execute(Event event)
{
    if (!IsMageTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh || !krosh->IsAlive())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] Krosh not found or not alive");
        return false;
    }

    ObjectGuid currentIconGuid = group->GetTargetIcon(triangleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != krosh->GetGUID())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] Bot GUID: {}, Krosh GUID: {}", bot->GetGUID().ToString().c_str(), krosh->GetGUID().ToString().c_str());
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] Group state: {}", group ? "Valid" : "Invalid");

        group->SetTargetIcon(triangleIcon, bot->GetGUID(), krosh->GetGUID());

        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] Successfully set triangle icon for Krosh");
    }

        if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "triangle" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != krosh)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] {} is updating RTI to triangle and setting target to Krosh", bot->GetName().c_str());
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(krosh);
    }

    if (bot->GetVictim() != krosh)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] {} is switching target to Krosh", bot->GetName().c_str());
        Attack(krosh);
    }

    float const OPTIMAL_RANGED_DISTANCE = 25.0f;
    float const MIN_RANGE = OPTIMAL_RANGED_DISTANCE - 4.0f;
    float const MAX_RANGE = OPTIMAL_RANGED_DISTANCE + 4.0f;

    // Too close or too far
    if (!bot->IsWithinRange(krosh, MIN_RANGE) || bot->IsWithinRange(krosh, MAX_RANGE))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] {} is adjusting position to maintain optimal range from Krosh", bot->GetName().c_str());

        // If too close, back away
        if (bot->IsWithinRange(krosh, MIN_RANGE))
        {
            // Calculate a position at optimal range away from the boss
            float dx = bot->GetPositionX() - krosh->GetPositionX();
            float dy = bot->GetPositionY() - krosh->GetPositionY();
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist > 0.1f) // Avoid division by zero
            {
                dx = dx / dist * OPTIMAL_RANGED_DISTANCE;
                dy = dy / dist * OPTIMAL_RANGED_DISTANCE;

                return MoveTo(krosh->GetMapId(),
                            krosh->GetPositionX() + dx,
                            krosh->GetPositionY() + dy,
                            krosh->GetPositionZ());
            }
        }
        // If too far, move closer
        else
        {
            // Move toward the boss, but stop at optimal distance
            float dx = krosh->GetPositionX() - bot->GetPositionX();
            float dy = krosh->GetPositionY() - bot->GetPositionY();
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist > 0.1f) // Avoid division by zero
            {
                // Calculate how much to move toward the boss
                float moveAmount = dist - OPTIMAL_RANGED_DISTANCE;
                dx = dx / dist * moveAmount;
                dy = dy / dist * moveAmount;
                
                return MoveTo(bot->GetMapId(), 
                            bot->GetPositionX() + dx, 
                            bot->GetPositionY() + dy, 
                            bot->GetPositionZ());
            }
        }
    }
    return false;
}

/*
    if (krosh && bot->GetVictim() == krosh)
    {
        if (krosh->HasAura(SPELL_AURA_SPELL_SHIELD))
        {
            botAI->CastSpell("spellsteal", krosh);
            return true;
        }

        if (!bot->HasAura(SPELL_AURA_SPELL_SHIELD))
        {
            botAI->CastSpell("fire ward", bot);
            return true;
        }
        return true;
    }
    return false;
} */

bool HighKingMaulgarBoomkinTankAction::Execute(Event event)
{
    if (!IsBoomkinTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler || !kiggler->IsAlive())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBoomkinTankAction] Kiggler not found or not alive");
        return false;
    }

    LOG_DEBUG("playerbots", "[HighKingMaulgarBoomkinTankAction] Kiggler found, GUID: {}", kiggler->GetGUID().ToString().c_str());

    ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != kiggler->GetGUID())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBoomkinTankAction] Bot GUID: {}, Kiggler GUID: {}", bot->GetGUID().ToString().c_str(), kiggler->GetGUID().ToString().c_str());
        LOG_DEBUG("playerbots", "[HighKingMaulgarBoomkinTankAction] Group state: {}", group ? "Valid" : "Invalid");

        group->SetTargetIcon(diamondIcon, bot->GetGUID(), kiggler->GetGUID());

        LOG_DEBUG("playerbots", "[HighKingMaulgarBoomkinTankAction] Successfully set diamond icon for Kiggler");
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != kiggler)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBoomkinTankAction] {} is updating RTI to diamond and setting target to Kiggler", bot->GetName().c_str());
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
    }

    if (bot->GetVictim() != kiggler)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBoomkinTankAction] {} is switching target to Kiggler", bot->GetName().c_str());
        Attack(kiggler);
    }

    float const OPTIMAL_RANGED_DISTANCE = 32.5f;
    float const MIN_RANGE = OPTIMAL_RANGED_DISTANCE - 2.5f;
    float const MAX_RANGE = OPTIMAL_RANGED_DISTANCE + 2.5f;

    // Too close or too far
    if (bot->IsWithinRange(kiggler, MIN_RANGE) || !bot->IsWithinRange(kiggler, MAX_RANGE))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarBoomkinTankAction] {} is adjusting position to maintain optimal range from Kiggler", bot->GetName().c_str());
        
        // If too close, back away
        if (bot->IsWithinRange(kiggler, MIN_RANGE))
        {
            // Calculate a position at optimal range away from the boss
            float dx = bot->GetPositionX() - kiggler->GetPositionX();
            float dy = bot->GetPositionY() - kiggler->GetPositionY();
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist > 0.1f) // Avoid division by zero
            {
                dx = dx / dist * OPTIMAL_RANGED_DISTANCE;
                dy = dy / dist * OPTIMAL_RANGED_DISTANCE;
                
                return MoveTo(kiggler->GetMapId(), 
                            kiggler->GetPositionX() + dx, 
                            kiggler->GetPositionY() + dy, 
                            kiggler->GetPositionZ());
            }
        }
        // If too far, move closer
        else
        {
            // Move toward the boss, but stop at optimal distance
            float dx = kiggler->GetPositionX() - bot->GetPositionX();
            float dy = kiggler->GetPositionY() - bot->GetPositionY();
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist > 0.1f) // Avoid division by zero
            {
                // Calculate how much to move toward the boss
                float moveAmount = dist - OPTIMAL_RANGED_DISTANCE;
                dx = dx / dist * moveAmount;
                dy = dy / dist * moveAmount;
                
                return MoveTo(bot->GetMapId(), 
                            bot->GetPositionX() + dx, 
                            bot->GetPositionY() + dy, 
                            bot->GetPositionZ());
            }
        }
    }
    return false;
}

bool HighKingMaulgarMeleeDPSAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;
    
    LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is executing Melee DPS Action", bot->GetName().c_str());

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    // Check if this bot should be performing the Melee DPS role
    if (!botAI->IsMelee(bot) || 
        (IsFirstTank(botAI, bot) && maulgar && maulgar->IsAlive()) || 
        (IsSecondTank(botAI, bot) && olm && olm->IsAlive()) || 
        (IsThirdTank(botAI, bot) && blindeye && blindeye->IsAlive()))
        return false;

    // Target priority 1: Blindeye (existing logic - looks good)
    if (blindeye && blindeye->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
    
        // Check if we need to update the RTI
        if (rtiValue != "star" || rtiTarget != blindeye)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is updating RTI to star for Blindeye", bot->GetName().c_str());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(blindeye);
        }

        // Check if we need to switch targets
        if (bot->GetVictim() != blindeye)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is attacking Blindeye", bot->GetName().c_str());
            Attack(blindeye);  // Don't return immediately
        }

        // Not in melee range OR unsafe position
        if (!bot->IsWithinMeleeRange(blindeye) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is repositioning for Blindeye while avoiding boss mechanics", bot->GetName().c_str());
            
            // Find safe position at melee distance
            Position safePos = FindSafePosition(botAI, bot, blindeye, 5.0f);
            
            return MoveTo(blindeye->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        return false;
    }

    // Target priority 2: Olm (if Blindeye is dead)
    if (olm && olm->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        // Check if we need to update the RTI
        if (rtiValue != "circle" || rtiTarget != olm)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is updating RTI to circle for Olm", bot->GetName().c_str());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(olm);
        }

        // Check if we need to switch targets
        if (bot->GetVictim() != olm)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is attacking Olm", bot->GetName().c_str());
            Attack(olm); // Don't return immediately
        }
        
        // Not in melee range OR unsafe position
        if (!bot->IsWithinMeleeRange(olm) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is repositioning for Olm while avoiding boss mechanics", bot->GetName().c_str());
            
            // Find safe position at melee distance
            Position safePos = FindSafePosition(botAI, bot, olm, 5.0f);
            
            return MoveTo(olm->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        return false;
    }

    // Target priority 3: Kiggler (if Blindeye and Olm are dead)
    if (kiggler && kiggler->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        // Check if we need to update the RTI
        if (rtiValue != "diamond" || rtiTarget != kiggler)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is updating RTI to diamond for Kiggler", bot->GetName().c_str());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
        }

        // Check if we need to switch targets
        if (bot->GetVictim() != kiggler)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is attacking Kiggler", bot->GetName().c_str());
            Attack(kiggler); // Don't return immediately
        }

        // Not in melee range OR unsafe position
        if (!bot->IsWithinMeleeRange(kiggler) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is repositioning for Kiggler while avoiding boss mechanics", bot->GetName().c_str());
            
            // Find safe position at melee distance
            Position safePos = FindSafePosition(botAI, bot, kiggler, 5.0f);
            
            return MoveTo(kiggler->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        return false;
    }

    // Target priority 4: Maulgar (if all others are dead)
    if (maulgar && maulgar->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        // Check if we need to update the RTI
        if (rtiValue != "square" || rtiTarget != maulgar)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is updating RTI to square for Maulgar", bot->GetName().c_str());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(maulgar);
        }

        // Check if we need to switch targets
        if (bot->GetVictim() != maulgar)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is attacking Maulgar", bot->GetName().c_str());
            Attack(maulgar); // Don't return immediately
        }
        
        // Not in melee range OR unsafe position
        if (!bot->IsWithinMeleeRange(maulgar) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarMeleeDPSAction] {} is repositioning for Maulgar while avoiding boss mechanics", bot->GetName().c_str());
            
            // Find safe position at melee distance
            Position safePos = FindSafePosition(botAI, bot, maulgar, 5.0f);
            
            return MoveTo(maulgar->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
        
        return false;
    }
    
    return false;
}

bool HighKingMaulgarRangedDPSAction::Execute(Event event) // Need separate action for Warlock
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is executing Ranged DPS Action", bot->GetName().c_str());

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    // Check if this bot should be performing the Ranged DPS role
    if (!botAI->IsRanged(bot) || 
        (IsMageTank(botAI, bot) && krosh && krosh->IsAlive()) || 
        (IsBoomkinTank(botAI, bot) && kiggler && kiggler->IsAlive()) || 
        botAI->IsHeal(bot))
        return false;

    // Define optimal range parameters for ranged DPS
    float const OPTIMAL_RANGED_DISTANCE = 25.0f;
    float const MIN_RANGE = OPTIMAL_RANGED_DISTANCE - 4.0f;
    float const MAX_RANGE = OPTIMAL_RANGED_DISTANCE + 4.0f;

    // Target priority 1: Blindeye
    if (blindeye && blindeye->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        // Check if we need to update the RTI
        if (rtiValue != "star" || rtiTarget != blindeye)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is updating RTI to star for Blindeye", bot->GetName().c_str());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(blindeye);
        }

        // Check if we need to switch targets
        if (bot->GetVictim() != blindeye)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is attacking Blindeye", bot->GetName().c_str());
            Attack(blindeye); // Don't return immediately
        }

        // Too close, too far, OR unsafe position
        if (bot->IsWithinRange(blindeye, MIN_RANGE) || 
            !bot->IsWithinRange(blindeye, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is adjusting position for Blindeye while avoiding boss mechanics", bot->GetName().c_str());
            
            // Find safe position at optimal ranged distance
            Position safePos = FindSafePosition(botAI, bot, blindeye, OPTIMAL_RANGED_DISTANCE);
            
            return MoveTo(blindeye->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        return false;
    }

    // Target priority 2: Olm (if Blindeye is dead)
    if (olm && olm->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        // Check if we need to update the RTI
        if (rtiValue != "circle" || rtiTarget != olm)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is updating RTI to circle for Olm", bot->GetName().c_str());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(olm);
        }

        // Check if we need to switch targets
        if (bot->GetVictim() != olm)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is attacking Olm", bot->GetName().c_str());
            Attack(olm); // Don't return immediately
        }
        
        // Too close, too far, OR unsafe position
        if (bot->IsWithinRange(olm, MIN_RANGE) || 
            !bot->IsWithinRange(olm, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is adjusting position for Olm while avoiding boss mechanics", bot->GetName().c_str());
            
            // Find safe position at optimal ranged distance
            Position safePos = FindSafePosition(botAI, bot, olm, OPTIMAL_RANGED_DISTANCE);
            
            return MoveTo(olm->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
        
        return false;
    }

    // Target priority 3: Kiggler (if Blindeye and Olm are dead)
    if (kiggler && kiggler->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        // Check if we need to update the RTI
        if (rtiValue != "diamond" || rtiTarget != kiggler)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is updating RTI to diamond for Kiggler", bot->GetName().c_str());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
        }

        // Check if we need to switch targets
        if (bot->GetVictim() != kiggler)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is attacking Kiggler", bot->GetName().c_str());
            Attack(kiggler); // Don't return immediately
        }
        
        // Too close, too far, OR unsafe position
        if (bot->IsWithinRange(kiggler, MIN_RANGE) || 
            !bot->IsWithinRange(kiggler, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is adjusting position for Kiggler while avoiding boss mechanics", bot->GetName().c_str());
            
            // Find safe position at optimal ranged distance
            Position safePos = FindSafePosition(botAI, bot, kiggler, OPTIMAL_RANGED_DISTANCE);
            
            return MoveTo(kiggler->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
        
        return false;
    }

    // Target priority 4: Krosh (if Blindeye, Olm, and Kiggler are dead)
    if (krosh && krosh->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        // Check if we need to update the RTI
        if (rtiValue != "triangle" || rtiTarget != krosh)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is updating RTI to triangle for Krosh", bot->GetName().c_str());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(krosh);
        }

        // Check if we need to switch targets
        if (bot->GetVictim() != krosh)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is attacking Krosh", bot->GetName().c_str());
            Attack(krosh); // Don't return immediately
        }
        
        // Too close, too far, OR unsafe position
        if (bot->IsWithinRange(krosh, MIN_RANGE) || 
            !bot->IsWithinRange(krosh, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is adjusting position for Krosh while avoiding boss mechanics", bot->GetName().c_str());
            
            // Find safe position at optimal ranged distance
            Position safePos = FindSafePosition(botAI, bot, krosh, OPTIMAL_RANGED_DISTANCE);
            
            return MoveTo(krosh->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
        
        return false;
    }

    // Target priority 5: Maulgar (if all others are dead)
    if (maulgar && maulgar->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        // Check if we need to update the RTI
        if (rtiValue != "square" || rtiTarget != maulgar)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is updating RTI to square for Maulgar", bot->GetName().c_str());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(maulgar);
        }

        // Check if we need to switch targets
        if (bot->GetVictim() != maulgar)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is attacking Maulgar", bot->GetName().c_str());
            Attack(maulgar); // Don't return immediately
        }
        
        // Too close, too far, OR unsafe position
        if (bot->IsWithinRange(maulgar, MIN_RANGE) || 
            !bot->IsWithinRange(maulgar, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarRangedDPSAction] {} is adjusting position for Maulgar while avoiding boss mechanics", bot->GetName().c_str());
            
            // Find safe position at optimal ranged distance
            Position safePos = FindSafePosition(botAI, bot, maulgar, OPTIMAL_RANGED_DISTANCE);
            
            return MoveTo(maulgar->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
        
        return false;
    }

    return false;
}

/* bool HighKingMaulgarAvoidBlastWaveAction::Execute(Event event)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    float safeDistance = 21.0f;
    if (bot->GetDistance(krosh) < safeDistance)
    {
        MoveTo(krosh, safeDistance);
        return true;
    }
    return false;
}

bool HighKingMaulgarAvoidBlastWaveAction::isUseful()
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");

    return krosh && krosh->IsAlive() && bot->GetDistance2d(krosh) < 21.0f;
}

bool HighKingMaulgarControlFelstalkerAction::Execute(Event event)
{
    Unit* charm = bot->GetCharm();
    if (charm)
    {
        MotionMaster* mm = charm->GetMotionMaster();
        UnitAI* charmAI = charm->GetAI();
        if (!mm || !charmAI)
            return false;

        Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");
        if (!felStalker || !felStalker->IsAlive())
            return false;

        if (mm->GetMotionSlotType(MOTION_SLOT_ACTIVE) == NULL_MOTION_TYPE)
        {
            mm->Clear();
            mm->MoveChase(felStalker);
            charmAI->AttackStart(felStalker);
        }
        Aura* aura = botAI->GetAura("enslave demon", charm);
        if (!aura)
            return false;

        const GuidVector party = AI_VALUE(GuidVector, "party");
        for (const auto& guid : party)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || !member->IsAlive())
                continue;
            if (member->HasAura(SPELL_AURA_DARK_DECAY) && !charm->HasSpellCooldown(SPELL_DETERMINATION))
            {
                charm->CastSpell(member, SPELL_DETERMINATION, true);
                charm->AddSpellCooldown(SPELL_DETERMINATION, 0, 10 * 1000);
                return true;
            }
        }

        Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
        if (olm && olm->IsAlive())
        {
            if (!charm->HasSpellCooldown(SPELL_THREATEN))
            {
                charm->CastSpell(olm, SPELL_THREATEN, true);
                charm->AddSpellCooldown(SPELL_THREATEN, 0, 20 * 1000);
                return true;
            }
            if (!charm->HasSpellCooldown(SPELL_WILD_BITE))
            {
                charm->CastSpell(olm, SPELL_WILD_BITE, true);
                charm->AddSpellCooldown(SPELL_WILD_BITE, 0, 5 * 1000);
                return true;
            }
        }
        else
        {
            Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
            if (krosh && krosh->IsAlive() && !charm->HasSpellCooldown(SPELL_WILD_BITE))
            {
                charm->CastSpell(krosh, SPELL_WILD_BITE, true);
                charm->AddSpellCooldown(SPELL_WILD_BITE, 0, 5 * 1000);
                return true;
            }
            Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
            if (maulgar && maulgar->IsAlive() && !charm->HasSpellCooldown(SPELL_WILD_BITE))
            {
                charm->CastSpell(maulgar, SPELL_WILD_BITE, true);
                charm->AddSpellCooldown(SPELL_WILD_BITE, 0, 5 * 1000);
                return true;
            }
        }
    }
    else
    {
        Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");
        if (felStalker && felStalker->IsAlive() && !felStalker->HasAura(SPELL_ENSLAVE_DEMON))
        {
            if (bot->GetDistance2d(felStalker) > sPlayerbotAIConfig->spellDistance)
                return MoveNear(felStalker, sPlayerbotAIConfig->spellDistance, MovementPriority::MOVEMENT_COMBAT);
            else
                return botAI->CastSpell(SPELL_ENSLAVE_DEMON, felStalker);
        }
    }
    return false;
}

bool HighKingMaulgarControlFelstalkerAction::isUseful()
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    return felStalker && felStalker->IsAlive() && bot->getClass() == CLASS_WARLOCK;
}

bool HighKingMaulgarAvoidWhirlwindAction::Execute(Event event)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    float safeDistance = 46.0f;
    if (bot->GetDistance(maulgar) < safeDistance)
    {
        MoveTo(maulgar, safeDistance);
        return true;
    }
    return false;
}

bool HighKingMaulgarAvoidWhirlwindAction::isUseful()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    
    return maulgar && maulgar->IsAlive() && maulgar->HasUnitState(UNIT_STATE_CASTING) 
    && maulgar->FindCurrentSpellBySpellId(SPELL_WHIRLWIND) && !botAI->IsTank(bot);
}*/

bool HighKingMaulgarHunterMisdirectionAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot) continue;
        Aura* aura = member->GetAura(SPELL_AURA_MISDIRECTION);
        if (aura && aura->GetCasterGUID() == bot->GetGUID())
        {
            Unit* target = member->GetVictim();
            if (!target) return false;
            botAI->CastSpell("aimed shot", target);
            botAI->CastSpell("steady shot", target);
            return true;
        }
    }
    return false;
}
