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
        const TankSpot& spot = GruulsLairTankSpots::Olm;
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

    // Check for Spell Shield to steal
    if (krosh->HasAura(SPELL_AURA_SPELL_SHIELD) && botAI->CanCastSpell(SPELL_SPELLSTEAL, krosh, true))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] {} is attempting to Spellsteal from Krosh", bot->GetName().c_str());
        botAI->CastSpell(SPELL_SPELLSTEAL, krosh);
    }

    // Apply Fire Ward if we don't have Spell Shield
    if (!bot->HasAura(SPELL_AURA_SPELL_SHIELD))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] {} is applying Fire Ward", bot->GetName().c_str());
        botAI->CastSpell("fire ward", bot);
    }

    if (bot->GetVictim() != krosh)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] {} is switching target to Krosh", bot->GetName().c_str());
        Attack(krosh);
    }

    float const OPTIMAL_RANGED_DISTANCE = 27.0f;
    float const MIN_RANGE = OPTIMAL_RANGED_DISTANCE - 2.0f;
    float const MAX_RANGE = OPTIMAL_RANGED_DISTANCE + 2.0f;

    // Too close, too far, OR unsafe position
    if (bot->IsWithinRange(krosh, MIN_RANGE) || !bot->IsWithinRange(krosh, MAX_RANGE) || 
        !IsPositionSafe(botAI, bot, bot->GetPosition()))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMageTankAction] {} is adjusting position for Krosh while avoiding boss mechanics", bot->GetName().c_str());
        
        // Find safe position at optimal ranged distance
        Position safePos = FindSafePosition(botAI, bot, krosh, OPTIMAL_RANGED_DISTANCE);
        
        return MoveTo(krosh->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
    }

    return false;
}

