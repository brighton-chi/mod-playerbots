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
        return false;
    
    ObjectGuid currentIconGuid = group->GetTargetIcon(squareIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != maulgar->GetGUID())
    {
        group->SetTargetIcon(squareIcon, bot->GetGUID(), maulgar->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "square" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != maulgar)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(maulgar);
        botAI->DoSpecificAction("attack rti target");
    }
    
    if (bot->GetVictim() == maulgar && maulgar->GetVictim() == bot)
    {
        // Use TankSpot for designated coordinates
        const TankSpot& spot = GruulsLairTankSpots::Maulgar;
        const float maxDistance = 3.0f;
        float distanceToMaulgar = maulgar->GetExactDist2d(spot.x, spot.y);

        float moveX = spot.x;
        float moveY = spot.y;
        if (distanceToMaulgar > maxDistance)
        {
            float dX = spot.x - maulgar->GetPositionX();
            float dY = spot.y - maulgar->GetPositionY();
            moveX = spot.x + (dX / distanceToMaulgar) * maxDistance;
            moveY = spot.y + (dY / distanceToMaulgar) * maxDistance;
            MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
        float orientation = atan2(maulgar->GetPositionY() - bot->GetPositionY(), maulgar->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return true;
    }
    return false;
}

/*{
    if (!IsFirstTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;
    std::vector<std::pair<std::string, int>> priorities = 
    {
        {"maulgar", 5}, // Square
        {"blindeye", 0}, // Star
        {"olm", 1}, // Circle
        {"kiggler", 2}, // Diamond
        // Krosh is 3,  Triangle
    };

    Unit* maulgar = nullptr;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

        if (icon == "maulgar" && target && target->IsAlive())
        {
            maulgar = target;
            group->SetTargetIcon(index, bot->GetGUID(), target->GetGUID());
        }

        if (target && target->IsAlive())
        {
            // Only reprioritize if not already targeting this icon and target
            std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
            Unit* currentVictim = bot->GetVictim();
            LOG_INFO("playerbots", "Gruul's Lair: {} comparing current RTI '{}' to icon '{}' ({}), current victim: {}, target: {}", 
                    bot->GetName(), currentRti.c_str(), GetRtiNameFromInternalId(icon).c_str(), icon.c_str(), 
                    (currentVictim ? currentVictim->GetName() : "none"), 
                    target->GetName());

            if (currentRti == GetRtiNameFromInternalId(icon) && currentVictim == target)
                // Already targeting correct icon and target, do nothing
                return true;

            // Otherwise, set RTI and switch target
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(GetRtiNameFromInternalId(icon));
            bot->SetTarget(target->GetGUID());
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
            bot->Attack(target, true);
            LOG_INFO("playerbots", "Gruul's Lair: {} direct bot->Attack() command for {}", bot->GetName(), target->GetName());
            break;
        }
        // If not found/alive, continue to next icon
    }
    // After targeting loop, handle maulgar special movement
    if (maulgar && bot->GetVictim() == maulgar && maulgar->GetVictim() == bot)
    {
        // Use TankSpot for designated coordinates
        const TankSpot& spot = GruulsLairTankSpots::Maulgar;
        const float maxDistance = 3.0f;
        float distanceToMaulgar = maulgar->GetExactDist2d(spot.x, spot.y);

        float moveX = spot.x;
        float moveY = spot.y;
        if (distanceToMaulgar > maxDistance)
        {
            float dX = spot.x - maulgar->GetPositionX();
            float dY = spot.y - maulgar->GetPositionY();
            moveX = spot.x + (dX / distanceToMaulgar) * maxDistance;
            moveY = spot.y + (dY / distanceToMaulgar) * maxDistance;
        }
        MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        float orientation = atan2(maulgar->GetPositionY() - bot->GetPositionY(), maulgar->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return true;
    }
    // No valid target found
    return false;
}*/

bool HighKingMaulgarOlmTankAction::Execute(Event event)
{
    if (!IsSecondTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (!olm || !olm->IsAlive())
        return false;

    ObjectGuid currentIconGuid = group->GetTargetIcon(circleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != olm->GetGUID())
    {
        group->SetTargetIcon(circleIcon, bot->GetGUID(), olm->GetGUID());
    }

    /* std::vector<std::pair<std::string, std::string>> priorities = 
    {
        {"olm", "circle"},
        {"blindeye", "star"},
        {"kiggler", "diamond"},
        {"maulgar", "square"},
    };

    Unit* olm = nullptr;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

        if (icon == "olm" && target && target->IsAlive())
        {
            olm = target;
        }

        if (target && target->IsAlive())
        {
            // Only reprioritize if not already targeting this icon and target
            std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
            if (currentRti == icon && bot->GetVictim() == target)
            {
                // Already targeting correct icon and target, do nothing
                return true;
            }
            // Otherwise, set RTI and switch target
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(icon);
            bot->SetTarget(target->GetGUID());
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
            botAI->DoSpecificAction("attack rti target");
            return true;
        }
        // If not found/alive, continue to next icon
    }
    // After targeting loop, handle olm special movement
    if (olm && bot->GetVictim() == olm && olm->GetVictim() == bot)
    {
        // Use TankSpot for designated coordinates
        const TankSpot& spot = GruulsLairTankSpots::Olm;
        const float maxDistance = 3.0f;
        float distanceToOlm = olm->GetExactDist2d(spot.x, spot.y);

        float moveX = spot.x;
        float moveY = spot.y;
        if (distanceToOlm > maxDistance)
        {
            float dX = spot.x - olm->GetPositionX();
            float dY = spot.y - olm->GetPositionY();
            moveX = spot.x + (dX / distanceToOlm) * maxDistance;
            moveY = spot.y + (dY / distanceToOlm) * maxDistance;
        }
        MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        float orientation = atan2(olm->GetPositionY() - bot->GetPositionY(), olm->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return true;
    } */
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
        return false;

    ObjectGuid currentIconGuid = group->GetTargetIcon(starIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != blindeye->GetGUID())
    {
        group->SetTargetIcon(starIcon, bot->GetGUID(), blindeye->GetGUID());
    }

    /* std::vector<std::pair<std::string, std::string>> priorities = 
    {
        {"blindeye", "star"},
        {"olm", "circle"},
        {"kiggler", "diamond"},
        {"maulgar", "square"},
    };

    Unit* blindeye = nullptr;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

        if (icon == "blindeye" && target && target->IsAlive())
        {
            blindeye = target;
        }

        if (target && target->IsAlive())
        {
            // Only reprioritize if not already targeting this icon and target
            std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
            if (currentRti == icon && bot->GetVictim() == target)
            {
                // Already targeting correct icon and target, do nothing
                return true;
            }
            // Otherwise, set RTI and switch target
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(icon);
            bot->SetTarget(target->GetGUID());
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
            botAI->DoSpecificAction("attack rti target");
            return true;
        }
        // If not found/alive, continue to next icon
    }
    // After targeting loop, handle blindeye special movement
    if (blindeye && bot->GetVictim() == blindeye && blindeye->GetVictim() == bot)
    {
        // Use TankSpot for designated coordinates
        const TankSpot& spot = GruulsLairTankSpots::Blindeye;
        const float maxDistance = 3.0f;
        float distanceToBlindeye = blindeye->GetExactDist2d(spot.x, spot.y);

        float moveX = spot.x;
        float moveY = spot.y;
        if (distanceToBlindeye > maxDistance)
        {
            float dX = spot.x - blindeye->GetPositionX();
            float dY = spot.y - blindeye->GetPositionY();
            moveX = spot.x + (dX / distanceToBlindeye) * maxDistance;
            moveY = spot.y + (dY / distanceToBlindeye) * maxDistance;
        }
        MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        float orientation = atan2(blindeye->GetPositionY() - bot->GetPositionY(), blindeye->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return true;
    } */
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
        return false;

    ObjectGuid currentIconGuid = group->GetTargetIcon(triangleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != krosh->GetGUID())
    {
        group->SetTargetIcon(triangleIcon, bot->GetGUID(), krosh->GetGUID());
    }

    /* std::vector<std::pair<std::string, std::string>> priorities = 
    {
        {"krosh", "triangle"},
        {"blindeye", "star"},
        {"olm", "circle"},
        {"kiggler", "diamond"},
        {"maulgar", "square"},
    };

    Unit* krosh = nullptr;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

        if (icon == "krosh" && target && target->IsAlive())
        {
            krosh = target;
        }

        if (target && target->IsAlive())
        {
            // Only reprioritize if not already targeting this icon and target
            std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
            if (currentRti == icon && bot->GetVictim() == target)
            {
                // Already targeting correct icon and target, do nothing
                return true;
            }
            // Otherwise, set RTI and switch target
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(icon);
            bot->SetTarget(target->GetGUID());
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
            botAI->DoSpecificAction("attack rti target");
            return true;
        }
        // If not found/alive, continue to next icon
    }
    // After targeting loop, handle triangle special movement if triangle is targeting the bot
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
    } */
    return false;
}

bool HighKingMaulgarBoomkinTankAction::Execute(Event event)
{
    if (!IsBoomkinTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler || !kiggler->IsAlive())
        return false;

    ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != kiggler->GetGUID())
    {
        group->SetTargetIcon(diamondIcon, bot->GetGUID(), kiggler->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != kiggler)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
        botAI->DoSpecificAction("attack rti target");
        return true;
    }
    return false;
}

bool HighKingMaulgarMeleeDPSAction::Execute(Event event)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (!botAI->IsMelee(bot) || (IsFirstTank(botAI, bot) && maulgar->IsAlive()) || 
        (IsSecondTank(botAI, bot) && olm->IsAlive()) || (IsThirdTank(botAI, bot) && blindeye->IsAlive()))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (blindeye && blindeye->IsAlive() && botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "star" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != blindeye)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(blindeye);
        botAI->DoSpecificAction("attack rti target");
        return true;
    }

    if (olm && olm->IsAlive() && botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "circle" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != olm)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(olm);
        botAI->DoSpecificAction("attack rti target");
        return true;
    }

    if (kiggler && kiggler->IsAlive() && botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != kiggler)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
        botAI->DoSpecificAction("attack rti target");
        return true;
    }

    if (maulgar && maulgar->IsAlive() && botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "square" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != maulgar)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(maulgar);
        botAI->DoSpecificAction("attack rti target");
        return true;
    }
    return false;
}

bool HighKingMaulgarRangedDPSAction::Execute(Event event) // Need separate action for Warlock
{

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (!botAI->IsRanged(bot) || (IsMageTank(botAI, bot) && krosh->IsAlive()) || 
        (IsBoomkinTank(botAI, bot) && kiggler->IsAlive()) || botAI->IsHeal(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (blindeye && blindeye->IsAlive() && botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "star" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != blindeye)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(blindeye);
        botAI->DoSpecificAction("attack rti target");
        return true;
    }

    if (olm && olm->IsAlive() && botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "circle" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != olm)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(olm);
        botAI->DoSpecificAction("attack rti target");
        return true;
    }
    
    if (kiggler && kiggler->IsAlive() && botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != kiggler)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
        botAI->DoSpecificAction("attack rti target");
        return true;
    }

    if (krosh && krosh->IsAlive() && botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "triangle" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != krosh)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(krosh);
        botAI->DoSpecificAction("attack rti target");
        return true;
    }

    if (maulgar && maulgar->IsAlive() && botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "square" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != maulgar)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(maulgar);
        botAI->DoSpecificAction("attack rti target");
        return true;
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
}

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
} */
