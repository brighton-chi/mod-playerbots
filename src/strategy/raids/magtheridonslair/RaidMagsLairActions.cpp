#include "RaidMagsLairActions.h"
#include "RaidMagsLairHelpers.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Timer.h"

#include "Log.h"

bool MagtheridonHellfireChannelerEastTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Creature* channelerTriangle = GetChanneler(bot, EAST_CHANNELER);
    if (!channelerTriangle || !channelerTriangle->IsAlive())
    {
        return false;
    }

    // Mark with triangle icon
    ObjectGuid currentIconGuid = group->GetTargetIcon(triangleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channelerTriangle->GetGUID())
    {
        group->SetTargetIcon(triangleIcon, bot->GetGUID(), channelerTriangle->GetGUID());
    }

    // Set RTI value and target
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
            return MoveTo(channelerTriangle->GetMapId(), channelerTriangle->GetPositionX(), channelerTriangle->GetPositionY(), channelerTriangle->GetPositionZ());
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerEastTankAction::isUseful()
{
    if (!IsEastTank(botAI, bot))
        return false;

    Creature* channelerTriangle = GetChanneler(bot, EAST_CHANNELER);

    return channelerTriangle && channelerTriangle->IsAlive();
}

bool MagtheridonHellfireChannelerSouthTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);

    if ((!channelerSquare || !channelerSquare->IsAlive()) && (!channelerStar || !channelerStar->IsAlive()))
        return false;

    // Target selection logic (swap every 6 seconds, or always target the alive one if the other is dead)
    static uint8 currentTargetIndex = 0;
    static time_t lastSwapTime = 0;
    time_t now = time(nullptr);

    if (!channelerSquare || !channelerSquare->IsAlive())
        currentTargetIndex = 1;
    else if (!channelerStar || !channelerStar->IsAlive())
        currentTargetIndex = 0;
    else if (now - lastSwapTime >= 6)
    {
        currentTargetIndex = (currentTargetIndex + 1) % 2;
        lastSwapTime = now;
    }

    // Mark both channelers
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

    // Set RTI and target
    Creature* currentTarget = (currentTargetIndex == 0) ? channelerSquare : channelerStar;
    std::string rtiName = (currentTargetIndex == 0) ? "square" : "star";
    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != currentTarget)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(currentTarget);
    }

    if (currentTarget && bot->GetVictim() != currentTarget)
    {
        Attack(currentTarget);

        if (!bot->IsWithinMeleeRange(currentTarget))
        {
            return MoveTo(currentTarget->GetMapId(), currentTarget->GetPositionX(), currentTarget->GetPositionY(), currentTarget->GetPositionZ());
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerSouthTankAction::isUseful()
{
    if (!IsSouthTank(botAI, bot))
        return false;

    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);

    return ((channelerSquare && channelerSquare->IsAlive()) || (channelerStar && channelerStar->IsAlive()));
}

bool MagtheridonHellfireChannelerWestTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerDiamond = GetChanneler(bot, NORTHEAST_CHANNELER);

    if ((!channelerCircle || !channelerCircle->IsAlive()) && (!channelerDiamond || !channelerDiamond->IsAlive()))
        return false;

    // Target selection logic (swap every 6 seconds, or always target the alive one if the other is dead)
    static uint8 currentTargetIndex = 0;
    static time_t lastSwapTime = 0;
    time_t now = time(nullptr);

    if (!channelerCircle || !channelerCircle->IsAlive())
        currentTargetIndex = 1;
    else if (!channelerDiamond || !channelerDiamond->IsAlive())
        currentTargetIndex = 0;
    else if (now - lastSwapTime >= 6)
    {
        currentTargetIndex = (currentTargetIndex + 1) % 2;
        lastSwapTime = now;
    }

    // Mark both channelers
    if (channelerCircle && channelerCircle->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(circleIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerCircle->GetGUID())
            group->SetTargetIcon(circleIcon, bot->GetGUID(), channelerCircle->GetGUID());
    }
    if (channelerDiamond && channelerDiamond->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerDiamond->GetGUID())
            group->SetTargetIcon(diamondIcon, bot->GetGUID(), channelerDiamond->GetGUID());
    }

    // Set RTI and target
    Creature* currentTarget = (currentTargetIndex == 0) ? channelerCircle : channelerDiamond;
    std::string rtiName = (currentTargetIndex == 0) ? "circle" : "diamond";
    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != currentTarget)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(currentTarget);
    }

    if (currentTarget && bot->GetVictim() != currentTarget)
    {
        Attack(currentTarget);

        if (!bot->IsWithinMeleeRange(currentTarget))
        {
            return MoveTo(currentTarget->GetMapId(), currentTarget->GetPositionX(), currentTarget->GetPositionY(), currentTarget->GetPositionZ());
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerWestTankAction::isUseful()
{
    if (!IsWestTank(botAI, bot))
        return false;

    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerDiamond = GetChanneler(bot, NORTHEAST_CHANNELER);

    return (channelerCircle && channelerCircle->IsAlive()) || (channelerDiamond && channelerDiamond->IsAlive());
}

bool MagtheridonHellfireChannelerEastWarlockAction::Execute(Event event)
{
    Creature* channelerTriangle = GetChanneler(bot, EAST_CHANNELER);

    std::string rtiName;
    Unit* rtiTarget = nullptr;

    if (channelerTriangle && channelerTriangle->IsAlive())
    {
        rtiName = "triangle";
        rtiTarget = channelerTriangle;
    }

    // Set RTI value and target if needed
    if (!rtiName.empty() &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != rtiTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(rtiTarget);
    }

    if (channelerTriangle && channelerTriangle->IsAlive())
    {
        if (!channelerTriangle->HasAura(SPELL_CURSE_OF_TONGUES) && botAI->CanCastSpell(SPELL_CURSE_OF_TONGUES, channelerTriangle, true))
        {
            botAI->CastSpell(SPELL_CURSE_OF_TONGUES, channelerTriangle);
            return true;
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerEastWarlockAction::isUseful()
{
    if (!IsEastWarlock(botAI, bot))
        return false;

    Creature* channelerTriangle = GetChanneler(bot, EAST_CHANNELER);
    return channelerTriangle && channelerTriangle->IsAlive();
}

bool MagtheridonHellfireChannelerSouthWarlockAction::Execute(Event event)
{
    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);

    std::string rtiName;
    Unit* rtiTarget = nullptr;

    if (channelerSquare && channelerSquare->IsAlive())
    {
        rtiName = "square";
        rtiTarget = channelerSquare;
    }
    else if (channelerStar && channelerStar->IsAlive())
    {
        rtiName = "star";
        rtiTarget = channelerStar;
    }

    if (!rtiName.empty() &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != rtiTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(rtiTarget);
    }

    Creature* moveTarget = nullptr;
    if (channelerSquare && channelerSquare->IsAlive())
        moveTarget = channelerSquare;
    else if (channelerStar && channelerStar->IsAlive())
        moveTarget = channelerStar;

    if (moveTarget && bot->GetExactDist2d(moveTarget) > 30.0f)
    {
        return MoveTo(moveTarget->GetMapId(), moveTarget->GetPositionX(), moveTarget->GetPositionY(), moveTarget->GetPositionZ());
    }

    for (Creature* channeler : {channelerSquare, channelerStar})
    {
        if (channeler && channeler->IsAlive() &&
            !channeler->HasAura(SPELL_CURSE_OF_TONGUES) &&
            botAI->CanCastSpell(SPELL_CURSE_OF_TONGUES, channeler, true))
        {
            botAI->CastSpell(SPELL_CURSE_OF_TONGUES, channeler);
            return true;
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerSouthWarlockAction::isUseful()
{
    if (!IsSouthWarlock(botAI, bot))
        return false;

    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);

    return (channelerSquare && channelerSquare->IsAlive()) || (channelerStar && channelerStar->IsAlive());
}

bool MagtheridonHellfireChannelerWestWarlockAction::Execute(Event event)
{
    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerDiamond = GetChanneler(bot, NORTHEAST_CHANNELER);

    std::string rtiName;
    Unit* rtiTarget = nullptr;
    if (channelerCircle && channelerCircle->IsAlive())
    {
        rtiName = "circle";
        rtiTarget = channelerCircle;
    }
    else if (channelerDiamond && channelerDiamond->IsAlive())
    {
        rtiName = "diamond";
        rtiTarget = channelerDiamond;
    }

    if (!rtiName.empty() &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
         botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != rtiTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(rtiTarget);
    }

    Creature* moveTarget = nullptr;
    if (channelerCircle && channelerCircle->IsAlive())
        moveTarget = channelerCircle;
    else if (channelerDiamond && channelerDiamond->IsAlive())
        moveTarget = channelerDiamond;

    if (moveTarget && bot->GetExactDist2d(moveTarget) > 30.0f)
    {
        return MoveTo(moveTarget->GetMapId(), moveTarget->GetPositionX(), moveTarget->GetPositionY(), moveTarget->GetPositionZ());
    }

    for (Creature* channeler : {channelerCircle, channelerDiamond})
    {
        if (channeler && channeler->IsAlive() &&
            !channeler->HasAura(SPELL_CURSE_OF_TONGUES) &&
            botAI->CanCastSpell(SPELL_CURSE_OF_TONGUES, channeler, true))
        {
            botAI->CastSpell(SPELL_CURSE_OF_TONGUES, channeler);
            return true;
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerWestWarlockAction::isUseful()
{
    if (!IsWestWarlock(botAI, bot))
        return false;

    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerDiamond = GetChanneler(bot, NORTHEAST_CHANNELER);

    return (channelerCircle && channelerCircle->IsAlive()) || (channelerDiamond && channelerDiamond->IsAlive());
}

/* bool MagtheridonHellfireChannelerWestHunterAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        LOG_DEBUG("playerbots", "WestHunterAction: No group (bot={})", bot->GetName());
        return false;
    }

    Player* westTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (IsWestTank(botAI, member))
        {
            westTank = member;
            break;
        }
    }
    LOG_DEBUG("playerbots", "WestHunterAction: westTank={}", westTank ? westTank->GetName() : "nullptr");

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);

    std::string rtiName;
    Unit* rtiTarget = nullptr;

    if (channelerStar && channelerStar->IsAlive())
    {
        rtiName = "star";
        rtiTarget = channelerStar;
    }
    else if (channelerCircle && channelerCircle->IsAlive())
    {
        rtiName = "circle";
        rtiTarget = channelerCircle;
    }

    LOG_DEBUG("playerbots", "WestHunterAction: rtiName={} rtiTarget={}", rtiName, rtiTarget ? rtiTarget->GetName() : "nullptr");

    // Set RTI value and target if needed
    if (!rtiName.empty() &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != rtiTarget))
    {
        LOG_DEBUG("playerbots", "WestHunterAction: Setting RTI to {} (bot={})", rtiName, bot->GetName());
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(rtiTarget);
    }

    // Prefer circle channeler if alive, else star channeler
    Creature* moveTarget = nullptr;
    if (channelerCircle && channelerCircle->IsAlive())
        moveTarget = channelerCircle;
    else if (channelerStar && channelerStar->IsAlive())
        moveTarget = channelerStar;

    LOG_DEBUG("playerbots", "WestHunterAction: moveTarget={}", moveTarget ? moveTarget->GetName() : "nullptr");

    if (moveTarget && bot->GetExactDist2d(moveTarget) > 35.0f)
    {
        LOG_DEBUG("playerbots", "WestHunterAction: Moving to moveTarget {} (bot={})", moveTarget->GetName(), bot->GetName());
        return MoveTo(moveTarget->GetMapId(), moveTarget->GetPositionX(), moveTarget->GetPositionY(), moveTarget->GetPositionZ());
    }

    if (channelerCircle && channelerCircle->IsAlive() && westTank && westTank->IsAlive())
    {
        if (channelerCircle->GetVictim() != westTank)
        {
            LOG_DEBUG("playerbots", "WestHunterAction: Misdirecting to westTank {} for channelerCircle (bot={})", westTank->GetName(), bot->GetName());
            if (botAI->CanCastSpell("misdirection", westTank))
            {
                botAI->CastSpell("misdirection", westTank);
            }

            if (!bot->HasAura(SPELL_AURA_MISDIRECTION))
            {
                return false;
            }

            if (botAI->CanCastSpell("steady shot", channelerCircle))
            {
                LOG_DEBUG("playerbots", "WestHunterAction: Casting steady shot on channelerCircle (bot={})", bot->GetName());
                botAI->CastSpell("steady shot", channelerCircle);
                return true;
            }
        }
    }

    if (channelerStar && channelerStar->IsAlive() && westTank && westTank->IsAlive())
    {
        if (channelerStar->GetVictim() != westTank)
        {
            LOG_DEBUG("playerbots", "WestHunterAction: Misdirecting to westTank {} for channelerStar (bot={})", westTank->GetName(), bot->GetName());
            if (botAI->CanCastSpell("misdirection", westTank))
            {
                botAI->CastSpell("misdirection", westTank);
            }

            if (!bot->HasAura(SPELL_AURA_MISDIRECTION))
            {
                return false;
            }

            if (botAI->CanCastSpell("steady shot", channelerStar))
            {
                LOG_DEBUG("playerbots", "WestHunterAction: Casting steady shot on channelerStar (bot={})", bot->GetName());
                botAI->CastSpell("steady shot", channelerStar);
                return true;
            }
        }
    }
            
    return false;
}

/* bool MagtheridonHellfireChannelerWestHunterAction::isUseful()
{
    if (!IsWestHunter(botAI, bot))
        return false;

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);

    return (channelerStar && channelerStar->IsAlive()) || (channelerCircle && channelerCircle->IsAlive());
}

bool MagtheridonHellfireChannelerEastHunterAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* eastTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (IsEastTank(botAI, member))
        {
            eastTank = member;
            break;
        }
    }

    Creature* channelerDiamond  = GetChanneler(bot, NORTHEAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, EAST_CHANNELER);

    std::string rtiName;
    Unit* rtiTarget = nullptr;

    if (channelerDiamond && channelerDiamond->IsAlive())
    {
        rtiName = "diamond";
        rtiTarget = channelerDiamond;
    }
    else if (channelerTriangle && channelerTriangle->IsAlive())
    {
        rtiName = "triangle";
        rtiTarget = channelerTriangle;
    }

    // Set RTI value and target if needed
    if (!rtiName.empty() &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
         botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != rtiTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(rtiTarget);
    }

    // Prefer triangle channeler if alive, else diamond channeler
    Creature* moveTarget = nullptr;
    if (channelerTriangle && channelerTriangle->IsAlive())
        moveTarget = channelerTriangle;
    else if (channelerDiamond && channelerDiamond->IsAlive())
        moveTarget = channelerDiamond;

    if (moveTarget && bot->GetExactDist2d(moveTarget) > 35.0f)
    {
        return MoveTo(moveTarget->GetMapId(), moveTarget->GetPositionX(), moveTarget->GetPositionY(), moveTarget->GetPositionZ());
    }

    if (channelerTriangle && channelerTriangle->IsAlive() && eastTank && eastTank->IsAlive())
    {
        if (channelerTriangle->GetVictim() != eastTank)
        {
            if (botAI->CanCastSpell("misdirection", eastTank))
            {
                botAI->CastSpell("misdirection", eastTank);
            }

            if (!bot->HasAura(SPELL_AURA_MISDIRECTION))
            {
                return false;
            }

            if (botAI->CanCastSpell("steady shot", channelerTriangle))
            {
                botAI->CastSpell("steady shot", channelerTriangle);
                return true;
            }
        }
    }

    if (channelerDiamond && channelerDiamond->IsAlive() && eastTank && eastTank->IsAlive())
    {
        if (channelerDiamond->GetVictim() != eastTank)
        {
            if (botAI->CanCastSpell("misdirection", eastTank))
            {
                botAI->CastSpell("misdirection", eastTank);
            }

            if (!bot->HasAura(SPELL_AURA_MISDIRECTION))
            {
                return false;
            }

            if (botAI->CanCastSpell("steady shot", channelerDiamond))
            {
                botAI->CastSpell("steady shot", channelerDiamond);
                return true;
            }
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerEastHunterAction::isUseful()
{
    if (!IsEastHunter(botAI, bot))
        return false;

    Creature* channelerDiamond  = GetChanneler(bot, NORTHEAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, EAST_CHANNELER);

    return (channelerDiamond && channelerDiamond->IsAlive()) || (channelerTriangle && channelerTriangle->IsAlive());
} */

bool MagtheridonHellfireChannelerDPSPriorityAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHEAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, EAST_CHANNELER);

    // Target priority 1: South Channeler
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

    // Target priority 2: West Channeler
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

    // Target priority 3: Northwest Channeler
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

    // Target priority 4: Northeast Channeler
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

    // Target priority 5: East Channeler
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

    // Target priority 6: Magtheridon
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(SPELL_AURA_SHADOW_CAGE) &&
       (!channelerSquare || !channelerSquare->IsAlive()) &&
       (!channelerStar || !channelerStar->IsAlive()) &&
       (!channelerCircle || !channelerCircle->IsAlive()) &&
       (!channelerDiamond || !channelerDiamond->IsAlive()) &&
       (!channelerTriangle || !channelerTriangle->IsAlive()))
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            ObjectGuid currentIconGuid = group->GetTargetIcon(crossIcon);
            if (currentIconGuid.IsEmpty() || currentIconGuid != magtheridon->GetGUID())
            {
                LOG_DEBUG("playerbots", "DPSPriorityAction: Marking Magtheridon with cross (bot={})", bot->GetName());
                group->SetTargetIcon(crossIcon, bot->GetGUID(), magtheridon->GetGUID());
            }
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
    }

    return false;
}

bool MagtheridonHellfireChannelerDPSPriorityAction::isUseful()
{
    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar     = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerCircle   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHEAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, EAST_CHANNELER);
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    if (botAI->IsHeal(bot) || 
        ((IsSouthTank(botAI, bot) || IsSouthWarlock(botAI, bot)) && 
        (channelerSquare->IsAlive() || channelerStar->IsAlive())) ||
        ((IsWestWarlock(botAI, bot) || IsWestTank(botAI, bot)) &&
        (channelerCircle->IsAlive() || channelerDiamond->IsAlive())) ||
        ((IsEastWarlock(botAI, bot) || IsEastTank(botAI, bot)) &&
        channelerTriangle->IsAlive()))
        return false;
    
    if ((channeler && channeler->IsAlive()) ||
        (magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(SPELL_AURA_SHADOW_CAGE)))
        return true;

    return false;
}

bool MagtheridonCCBurningAbyssalAction::Execute(Event event)
{
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    // Track if we are already banishing an abyssal
    bool isAlreadyBanishing = false;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL && unit->HasAura(SPELL_BANISH))
        {
            isAlreadyBanishing = true;
            LOG_DEBUG("playerbots", "CCInfernalAction: Already banishing an abyssal (bot={})", bot->GetName());
            break;
        }
    }

    // 1. If not already banishing, banish the first unbanished abyssal
    if (!isAlreadyBanishing)
    {
        for (const auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL && !unit->HasAura(SPELL_BANISH))
            {
                if (unit->IsAlive() && botAI->CanCastSpell(SPELL_BANISH, unit, true))
                {
                    LOG_DEBUG("playerbots", "CCInfernalAction: Casting Banish on abyssal (bot={})", bot->GetName());
                    botAI->CastSpell(SPELL_BANISH, unit);
                    return true;
                }
            }
        }
    }

    // 2. Fear the first unbanished, unfeared abyssal
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL &&
            !unit->HasAura(SPELL_BANISH) && !unit->HasAura(SPELL_FEAR))
        {
            if (unit->IsAlive() && botAI->CanCastSpell(SPELL_FEAR, unit, true))
            {
                LOG_DEBUG("playerbots", "CCInfernalAction: Casting Fear on abyssal (bot={})", bot->GetName());
                botAI->CastSpell(SPELL_FEAR, unit);
                return true;
            }
        }
    }

    return false;
}

