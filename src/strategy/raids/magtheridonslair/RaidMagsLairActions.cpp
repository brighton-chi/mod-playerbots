#include <unordered_map>
#include <ctime>

#include "RaidMagsLairActions.h"
#include "RaidMagsLairHelpers.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

#include "Log.h"

static std::unordered_map<uint32, time_t> magtheridonSpreadWaitTimers;

bool MagtheridonHellfireChannelerEastTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
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

        /* if (!bot->IsWithinMeleeRange(channelerTriangle))
        {
            return MoveTo(channelerTriangle->GetMapId(), channelerTriangle->GetPositionX(), channelerTriangle->GetPositionY(), channelerTriangle->GetPositionZ());
        } */
    }

    return false;
}

bool MagtheridonHellfireChannelerEastTankAction::isUseful()
{
    if (!IsEastTank(botAI, bot))
        return false;

    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    return channelerTriangle && channelerTriangle->IsAlive();
}

bool MagtheridonHellfireChannelerSouthTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    // If all are dead, nothing to do
    if ((!channelerSquare || !channelerSquare->IsAlive()) &&
        (!channelerStar   || !channelerStar->IsAlive()) &&
        (!channelerCircle || !channelerCircle->IsAlive()))
        return false;

    // Mark all alive channelers with RTI
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

    // Priority: square > star > circle
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

    // Set RTI and target
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

        /* if (!bot->IsWithinMeleeRange(currentTarget))
        {
            return MoveTo(currentTarget->GetMapId(), currentTarget->GetPositionX(), currentTarget->GetPositionY(), currentTarget->GetPositionZ());
        } */
    }

    return false;
}

bool MagtheridonHellfireChannelerSouthTankAction::isUseful()
{
    if (!IsSouthTank(botAI, bot))
        return false;

    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return ((channelerSquare && channelerSquare->IsAlive()) || (channelerStar && channelerStar->IsAlive()) || 
            (channelerCircle && channelerCircle->IsAlive()));
}

bool MagtheridonHellfireChannelerWestTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);
    if (!channelerDiamond || !channelerDiamond->IsAlive())
    {
        return false;
    }

    // Mark with diamond icon
    ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channelerDiamond->GetGUID())
    {
        group->SetTargetIcon(diamondIcon, bot->GetGUID(), channelerDiamond->GetGUID());
    }

    // Set RTI value and target
    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != channelerDiamond)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerDiamond);
    }

    if (bot->GetVictim() != channelerDiamond)
    {
        Attack(channelerDiamond);

        /* if (!bot->IsWithinMeleeRange(channelerDiamond))
        {
            return MoveTo(channelerDiamond->GetMapId(), channelerDiamond->GetPositionX(), channelerDiamond->GetPositionY(), channelerDiamond->GetPositionZ());
        } */
    }

    return false;
}

bool MagtheridonHellfireChannelerWestTankAction::isUseful()
{
    if (!IsWestTank(botAI, bot))
        return false;

    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    return channelerDiamond && channelerDiamond->IsAlive();
}

/* bool MagtheridonHellfireChannelerEastWarlockAction::Execute(Event event)
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
}*/

bool MagtheridonHellfireChannelerMisdirectionAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Find the first two alive hunters in the group
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_HUNTER)
            continue;
        hunters.push_back(member);
        if (hunters.size() == 2)
            break;
    }

    if (hunters.size() < 2)
        return false;

    Player* firstHunter = hunters[0];
    Player* secondHunter = hunters[1];

    // Find south tank
    Player* southTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (IsSouthTank(botAI, member))
        {
            southTank = member;
            break;
        }
    }

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);   // west channeler
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER); // east channeler

    bool actionTaken = false;

    // First hunter: misdirect to south tank, steady shot star
    if (bot == firstHunter && southTank && channelerStar && channelerStar->IsAlive())
    {
        if (channelerStar->GetVictim() != southTank)
        {
            if (botAI->CanCastSpell("misdirection", southTank))
                botAI->CastSpell("misdirection", southTank);

            if (!bot->HasAura(SPELL_AURA_MISDIRECTION))
                return actionTaken;

            if (botAI->CanCastSpell("steady shot", channelerStar))
            {
                botAI->CastSpell("steady shot", channelerStar);
                actionTaken = true;
            }
        }
    }

    // Second hunter: misdirect to south tank, steady shot circle
    if (bot == secondHunter && southTank && channelerCircle && channelerCircle->IsAlive())
    {
        if (channelerCircle->GetVictim() != southTank)
        {
            if (botAI->CanCastSpell("misdirection", southTank))
                botAI->CastSpell("misdirection", southTank);

            if (!bot->HasAura(SPELL_AURA_MISDIRECTION))
                return actionTaken;

            if (botAI->CanCastSpell("steady shot", channelerCircle))
            {
                botAI->CastSpell("steady shot", channelerCircle);
                actionTaken = true;
            }
        }
    }

    return actionTaken;
}