bool HighKingMaulgarMoonkinTankAction::Execute(Event event)
{
    if (!IsMoonkinTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler || !kiggler->IsAlive())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMoonkinTankAction] Kiggler not found or not alive");
        return false;
    }

    LOG_DEBUG("playerbots", "[HighKingMaulgarMoonkinTankAction] Kiggler found, GUID: {}", kiggler->GetGUID().ToString().c_str());

    ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != kiggler->GetGUID())
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMoonkinTankAction] Bot GUID: {}, Kiggler GUID: {}", bot->GetGUID().ToString().c_str(), kiggler->GetGUID().ToString().c_str());
        LOG_DEBUG("playerbots", "[HighKingMaulgarMoonkinTankAction] Group state: {}", group ? "Valid" : "Invalid");

        group->SetTargetIcon(diamondIcon, bot->GetGUID(), kiggler->GetGUID());

        LOG_DEBUG("playerbots", "[HighKingMaulgarMoonkinTankAction] Successfully set diamond icon for Kiggler");
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != kiggler)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMoonkinTankAction] {} is updating RTI to diamond and setting target to Kiggler", bot->GetName().c_str());
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
    }

    if (bot->GetVictim() != kiggler)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMoonkinTankAction] {} is switching target to Kiggler", bot->GetName().c_str());
        Attack(kiggler);
    }

    float const OPTIMAL_RANGED_DISTANCE = 29.0f;
    float const MIN_RANGE = OPTIMAL_RANGED_DISTANCE - 1.0f;
    float const MAX_RANGE = OPTIMAL_RANGED_DISTANCE + 1.0f;

    // Too close, too far, OR unsafe position
    if (bot->IsWithinRange(kiggler, MIN_RANGE) || !bot->IsWithinRange(kiggler, MAX_RANGE) || 
        !IsPositionSafe(botAI, bot, bot->GetPosition()))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarMoonkinTankAction] {} is adjusting position for Kiggler while avoiding boss mechanics", bot->GetName().c_str());
        
        // Find safe position at optimal ranged distance
        Position safePos = FindSafePosition(botAI, bot, kiggler, OPTIMAL_RANGED_DISTANCE);
        
        return MoveTo(kiggler->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
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

    // Target priority 1: Blindeye
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

bool HighKingMaulgarRangedDPSAction::Execute(Event event)
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
        (IsMoonkinTank(botAI, bot) && kiggler && kiggler->IsAlive()) || 
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

    // Target priority 3: Kiggler
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

    // Target priority 4: Krosh
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

    // Target priority 5: Maulgar
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

bool HighKingMaulgarHealerAvoidanceAction::Execute(Event event)
{
    if (!botAI->IsHeal(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    LOG_DEBUG("playerbots", "[HighKingMaulgarHealerAvoidanceAction] {} is executing Healer Avoidance Action", bot->GetName().c_str());

    // Find Kiggler to check distance
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    
    // Check if we need to move because:
    // 1. We're in an unsafe position (whirlwind or blast wave), OR
    // 2. We're too close to Kiggler (less than 30 yards)
    bool needToMove = !IsPositionSafe(botAI, bot, bot->GetPosition());
    if (kiggler && kiggler->IsAlive())
    {
        float distanceToKiggler = bot->GetDistance(kiggler);
        if (distanceToKiggler < 30.0f)
        {
            needToMove = true;
            LOG_DEBUG("playerbots", "[HighKingMaulgarHealerAvoidanceAction] {} is too close to Kiggler ({} yards), needs to move", 
                      bot->GetName().c_str(), distanceToKiggler);
        }
    }
    
    if (needToMove)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarHealerAvoidanceAction] {} is moving to safe position", bot->GetName().c_str());
        
        // Calculate the raid center as a reference point
        float centerX = 0, centerY = 0, centerZ = 0;
        uint32 count = 0;
        
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot) 
                continue;
            
            centerX += member->GetPositionX();
            centerY += member->GetPositionY();
            centerZ += member->GetPositionZ();
            count++;
        }
        
        if (count == 0) // No other alive players
            return false;
            
        centerX /= count;
        centerY /= count;
        centerZ /= count;
        
        // Find the player closest to center to use as reference
        Player* centerPlayer = nullptr;
        float minDistToCenter = 9999.0f;
        
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot) 
                continue;
            
            float dist = sqrt(pow(member->GetPositionX() - centerX, 2) + 
                             pow(member->GetPositionY() - centerY, 2) + 
                             pow(member->GetPositionZ() - centerZ, 2));
                             
            if (dist < minDistToCenter)
            {
                minDistToCenter = dist;
                centerPlayer = member;
            }
        }
        
        // Use the player closest to center as our reference point
        if (centerPlayer)
        {
            // Find a safe position that's also far enough from Kiggler
            Position safePos = FindSafePosition(botAI, bot, centerPlayer, 30.0f);
            
            // Do an additional check to make sure our safe position is far enough from Kiggler
            if (kiggler && kiggler->IsAlive())
            {
                float safeDistToKiggler = sqrt(pow(safePos.GetPositionX() - kiggler->GetPositionX(), 2) +
                                             pow(safePos.GetPositionY() - kiggler->GetPositionY(), 2));
                
                // If the position isn't far enough from Kiggler, adjust it
                if (safeDistToKiggler < 31.0f)
                {
                    LOG_DEBUG("playerbots", "[HighKingMaulgarHealerAvoidanceAction] Safe position still too close to Kiggler ({} yards), adjusting", 
                              safeDistToKiggler);
                    
                    // Calculate vector from Kiggler to safe position
                    float dx = safePos.GetPositionX() - kiggler->GetPositionX();
                    float dy = safePos.GetPositionY() - kiggler->GetPositionY();
                    
                    // Normalize and extend to ensure 31+ yard distance
                    float distance = sqrt(dx*dx + dy*dy);
                    if (distance < 0.001f) // Avoid division by zero
                    {
                        dx = 1.0f;
                        dy = 0.0f;
                        distance = 1.0f;
                    }
                    
                    dx /= distance;
                    dy /= distance;
                    
                    float newX = kiggler->GetPositionX() + dx * 30.0f;
                    float newY = kiggler->GetPositionY() + dy * 30.0f;
                    
                    // Keep the same Z coordinate from the original safe position
                    safePos.m_positionX = newX;
                    safePos.m_positionY = newY;
                }
            }
            
            return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
    }
    
    return false;
}

bool HighKingMaulgarBanishFelstalkerAction::Execute(Event event)
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    if (botAI->CanCastSpell("banish", felStalker))
    {
        return botAI->CastSpell("banish", felStalker);
    }

    return false;
}

bool HighKingMaulgarBanishFelstalkerAction::isUseful()
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    return felStalker && felStalker->IsAlive() && bot->getClass() == CLASS_WARLOCK;
} 