bool MagtheridonCCBurningAbyssalAction::isUseful()
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_WARLOCK)
    {
        return false;
    }

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL &&
           (!unit->HasAura(SPELL_BANISH) || !unit->HasAura(SPELL_FEAR)))
        {
            return true;
        }
    }
    return false;
}

bool MagtheridonPositionBossAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    static const TankSpot& tankSpot = MagtheridonTankSpot;
    const float maxStep = 5.0f;
    float distanceToTankSpot = magtheridon->GetExactDist2d(tankSpot.x, tankSpot.y);

    if (distanceToTankSpot > maxStep)
    {
        float dX = tankSpot.x - magtheridon->GetPositionX();
        float dY = tankSpot.y - magtheridon->GetPositionY();
        float moveX = magtheridon->GetPositionX() + (dX / distanceToTankSpot) * maxStep;
        float moveY = magtheridon->GetPositionY() + (dY / distanceToTankSpot) * maxStep;
        float moveZ = tankSpot.z;

        float moveDistance = bot->GetExactDist2d(moveX, moveY);
        if (moveDistance < 2.0f)
        {
            LOG_DEBUG("playerbots", "PositionBossAction: Already at move target (bot={})", bot->GetName());
            return false;
        }

        // Collision/path check
        float destX = moveX;
        float destY = moveY;
        float destZ = moveZ;
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
        {
            LOG_DEBUG("playerbots", "PositionBossAction: Collision detected, cannot move to intermediate spot (bot={})", bot->GetName());
            return false;
        }
        LOG_DEBUG("playerbots", "PositionBossAction: Moving stepwise to ({}, {}) (bot={})", destX, destY, bot->GetName());
        bool moveResult = MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        LOG_DEBUG("playerbots", "PositionBossAction: MoveTo({}, {}, {}, {}) result={} (bot={})", destX, destY, destZ, bot->GetMapId(), moveResult, bot->GetName());
        return moveResult;
    }

    float currentOrientation = bot->GetOrientation();
    float delta = tankSpot.orientation - currentOrientation;
    while (delta > M_PI)
        delta -= 2 * M_PI;
    while (delta < -M_PI)
        delta += 2 * M_PI;
    float orientationDifference = fabs(static_cast<double>(delta));

    const float orientationLeeway = 15.0f * M_PI / 180.0f;
    if (orientationDifference > orientationLeeway)
    {
        LOG_DEBUG("playerbots", "PositionBossAction: Adjusting facing to tank spot (bot={})", bot->GetName());
        bot->SetFacingTo(tankSpot.orientation);
    }
    else
    {
        LOG_DEBUG("playerbots", "PositionBossAction: Already facing tank spot (bot={})", bot->GetName());
    }

    return false; 
}