bool MagtheridonHellfireChannelerMisdirectionAction::isUseful()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return (channelerStar && channelerStar->IsAlive()) || 
           (channelerCircle && channelerCircle->IsAlive());
}

bool MagtheridonHellfireChannelerDPSPriorityAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

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

    // Target priority 3: East Channeler
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

    // Target priority 4: Northwest Channeler
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

    // Target priority 5: Northeast Channeler
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
    if (magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(SPELL_SHADOW_CAGE) &&
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
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    if (botAI->IsHeal(bot) || 
        (IsSouthTank(botAI, bot) && 
        (channelerSquare->IsAlive() || channelerStar->IsAlive() || channelerCircle->IsAlive())) ||
        (IsWestTank(botAI, bot) && channelerDiamond->IsAlive()) ||
        (IsEastTank(botAI, bot) && channelerTriangle->IsAlive()))
        return false;
    
    if ((channeler && channeler->IsAlive()) ||
        (magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(SPELL_SHADOW_CAGE)))
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
            LOG_DEBUG("playerbots", "CCBurningAbyssalAction: Already banishing an abyssal (bot={})", bot->GetName());
            break;
        }
    }

    // Track if we are already fearing an abyssal
    bool isAlreadyFearing = false;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL && unit->HasAura(SPELL_FEAR))
        {
            isAlreadyFearing = true;
            LOG_DEBUG("playerbots", "CCBurningAbyssalAction: Already fearing an abyssal (bot={})", bot->GetName());
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
                    LOG_DEBUG("playerbots", "CCBurningAbyssalAction: Casting Banish on abyssal (bot={})", bot->GetName());
                    botAI->CastSpell(SPELL_BANISH, unit);
                    return false;
                }
            }
        }
    }

    // 2. If not already fearing, fear the first unbanished, unfeared abyssal
    if (!isAlreadyFearing)
    {
        for (const auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL &&
                !unit->HasAura(SPELL_BANISH) && !unit->HasAura(SPELL_FEAR))
            {
                if (unit->IsAlive() && botAI->CanCastSpell(SPELL_FEAR, unit, true))
                {
                    LOG_DEBUG("playerbots", "CCBurningAbyssalAction: Casting Fear on abyssal (bot={})", bot->GetName());
                    botAI->CastSpell(SPELL_FEAR, unit);
                    return false;
                }
            }
        }
    }

    // 3. Curse of Tongues on any channeler without it
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_HELLFIRE_CHANNELER)
        {
            if (!unit->HasAura(SPELL_CURSE_OF_TONGUES))
            {
                if (botAI->CanCastSpell(SPELL_CURSE_OF_TONGUES, unit, true))
                {
                    LOG_DEBUG("playerbots", "CCBurningAbyssalAction: Casting Curse of Tongues on channeler (bot={})", bot->GetName());
                    botAI->CastSpell(SPELL_CURSE_OF_TONGUES, unit);
                    return false;
                }
            }
            else
            {
                LOG_DEBUG("playerbots", "CCBurningAbyssalAction: Channeler already has Curse of Tongues (bot={}, channeler={})", bot->GetName(), unit->GetName());
            }
        }
    }

    /* Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
    if (rtiTarget && rtiTarget->IsAlive())
    {
        LOG_DEBUG("playerbots", "CCBurningAbyssalAction: {} fallback attacking RTI target {}", bot->GetName(), rtiTarget->GetName());
        Attack(rtiTarget);
        return false;
    } */

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
        if (unit && (unit->GetEntry() == NPC_BURNING_ABYSSAL || 
            unit->GetEntry() == NPC_HELLFIRE_CHANNELER))
            return true;
    }
    return false;
}

bool MagtheridonPositionBossAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    static const TankSpot& tankSpot = MagtheridonTankSpot;
    const float maxStep = 5.0f;

    float dX = tankSpot.x - bot->GetPositionX();
    float dY = tankSpot.y - bot->GetPositionY();
    float distanceToTankSpot = bot->GetExactDist2d(tankSpot.x, tankSpot.y);

    if (distanceToTankSpot > maxStep)
    {
        float moveX = bot->GetPositionX() + (dX / distanceToTankSpot) * maxStep;
        float moveY = bot->GetPositionY() + (dY / distanceToTankSpot) * maxStep;
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

    return magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(SPELL_SHADOW_CAGE) && 
           botAI->HasAggro(magtheridon) && magtheridon->GetVictim() == bot;
}