// Fel stalkers are enslaved successfully but break out an instant later; logs confirm the enslave aura is never applied--why?
/* bool HighKingMaulgarControlFelstalkerAction::Execute(Event event)
{
    Pet* felStalker = bot->GetPet();

    LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Pet pointer: {}, IsAlive: {}, HasEnslave: {}",
    felStalker ? "valid" : "null",
    felStalker && felStalker->IsAlive() ? "yes" : "no",
    felStalker && felStalker->HasAura(SPELL_ENSLAVE_DEMON) ? "yes" : "no");

    if (felStalker && felStalker->IsAlive() && felStalker->HasAura(SPELL_ENSLAVE_DEMON))
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] ReactState: {}", felStalker->GetReactState());
        LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Current victim: {}", felStalker->GetVictim() ? felStalker->GetVictim()->GetName() : "none");
        LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Position: X={}, Y={}, Z={}", felStalker->GetPositionX(), felStalker->GetPositionY(), felStalker->GetPositionZ());

        if (felStalker->GetReactState() != REACT_AGGRESSIVE)
        {
            felStalker->SetReactState(REACT_AGGRESSIVE);
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Set felstalker to aggressive react state");
        }
        // Enable autocast for Wild Bite if appropriate
        const SpellInfo* wildBite = sSpellMgr->GetSpellInfo(SPELL_WILD_BITE);
        if (wildBite && wildBite->IsAutocastable())
        {
            bool isAutoCast = false;
            for (unsigned int& m_autospell : felStalker->m_autospells)
            {
                if (m_autospell == SPELL_WILD_BITE)
                {
                    isAutoCast = true;
                    break;
                }
            }
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Wild Bite autocast: {}", isAutoCast ? "enabled" : "disabled");
            if (!isAutoCast)
            {
                felStalker->ToggleAutocast(wildBite, true);
                LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Enabled autocast for Wild Bite");
            }
        }
        if (felStalker->HasAura(SPELL_AURA_DARK_DECAY) && !felStalker->HasSpellCooldown(SPELL_DETERMINATION))
        {
            felStalker->CastSpell(felStalker, SPELL_DETERMINATION, true);
            felStalker->AddSpellCooldown(SPELL_DETERMINATION, 0, 10 * 1000);
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Felstalker cast Determination on itself to remove Death and Decay.");
            return false;
        }
        Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
        LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Olm pointer: {}, IsAlive: {}",
            olm ? "valid" : "null",
            olm && olm->IsAlive() ? "yes" : "no");

        if (olm && olm->IsAlive())
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Distance to Olm: {}", felStalker->GetDistance(olm));
            if (felStalker->GetVictim() != olm)
            {
                felStalker->Attack(olm, true);
                LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Commanded felstalker to attack Olm");
            }

            if (felStalker->GetDistance(olm) <= 20.0f && !felStalker->HasSpellCooldown(SPELL_THREATEN))
            {
                felStalker->CastSpell(olm, SPELL_THREATEN, true);
                felStalker->AddSpellCooldown(SPELL_THREATEN, 0, 20 * 1000);
                LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Cast Threaten on Olm");
                return false;
            }
        }
        Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
        LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Krosh pointer: {}, IsAlive: {}",
            krosh ? "valid" : "null",
            krosh && krosh->IsAlive() ? "yes" : "no");

        if (krosh && krosh->IsAlive() && olm && !olm->IsAlive())
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Distance to Krosh: {}", felStalker->GetDistance(krosh));
            if (felStalker->GetVictim() != krosh)
            {
                felStalker->Attack(krosh, true);
                LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Commanded felstalker to attack Krosh");
            }

            if (felStalker->GetDistance(krosh) <= 20.0f && !felStalker->HasSpellCooldown(SPELL_THREATEN))
            {
                felStalker->CastSpell(krosh, SPELL_THREATEN, true);
                felStalker->AddSpellCooldown(SPELL_THREATEN, 0, 20 * 1000);
                LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Cast Threaten on Krosh");
                return false;
            }
        }

        return false;
    }
    else
    {
        if (bot->getClass() != CLASS_WARLOCK)
            return false;
            
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Felstalker pointer: {}, IsAlive: {}, HasEnslave: {}",
        felStalker ? "valid" : "null",
        felStalker && felStalker->IsAlive() ? "yes" : "no",
        felStalker && felStalker->HasAura(SPELL_ENSLAVE_DEMON) ? "yes" : "no");

        if (!felStalker)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] No fel stalker found.");
            return false;
        }
        if (!felStalker->IsAlive())
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Fel stalker is not alive.");
            return false;
        }
        if (felStalker->HasAura(SPELL_ENSLAVE_DEMON))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Fel stalker is already enslaved.");
            return false;
        }

        float distance = bot->GetDistance2d(felStalker);
        LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Distance to fel stalker: {}", distance);

        if (distance > sPlayerbotAIConfig->spellDistance)
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Moving closer to fel stalker (distance: {}, spellDistance: {}).", distance, sPlayerbotAIConfig->spellDistance);
            return MoveNear(felStalker, sPlayerbotAIConfig->spellDistance, MovementPriority::MOVEMENT_COMBAT);
        }

        if (botAI->CanCastSpell("enslave demon", felStalker))
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Casting Enslave Demon on fel stalker.");

            bool castResult = botAI->CastSpell("enslave demon", felStalker);
            LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Enslave Demon cast result: {}", castResult ? "success" : "failure");
            if (felStalker->HasAura(SPELL_ENSLAVE_DEMON))
            {
                LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Felstalker has Enslave Demon aura after cast.");
                LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] bot->GetPet() GUID: {}", bot->GetPet() ? bot->GetPet()->GetGUID().ToString().c_str() : "none");
                LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] felStalker GUID: {}", felStalker->GetGUID().ToString().c_str());
                return true;
            }
            else
            {
                LOG_DEBUG("playerbots", "[HighKingMaulgarControlFelstalkerAction] Felstalker does NOT have Enslave Demon aura after cast.");
            }
        }
    }
    return false;
}

bool HighKingMaulgarControlFelstalkerAction::isUseful()
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    return felStalker && felStalker->IsAlive() && bot->getClass() == CLASS_WARLOCK;
} */

