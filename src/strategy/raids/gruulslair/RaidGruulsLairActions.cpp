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
    std::vector<std::pair<std::string, int>> priorities = 
    {
        {"square", 5},
        {"star", 0},
        {"circle", 1},
        {"diamond", 2},
    };

    Unit* maulgar = nullptr;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);
        
        LOG_INFO("playerbots", "Gruul's Lair: {} checking icon {} (index {})", 
                 bot->GetName(), icon.c_str(), index);

        if (icon == "square" && target && target->IsAlive())
        {
            maulgar = target;
            LOG_INFO("playerbots", "Gruul's Lair: {} found maulgar at icon square: {}", 
                     bot->GetName(), target->GetName());
        }

        if (target && target->IsAlive())
        {
            LOG_INFO("playerbots", "Gruul's Lair: {} found alive target {} at icon {}", 
                     bot->GetName(), target->GetName(), icon.c_str());
                     
            // Only reprioritize if not already targeting this icon and target
            std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
            Unit* currentVictim = bot->GetVictim();
            LOG_INFO("playerbots", "Gruul's Lair: {} comparing current RTI '{}' to icon '{}', current victim: {}, target: {}", 
                    bot->GetName(), currentRti.c_str(), icon.c_str(), 
                    (currentVictim ? currentVictim->GetName() : "none"), 
                    target->GetName());

            if (currentRti == icon && currentVictim == target)
            {
                LOG_INFO("playerbots", "Gruul's Lair: {} already using correct RTI {} and targeting {}", 
                         bot->GetName(), icon.c_str(), target->GetName());
                // Already targeting correct icon and target, do nothing
                return true;
            }
            // Otherwise, set RTI and switch target
            LOG_INFO("playerbots", "Gruul's Lair: {} setting RTI to {} and targeting {}", 
                     bot->GetName(), icon.c_str(), target->GetName());
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(icon);
            bot->SetTarget(target->GetGUID());
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
            botAI->DoSpecificAction("attack rti target");
            LOG_INFO("playerbots", "Gruul's Lair: {} executing attack rti target for {}", bot->GetName(), target->GetName());
            return true;
        }
        // If not found/alive, continue to next icon
    }
    // After targeting loop, handle maulgar special movement
    LOG_INFO("playerbots", "Gruul's Lair: {} finished icon check, checking for special movement", 
             bot->GetName());
             
    if (maulgar && bot->GetVictim() == maulgar && maulgar->GetVictim() == bot)
    {
        LOG_INFO("playerbots", "Gruul's Lair: {} is tanking Maulgar, positioning at tank spot", 
                 bot->GetName());
                 
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
}

bool HighKingMaulgarOlmTankAction::Execute(Event event)
{
    if (!IsSecondTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;
    std::vector<std::pair<std::string, int>> priorities = 
    {
        {"circle", 1},
        {"star", 0},
        {"diamond", 2},
        {"square", 5},
    };

    Unit* olm = nullptr;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

        if (icon == "circle" && target && target->IsAlive())
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
    }
    // No valid target found
    return false;
}

bool HighKingMaulgarBlindeyeTankAction::Execute(Event event)
{
    if (!IsThirdTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;
    std::vector<std::pair<std::string, int>> priorities = 
    {
        {"star", 0},
        {"circle", 1},
        {"diamond", 2},
        {"square", 5},
    };

    Unit* blindeye = nullptr;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

        if (icon == "star" && target && target->IsAlive())
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
    }
    // No valid target found
    return false;
}

bool HighKingMaulgarMageTankAction::Execute(Event event)
{
    if (!IsMageTank(botAI, bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;
    std::vector<std::pair<std::string, int>> priorities = 
    {
        {"triangle", 3},
        {"star", 0},
        {"circle", 1},
        {"diamond", 2},
        {"square", 5},
    };

    Unit* krosh = nullptr;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

        if (icon == "triangle" && target && target->IsAlive())
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
    }
    // No valid target found
    return false;
}

bool HighKingMaulgarBoomkinTankAction::Execute(Event event)
{
    if (!IsBoomkinTank(botAI, bot))
        return false;

    std::vector<std::pair<std::string, int>> priorities = 
    {
        {"diamond", 2},
        {"star", 0},
        {"circle", 1},
        {"triangle", 3},
        {"square", 5}
    };

    Group* group = bot->GetGroup();
    if (!group)
        return false;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

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
    // No valid target found
    return false;
}

bool HighKingMaulgarMeleeDPSAction::Execute(Event event)
{
    if (!botAI->IsMelee(bot) || IsFirstTank(botAI, bot) || IsSecondTank(botAI, bot) || IsThirdTank(botAI, bot))
        return false;

    std::vector<std::pair<std::string, int>> priorities = {
        {"star", 0},
        {"circle", 1},
        {"diamond", 2},
        {"square", 5}
    };

    Group* group = bot->GetGroup();
    if (!group)
        return false;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

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
    // No valid target found
    return false;
}

bool HighKingMaulgarRangedDPSAction::Execute(Event event) // Need separate action for Warlock
{
    if (!botAI->IsRanged(bot) || IsMageTank(botAI, bot) || IsBoomkinTank(botAI, bot) || botAI->IsHeal(bot))
        return false;

    std::vector<std::pair<std::string, int>> priorities = {
        {"star", 0},
        {"circle", 1},
        {"diamond", 2},
        {"triangle", 3},
        {"square", 5}
    };

    Group* group = bot->GetGroup();
    if (!group)
        return false;
    for (const auto& [icon, index] : priorities)
    {
        ObjectGuid guid = group->GetTargetIcon(index);
        Unit* target = botAI->GetUnit(guid);

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
    // No valid target found
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