bool MagtheridonSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->HasAura(SPELL_SHADOW_CAGE))
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const float roomCenterX = -19.804f;
    const float roomCenterY = 0.708f;

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

        // Spread evenly in a full circle around the room center
        float arcRadians = 2.0f * M_PI; // 360 degree arc
        float startAngle = 0.0f;
        float angleStep = rangedBots.size() > 0 ? arcRadians / rangedBots.size() : 0.0f;
        float angle = startAngle + botIndex * angleStep;

        float minRadius = 5.0f;
        float maxRadius = 20.0f;
        float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius);

        float targetX = roomCenterX + radius * cos(angle);
        float targetY = roomCenterY + radius * sin(angle);
        float targetZ = bot->GetPositionZ();

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, targetZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        float destX = targetPosition.GetPositionX();
        float destY = targetPosition.GetPositionY();
        float destZ = bot->GetPositionZ(); // Use bot's current Z
        if (!bot->IsWithinDist2d(destX, destY, 2.0f) && 
            bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
        {
            return MoveTo(bot->GetMapId(), destX, destY, destZ);
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    float magtheridonRangedRadius = 15.0f;
    float minSpreadDistance = 8.0f;
    float movementThreshold = 2.0f;

    Unit* closestMember = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member || !member->IsAlive() || member == bot)
            continue; // Consider all alive bots except self

        if (!closestMember || bot->GetExactDist2d(member) < bot->GetExactDist2d(closestMember))
            closestMember = member;
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    // Keep distance from Magtheridon (use his current position)
    float magX = magtheridon->GetPositionX();
    float magY = magtheridon->GetPositionY();
    float distanceToMagtheridon = bot->GetExactDist2d(magX, magY);

    float destX2 = magX;
    float destY2 = magY;
    float destZ2 = bot->GetPositionZ();
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
    if (!magtheridon || magtheridon->HasAura(SPELL_SHADOW_CAGE) || !botAI->IsRanged(bot) || bot->HasAura(SPELL_SHADOW_GRASP))
        return false;

    // Timer logic: wait X seconds after Shadow Cage ends
    static bool lastShadowCage = false;
    bool shadowCage = magtheridon->HasAura(SPELL_SHADOW_CAGE);
    if (lastShadowCage && !shadowCage)
    {
        magtheridonSpreadWaitTimers[bot->GetMapId()] = time(nullptr);
    }
    lastShadowCage = shadowCage;

    const int spreadWaitSeconds = 10;
    auto it = magtheridonSpreadWaitTimers.find(bot->GetMapId());
    if (it != magtheridonSpreadWaitTimers.end())
    {
        time_t since = time(nullptr) - it->second;
        if (since < spreadWaitSeconds)
            return false;
    }

    // Inline cube action exclusion logic
    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        static std::unordered_map<ObjectGuid, time_t> cubeTimers;
        time_t now = time(nullptr);
        time_t lastCubeUse = cubeTimers[bot->GetGUID()];
        bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);

        if ((lastCubeUse == 0 || (now - lastCubeUse < 45)) || blastNovaActive)
        {
            LOG_DEBUG("playerbots", "SpreadRangedAction: Disabled for cube clicker (bot={}) due to cube action active", bot->GetName());
            return false;
        }
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

bool MagtheridonSpreadHealerAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float magtheridonHealerRadius = 15.0f;
    float minSpreadDistance = 8.0f;
    float movementThreshold = 2.0f;

    Unit* closestMember = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member || !member->IsAlive() || member == bot)
            continue;

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
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), destX2, destY2, destZ2))
    {
        return MoveTo(bot->GetMapId(), destX2, destY2, destZ2);
    }

    return false;
}


bool MagtheridonSpreadHealerAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || magtheridon->HasAura(SPELL_SHADOW_CAGE) || !botAI->IsHeal(bot) || bot->HasAura(SPELL_SHADOW_GRASP))
        return false;

    // Inline cube action exclusion logic
    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        static std::unordered_map<ObjectGuid, time_t> cubeTimers;
        time_t now = time(nullptr);
        time_t lastCubeUse = cubeTimers[bot->GetGUID()];
        bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);

        if ((lastCubeUse == 0 || (now - lastCubeUse < 45)) || blastNovaActive)
        {
            LOG_DEBUG("playerbots", "SpreadHealerAction: Disabled for cube clicker (bot={}) due to cube action active", bot->GetName());
            return false;
        }
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

    if (botToCubeAssignment.empty())
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);
            AssignBotsToCubesByGuidAndCoords(group, cubes, botAI);
            LOG_DEBUG("playerbots", "CubeAction: Assigned bots to cubes (group size={}, cubes={})", group->GetMembersCount(), cubes.size());
        }
    }

    if (bot->HasAura(SPELL_SHADOW_GRASP)) 
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} has SHADOW_GRASP, exiting", bot->GetName());
        return false;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !magtheridon->IsAlive()) 
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} cannot find or Magtheridon is dead, exiting", bot->GetName());
        return false;
    }

    if (magtheridon->HasAura(SPELL_SHADOW_CAGE)) 
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} Magtheridon is caged, resetting timer and exiting", bot->GetName());
        cubeTimers[bot->GetGUID()] = 0;
        return false;
    }

    auto it = botToCubeAssignment.find(bot->GetGUID());
    if (it == botToCubeAssignment.end()) 
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} not assigned to a cube, exiting", bot->GetName());
        return false;
    }

    const CubeInfo& cubeInfo = it->second;
    GameObject* cube = botAI->GetGameObject(cubeInfo.guid);
    if (!cube) 
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} cannot find assigned cube, exiting", bot->GetName());
        return false;
    }

    time_t now = time(nullptr);
    if (cubeTimers[bot->GetGUID()] == 0) 
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} initializing timer", bot->GetName());
        cubeTimers[bot->GetGUID()] = now;
    }

    const float safeWaitDistance = 7.0f;
    float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

    if (now - cubeTimers[bot->GetGUID()] < 45) 
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} waiting for 45s timer, elapsed={}s", bot->GetName(), now - cubeTimers[bot->GetGUID()]);
        return false;
    }

    if (!(magtheridon->HasUnitState(UNIT_STATE_CASTING) && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))) {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} entering safe wait area logic, cubeDist={}", bot->GetName(), cubeDist);
        if (fabs(cubeDist - safeWaitDistance) > 0.5f) {
            LOG_DEBUG("playerbots", "CubeAction: Bot {} not at safe wait location, searching for safe spot", bot->GetName());
            // Try up to 12 positions around the cube
            for (int i = 0; i < 12; ++i)
            {
                float angle = i * M_PI / 6.0f; // 12 spots around the circle
                float targetX = cubeInfo.x + cos(angle) * safeWaitDistance;
                float targetY = cubeInfo.y + sin(angle) * safeWaitDistance;
                float targetZ = bot->GetPositionZ();

                if (IsSafeFromMagtheridonHazards(botAI, bot, targetX, targetY, targetZ))
                {
                    LOG_DEBUG("playerbots", "CubeAction: Bot {} moving to safe wait spot ({}, {})", bot->GetName(), targetX, targetY);
                    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
                }
            }
            // If no safe spot found, fallback to default position
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
            float fallbackX = cubeInfo.x + cos(angle) * safeWaitDistance;
            float fallbackY = cubeInfo.y + sin(angle) * safeWaitDistance;
            float fallbackZ = bot->GetPositionZ();
            LOG_DEBUG("playerbots", "CubeAction: Bot {} fallback to wait spot ({}, {})", bot->GetName(), fallbackX, fallbackY);
            return MoveTo(bot->GetMapId(), fallbackX, fallbackY, fallbackZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
        else
        {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} at safe wait location, idling", bot->GetName());
        return true;
        }
    }

    LOG_DEBUG("playerbots", "CubeAction: Bot {} entering cube use logic, cubeDist={}", bot->GetName(), cubeDist);

    // 3. If Blast Nova is casting: move to cube and use it
    const float interactDistance = 1.0f;
    const float interactDistanceBuffer = 0.5f;

    if (cubeDist > interactDistance) 
    {
        if (cubeDist <= interactDistance + interactDistanceBuffer) {
            LOG_DEBUG("playerbots", "CubeAction: Bot {} close enough to use cube, using", bot->GetName());
            bot->StopMoving();
            cube->Use(bot);
            cubeTimers[bot->GetGUID()] = now;
            return true;
        }
        LOG_DEBUG("playerbots", "CubeAction: Bot {} moving to cube at ({}, {})", bot->GetName(), cubeInfo.x, cubeInfo.y);
        float angle = atan2(cubeInfo.y - bot->GetPositionY(), cubeInfo.x - bot->GetPositionX());
        float targetX = cubeInfo.x - cos(angle) * interactDistance;
        float targetY = cubeInfo.y - sin(angle) * interactDistance;
        float targetZ = bot->GetPositionZ();

        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, MovementPriority::MOVEMENT_FORCED);
    }
    else 
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} at cube, using cube", bot->GetName());
        bot->StopMoving();
        cube->Use(bot);
        cubeTimers[bot->GetGUID()] = now;
        return true;
    }
}