// Misdirection doesn't set a visible aura on the target so I need to figure out if there is another approach
// Potentially cast after combat starts but need to prioritize targets
/* bool HighKingMaulgarHunterMisdirectionAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
        return false;
    
    LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] {} is executing Hunter Misdirection Action", bot->GetName().c_str());

    // Find all boss targets
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");

    // Check aura abilities
    LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] {} has 'aimed shot': {}, has 'steady shot': {}", 
              bot->GetName().c_str(), botAI->CanCastSpell("aimed shot", bot) ? "yes" : "no", 
              botAI->CanCastSpell("steady shot", bot) ? "yes" : "no");

    bool foundMisdirection = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot) continue;
        
        // Log member details
        LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] Checking member {} for misdirection", 
                  member->GetName().c_str());

        Aura* aura = member->GetAura(SPELL_AURA_MISDIRECTION);
        if (!aura) 
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] {} does not have aura with ID {}", 
                      member->GetName().c_str(), SPELL_AURA_MISDIRECTION);
            continue;
        }

        if (aura->GetCasterGUID() != bot->GetGUID())
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] {} has misdirection, but from {} not from {}", 
                      member->GetName().c_str(), aura->GetCasterGUID().ToString().c_str(), bot->GetGUID().ToString().c_str());
            continue;
        }
        
        foundMisdirection = true;
        LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] Found misdirection on {} from {}", 
                  member->GetName().c_str(), bot->GetName().c_str());
        
        // First tank with Misdirection - should attack Maulgar
        if (IsFirstTank(botAI, member) && maulgar && maulgar->IsAlive())
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] {} casting shots at Maulgar due to MD on first tank", bot->GetName().c_str());
            // Force target to Maulgar regardless of current target
            if (bot->GetTarget() != maulgar->GetGUID())
                bot->SetTarget(maulgar->GetGUID());
            
            // Try Aimed Shot first, but only if the hunter has it
            if (botAI->CanCastSpell("aimed shot", maulgar))
                botAI->CastSpell("aimed shot", maulgar);
                
            // Always try Steady Shot as a fallback
            botAI->CastSpell("steady shot", maulgar);
            return true;
        }
        
        // Second tank with Misdirection - should attack Olm
        else if (IsSecondTank(botAI, member) && olm && olm->IsAlive())
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] {} casting shots at Olm due to MD on second tank", bot->GetName().c_str());
            // Force target to Olm regardless of current target
            if (bot->GetTarget() != olm->GetGUID())
                bot->SetTarget(olm->GetGUID());
            
            // Try Aimed Shot first, but only if the hunter has it
            if (botAI->CanCastSpell("aimed shot", olm))
                botAI->CastSpell("aimed shot", olm);

            // Always try Steady Shot as a fallback
            botAI->CastSpell("steady shot", olm);
            return true;
        }
        
        // Third tank with Misdirection - should attack Blindeye
        else if (IsThirdTank(botAI, member) && blindeye && blindeye->IsAlive())
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] {} casting shots at Blindeye due to MD on third tank", bot->GetName().c_str());
            // Force target to Blindeye regardless of current target
            if (bot->GetTarget() != blindeye->GetGUID())
                bot->SetTarget(blindeye->GetGUID());
            
            // Try Aimed Shot first, but only if the hunter has it
            if (botAI->CanCastSpell("aimed shot", blindeye))
                botAI->CastSpell("aimed shot", blindeye);

            // Always try Steady Shot as a fallback
            botAI->CastSpell("steady shot", blindeye);
            return true;
        }
        
        // Mage tank with Misdirection - should attack Krosh
        else if (IsMageTank(botAI, member) && krosh && krosh->IsAlive())
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] {} casting shots at Krosh due to MD on mage tank", bot->GetName().c_str());
            // Force target to Krosh regardless of current target
            if (bot->GetTarget() != krosh->GetGUID())
                bot->SetTarget(krosh->GetGUID());
            
            // Try Aimed Shot first, but only if the hunter has it
            if (botAI->CanCastSpell("aimed shot", krosh))
                botAI->CastSpell("aimed shot", krosh);

            // Always try Steady Shot as a fallback
            botAI->CastSpell("steady shot", krosh);
            return true;
        }

        // Moonkin tank with Misdirection - should attack Kiggler
        else if (IsMoonkinTank(botAI, member) && kiggler && kiggler->IsAlive())
        {
            LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] {} casting shots at Kiggler due to MD on moonkin tank", bot->GetName().c_str());
            // Force target to Kiggler regardless of current target
            if (bot->GetTarget() != kiggler->GetGUID())
                bot->SetTarget(kiggler->GetGUID());
            
            // Try Aimed Shot first, but only if the hunter has it
            if (botAI->CanCastSpell("aimed shot", kiggler))
                botAI->CastSpell("aimed shot", kiggler);

            // Always try Steady Shot as a fallback
            botAI->CastSpell("steady shot", kiggler);
            return true;
        }
    }
    if (!foundMisdirection)
    {
        LOG_DEBUG("playerbots", "[HighKingMaulgarHunterMisdirectionAction] No members have misdirection from {}", 
                bot->GetName().c_str());
    }
    // No player found with Misdirection from this hunter
    return false;
} */