bool MagtheridonPositionBossAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    return magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(SPELL_AURA_SHADOW_CAGE) && 
           bot->HasAggro(magtheridon) && magtheridon->GetVictim() == bot;
}

bool MagtheridonSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && !magtheridon->HasAura(SPELL_AURA_SHADOW_CAGE) && magtheridon->GetHealth() == magtheridon->GetMaxHealth())
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const TankSpot& tankSpot = MagtheridonTankSpot;

    // Gather all ranged bots in the group
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            continue;
        rangedBots.push_back(member);
    }

    // Assign initial positions if not already done
    if (initialPositions.find(bot->GetGUID()) == initialPositions.end())
    {
        // Find this bot's index among ranged bots
        size_t botIndex = 0;
        for (size_t i = 0; i < rangedBots.size(); ++i)
        {
            if (rangedBots[i] == bot)
            {
                botIndex = i;
                break;
            }
        }

        // Fan parameters
        float arcDegrees = 100.0f; // 50 degrees either side
        float arcRadians = arcDegrees * M_PI / 180.0f;
        float centerAngle = tankSpot.orientation;
        float startAngle = centerAngle - arcRadians / 2.0f;
        float angleStep = rangedBots.size() > 1 ? arcRadians / (rangedBots.size() - 1) : 0.0f;
        float angle = startAngle + botIndex * angleStep;

        float minRadius = 20.0f;
        float maxRadius = 50.0f;
        float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius);

        float targetX = tankSpot.x + radius * cos(angle);
        float targetY = tankSpot.y + radius * sin(angle);
        float targetZ = bot->GetPositionZ();

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, targetZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        float destX = targetPosition.GetPositionX();
        float destY = targetPosition.GetPositionY();
        float destZ = targetPosition.GetPositionZ();
        if (!bot->IsWithinDist2d(destX, destY, 2.0f) && 
            bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
        {
            return MoveTo(bot->GetMapId(), destX, destY, destZ);
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    float magtheridonRangedRadius = 25.0f;
    float minSpreadDistance = 15.0f;
    float movementThreshold = 2.0f;
    Unit* closestMember = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member || !member->IsAlive() || member == bot || !botAI->IsRanged(member))
        {
            continue;
        }

        if (!closestMember || bot->GetExactDist2d(member) < bot->GetExactDist2d(closestMember))
        {
            closestMember = member;
        }
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    float distanceToMagtheridon = bot->GetExactDist2d(tankSpot.x, tankSpot.y);

    float destX2 = tankSpot.x;
    float destY2 = tankSpot.y;
    float destZ2 = tankSpot.z;
    if ((distanceToMagtheridon < magtheridonRangedRadius - 3.0f - movementThreshold) && 
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), destX2, destY2, destZ2))
    {
        return MoveTo(bot->GetMapId(), destX2, destY2, destZ2);
    }

    return false;
}

bool MagtheridonSpreadRangedAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || magtheridon->HasAura(SPELL_AURA_SHADOW_CAGE) || !botAI->IsRanged(bot))
    {
        return false;
    }

    // Ensure cube assignment is done
    if (botToCubeAssignment.empty())
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);
            AssignBotsToCubesByGuidAndCoords(group, cubes, botAI);
        }
    }

    // If this bot is a cube clicker, do not spread
    if (botToCubeAssignment.find(bot->GetGUID()) != botToCubeAssignment.end())
    {
        LOG_DEBUG("playerbots", "SpreadRangedAction: Disabled for cube clicker (bot={})", bot->GetName());
        return false;
    }

    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar     = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerCircle   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHEAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, EAST_CHANNELER);

    if ((channelerSquare   && channelerSquare->IsAlive())   ||
        (channelerStar     && channelerStar->IsAlive())     ||
        (channelerCircle   && channelerCircle->IsAlive())   ||
        (channelerDiamond  && channelerDiamond->IsAlive())  ||
        (channelerTriangle && channelerTriangle->IsAlive()))
    {
        return false;
    }

    return true;
}

bool MagtheridonUseManticronCubeAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, time_t> cubeTimers;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !magtheridon->IsAlive())
    {
        return false;
    }

    // 1. Assign bots to cubes if not already assigned
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    if (botToCubeAssignment.empty())
    {
        LOG_DEBUG("playerbots", "CubeAction: Assigning bots to cubes (bot={})", bot->GetName());
        std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);
        LOG_DEBUG("playerbots", "CubeAction: Found {} cubes in instance", cubes.size());
        AssignBotsToCubesByGuidAndCoords(group, cubes, botAI);
    }

    auto it = botToCubeAssignment.find(bot->GetGUID());
    if (it == botToCubeAssignment.end())
    {
        LOG_DEBUG("playerbots", "CubeAction: No cube assigned for bot={}", bot->GetName());
        return false;
    }

    const CubeInfo& cubeInfo = it->second;
    LOG_DEBUG("playerbots", "CubeAction: Assigned cube GUID={} at ({}, {}, {}) (bot={})",
        cubeInfo.guid.ToString(), cubeInfo.x, cubeInfo.y, cubeInfo.z, bot->GetName());

    // 2. Timer logic: 45s after Shadow Cage ends, move to cube
    time_t now = time(nullptr);
    bool shadowCage = magtheridon->HasAura(SPELL_AURA_SHADOW_CAGE);

    // Reset timer if Shadow Cage is up
    if (shadowCage)
    {
        LOG_DEBUG("playerbots", "CubeAction: Shadow Cage active, resetting timer (bot={})", bot->GetName());
        cubeTimers[bot->GetGUID()] = 0;
        return false;
    }

    // Start timer if not already started
    if (cubeTimers[bot->GetGUID()] == 0)
    {
        LOG_DEBUG("playerbots", "CubeAction: Starting 45s timer (bot={})", bot->GetName());
        cubeTimers[bot->GetGUID()] = now;
    }

    // 3. If timer expired, move to safe distance from cube
    const float safeWaitDistance = 10.0f;
    const float interactDistance = 3.0f;
    float botDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

    LOG_DEBUG("playerbots", "CubeAction: Timer={}s, botDist={:.2f} (bot={})",
        now - cubeTimers[bot->GetGUID()], botDist, bot->GetName());

    if (now - cubeTimers[bot->GetGUID()] >= 45)
    {
        // If too far, move closer to safe wait distance
        if (botDist > safeWaitDistance + 0.5f)
        {
            float angle = atan2(cubeInfo.y - bot->GetPositionY(), cubeInfo.x - bot->GetPositionX());
            float targetX = cubeInfo.x - cos(angle) * safeWaitDistance;
            float targetY = cubeInfo.y - sin(angle) * safeWaitDistance;
            float targetZ = cubeInfo.z;

            float destX = targetX, destY = targetY, destZ = targetZ;
            LOG_DEBUG("playerbots", "CubeAction: Moving closer to cube (target=({}, {}, {})) (bot={})",
                destX, destY, destZ, bot->GetName());
            if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
                return MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            else
                LOG_DEBUG("playerbots", "CubeAction: Path blocked to safe wait distance (bot={})", bot->GetName());
        }
    }

    // 4. If Magtheridon is casting Blast Nova, move in and use the cube
    if (magtheridon->HasUnitState(UNIT_STATE_CASTING) && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
    {
        LOG_DEBUG("playerbots", "CubeAction: Magtheridon is casting Blast Nova (bot={})", bot->GetName());
        GameObject* cube = botAI->GetGameObject(cubeInfo.guid);
        if (cube && cube->isSpawned())
        {
            float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);
            if (cube->IsWithinDistInMap(bot, interactDistance + 0.5f))
            {
                LOG_DEBUG("playerbots", "CubeAction: Ready to use cube (bot={})", bot->GetName());

                // Face the cube
                bot->SetFacingToObject(cube);

                // Stop moving if needed
                if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE)
                {
                    LOG_DEBUG("playerbots", "CubeAction: Stopping movement before using cube (bot={})", bot->GetName());
                    bot->StopMoving();
                }

                // Try to use the cube
                if (cube->isSpawned() && !cube->GetGoState())
                {
                    LOG_DEBUG("playerbots", "CubeAction: Using cube! (bot={})", bot->GetName());
                    cube->Use(bot);
                    cubeTimers[bot->GetGUID()] = now;
                    return true;
                }
                else
                {
                    LOG_DEBUG("playerbots", "CubeAction: Cube not usable right now (bot={})", bot->GetName());
                }
            }
            else if (cubeDist > interactDistance)
            {
                // Move closer
                float angle = atan2(cubeInfo.y - bot->GetPositionY(), cubeInfo.x - bot->GetPositionX());
                float targetX = cubeInfo.x - cos(angle) * interactDistance;
                float targetY = cubeInfo.y - sin(angle) * interactDistance;
                float targetZ = cubeInfo.z;

                float destX = targetX, destY = targetY, destZ = targetZ;
                LOG_DEBUG("playerbots", "CubeAction: Moving in to interact with cube (target=({}, {}, {})) (bot={})",
                    destX, destY, destZ, bot->GetName());
                if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
                    return MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, false, false, MovementPriority::MOVEMENT_FORCED);
                else
                    LOG_DEBUG("playerbots", "CubeAction: Path blocked to interact distance (bot={})", bot->GetName());
            }
        }
    }

    LOG_DEBUG("playerbots", "CubeAction: No action taken (bot={})", bot->GetName());
    return false;
}