bool GruulTheDragonkillerPositionBossAction::Execute(Event event)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    
    const TankSpot& tankSpot = GruulsLairTankSpots::Gruul;
    const float maxDistance = 5.0f;
    float distanceToTankSpot = gruul->GetExactDist2d(tankSpot.x, tankSpot.y);

    if (distanceToTankSpot > maxDistance)
    {
        float dX = tankSpot.x - gruul->GetPositionX();
        float dY = tankSpot.y - gruul->GetPositionY();
        float moveX = tankSpot.x + (dX / distanceToTankSpot) * maxDistance;
        float moveY = tankSpot.y + (dY / distanceToTankSpot) * maxDistance;

        float moveDistance = bot->GetExactDist2d(moveX, moveY);
        if (moveDistance < 0.5f)
        {
            return false;
        }

        return MoveTo(bot->GetMapId(), moveX, moveY, tankSpot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }
    
    float desiredOrientation = atan2(gruul->GetPositionY() - bot->GetPositionY(), gruul->GetPositionX() - bot->GetPositionX());
    float currentOrientation = bot->GetOrientation();
    float delta = desiredOrientation - currentOrientation;
    while (delta > M_PI)
        delta -= 2 * M_PI;
    while (delta < -M_PI)
        delta += 2 * M_PI;
    float orientationDifference = fabs(static_cast<double>(delta));

    const float orientationLeeway = 30.0f * M_PI / 180.0f;
    if (orientationDifference > orientationLeeway)
    {
        bot->SetFacingTo(desiredOrientation);
    }

    return false;   
}

bool GruulTheDragonkillerPositionBossAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && botAI->IsTank(bot) && 
           botAI->HasAggro(gruul) && gruul->GetVictim() == bot;
}

/* bool GruulTheDragonkillerSpreadMeleeAction::Execute(Event event)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    float gruulMeleeRadius = 15.0f; // Gruul's hitbox radius
    float minSpreadDistance = 5.0f; // Minimum distance to maintain from other players
    float movementThreshold = 2.0f;
    Unit* closestMember = nullptr;

    // Get all group members
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || !unit->IsAlive() || unit == bot)
            continue;

        // Find the closest group member
        if (!closestMember || bot->GetExactDist2d(unit) < bot->GetExactDist2d(closestMember))
        {
            closestMember = unit;
        }
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    float distanceToGruul = bot->GetExactDist2d(gruul);

    if (distanceToGruul > gruulMeleeRadius || 
        distanceToGruul < gruulMeleeRadius - 4.0f - movementThreshold)
    {
        return MoveTo(gruul->GetMapId(), gruul->GetPositionX(), gruul->GetPositionY(), 
               gruul->GetPositionZ(), gruulMeleeRadius);
    }

    return false;
}

bool GruulTheDragonkillerSpreadMeleeAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    // Don't want tanks to be moving around so spreading is just for melee DPS
    return gruul && gruul->IsAlive() && botAI->IsMelee(bot) && !botAI->IsTank(bot);
} */

bool GruulTheDragonkillerSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (gruul && gruul->IsAlive() && gruul->GetHealth() == gruul->GetMaxHealth())
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static const TankSpot& tankSpot = GruulsLairTankSpots::Gruul; // Use ideal tanking coordinates

    if (initialPositions.find(bot->GetGUID()) == initialPositions.end())
    {
        // Use the ideal tanking coordinates as the center
        float centerX = tankSpot.x;
        float centerY = tankSpot.y;
        float centerZ = bot->GetPositionZ();

        // Define a range for the radius
        float minRadius = 25.0f; // Minimum distance from the center
        float maxRadius = 40.0f; // Maximum distance from the center

        uint32 count = 0;
        uint32 botIndex = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsRanged(member))
                continue;

            if (member == bot)
                botIndex = count;

            count++;
        }

        if (count == 0)
            return false;

        float angle = 2 * M_PI * botIndex / count; // Divide bots evenly around the ideal spot
        float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius); // Randomize radius within range
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, centerZ);
        hasReachedInitialPosition[bot->GetGUID()] = false; // Initialize the flag to false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            return MoveTo(bot->GetMapId(), targetPosition.GetPositionX(), targetPosition.GetPositionY(), targetPosition.GetPositionZ());
        }

        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    float gruulRangedRadius = 25.0f; // Minimum distance to maintain from Gruul
    float minSpreadDistance = 10.0f; // Minimum distance to maintain from other players
    float movementThreshold = 2.0f;
    Unit* closestMember = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member || !member->IsAlive() || member == bot || !botAI->IsRanged(member))
            continue;

        if (!closestMember || bot->GetExactDist2d(member) < bot->GetExactDist2d(closestMember))
            closestMember = member;
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    float distanceToGruul = bot->GetExactDist2d(tankSpot.x, tankSpot.y);
    
    if (distanceToGruul < gruulRangedRadius - 3.0f - movementThreshold)
    {
        return MoveTo(bot->GetMapId(), tankSpot.x, tankSpot.y, tankSpot.z);
    }

    return false;
}

bool GruulTheDragonkillerSpreadRangedAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && botAI->IsRanged(bot);
}

bool GruulTheDragonkillerShatterSpreadAction::Execute(Event event)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    float radius = 22.0f;
    Unit* closestMember = nullptr;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || bot->GetGUID() == member)
        {
            continue;
        }
        if (!closestMember || bot->GetExactDist2d(unit) < bot->GetExactDist2d(closestMember))
        {
            closestMember = unit;
        }
        return MoveAway(closestMember, 6.0f);
    }
    return false;
}

bool GruulTheDragonkillerShatterSpreadAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && 
           (bot->HasAura(SPELL_AURA_GROUND_SLAM_1) || bot->HasAura(SPELL_AURA_GROUND_SLAM_2));
}
