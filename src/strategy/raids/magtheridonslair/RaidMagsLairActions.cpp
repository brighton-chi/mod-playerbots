#include <ctime>
#include <unordered_map>

#include "RaidMagsLairActions.h"
#include "RaidMagsLairHelpers.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

#include "Log.h"

using namespace MagsLairHelpers;
static std::unordered_map<uint32, time_t> magtheridonSpreadWaitTimer;
static std::unordered_map<uint32, time_t> magtheridonBlastNovaTimer;

bool MagtheridonHellfireChannelerMagtheridonTankAction::Execute(Event event)
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

bool MagtheridonHellfireChannelerMagtheridonTankAction::isUseful()
{
    Group* group = bot->GetGroup();
    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return group && botAI->IsMainTank(bot) && 
           (channelerSquare && channelerSquare->IsAlive() || 
           channelerStar && channelerStar->IsAlive() || 
           channelerCircle && channelerCircle->IsAlive());
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

    if (channelerDiamond->GetVictim() == bot && !channelerDiamond->IsNonMeleeSpellCast(false))
    {
        const TankSpot& spot = MagsLairTankSpots::NWChanneler;
        const float maxDistance = 3.0f;
        float distanceToSpot = channelerDiamond->GetExactDist2d(spot.x, spot.y);
        if (distanceToSpot > maxDistance)
        {
            float dX = spot.x - channelerDiamond->GetPositionX();
            float dY = spot.y - channelerDiamond->GetPositionY();
            float moveX = spot.x + (dX / distanceToSpot) * maxDistance;
            float moveY = spot.y + (dY / distanceToSpot) * maxDistance;
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerNWChannelerTankAction::isUseful()
{
    Group* group = bot->GetGroup();
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    return group && botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond && channelerDiamond->IsAlive();
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

    if (channelerTriangle->GetVictim() == bot && !channelerTriangle->IsNonMeleeSpellCast(false))
    {
        const TankSpot& spot = MagsLairTankSpots::NEChanneler;
        const float maxDistance = 3.0f;
        float distanceToSpot = channelerTriangle->GetExactDist2d(spot.x, spot.y);
        if (distanceToSpot > maxDistance)
        {
            float dX = spot.x - channelerTriangle->GetPositionX();
            float dY = spot.y - channelerTriangle->GetPositionY();
            float moveX = spot.x + (dX / distanceToSpot) * maxDistance;
            float moveY = spot.y + (dY / distanceToSpot) * maxDistance;
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerNEChannelerTankAction::isUseful()
{
    Group* group = bot->GetGroup();
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    return group && botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle && channelerTriangle->IsAlive();
}

bool MagtheridonHellfireChannelerMisdirectionAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_HUNTER)
        {
            continue;
        }
        hunters.push_back(member);
        if (hunters.size() == 2)
        {
            break;
        }
    }

    if (hunters.size() < 2)
    {
        return false;
    }

    Player* firstHunter = hunters[0];
    Player* secondHunter = hunters[1];

    Player* magtheridonTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        if (botAI->IsMainTank(member))
        {
            magtheridonTank = member;
            break;
        }
    }

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    bool actionTaken = false;

    // First hunter: misdirect to magtheridon tank, steady shot star
    if (bot == firstHunter && magtheridonTank && channelerStar && channelerStar->IsAlive())
    {
        if (channelerStar->GetVictim() != magtheridonTank)
        {
            if (botAI->CanCastSpell("misdirection", magtheridonTank))
                botAI->CastSpell("misdirection", magtheridonTank);

            if (!bot->HasAura(static_cast<uint32>(MagsLairSpells::MISDIRECTION)))
            {
                return actionTaken;
            }

            if (botAI->CanCastSpell("steady shot", channelerStar))
            {
                botAI->CastSpell("steady shot", channelerStar);
                actionTaken = true;
            }
        }
    }

    // Second hunter: misdirect to magtheridon tank, steady shot circle
    if (bot == secondHunter && magtheridonTank && channelerCircle && channelerCircle->IsAlive())
    {
        if (channelerCircle->GetVictim() != magtheridonTank)
        {
            if (botAI->CanCastSpell("misdirection", magtheridonTank))
                botAI->CastSpell("misdirection", magtheridonTank);

            if (!bot->HasAura(static_cast<uint32>(MagsLairSpells::MISDIRECTION)))
            {
                return actionTaken;
            }

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
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return group && (channelerStar && channelerStar->IsAlive() || 
           channelerCircle && channelerCircle->IsAlive());
}

bool MagtheridonHellfireChannelerDPSPriorityAction::Execute(Event event)
{
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
    if (magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)) &&
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

bool MagtheridonHellfireChannelerDPSPriorityAction::isUseful()
{
    Group* group = bot->GetGroup();
    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    if (!group || botAI->IsHeal(bot) || 
        botAI->IsMainTank(bot) && (channelerSquare->IsAlive() || channelerStar->IsAlive() || channelerCircle->IsAlive()) ||
        botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond->IsAlive() ||
        botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle->IsAlive())
    {
        return false;
    }

    return channeler && channeler->IsAlive() || 
           magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE));
}

bool MagtheridonHellfireChannelerWarlockAction::Execute(Event event)
{
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    // Track if we are already banishing an abyssal
    bool isAlreadyBanishing = false;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == static_cast<uint32>(MagsLairNPCs::BURNING_ABYSSAL) && 
            unit->HasAura(static_cast<uint32>(MagsLairSpells::BANISH)))
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
        if (unit && unit->GetEntry() == static_cast<uint32>(MagsLairNPCs::BURNING_ABYSSAL) && 
            unit->HasAura(static_cast<uint32>(MagsLairSpells::FEAR)))
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
            if (unit && unit->GetEntry() == static_cast<uint32>(MagsLairNPCs::BURNING_ABYSSAL) && 
                !unit->HasAura(static_cast<uint32>(MagsLairSpells::BANISH)))
            {
                if (unit->IsAlive() && botAI->CanCastSpell(static_cast<uint32>(MagsLairSpells::BANISH), unit, true))
                {
                    LOG_DEBUG("playerbots", "CCBurningAbyssalAction: Casting Banish on abyssal (bot={})", bot->GetName());
                    botAI->CastSpell(static_cast<uint32>(MagsLairSpells::BANISH), unit);
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
            if (unit && unit->GetEntry() == static_cast<uint32>(MagsLairNPCs::BURNING_ABYSSAL) &&
                !unit->HasAura(static_cast<uint32>(MagsLairSpells::BANISH)) && !unit->HasAura(static_cast<uint32>(MagsLairSpells::FEAR)))
            {
                if (unit->IsAlive() && botAI->CanCastSpell(static_cast<uint32>(MagsLairSpells::FEAR), unit, true))
                {
                    LOG_DEBUG("playerbots", "CCBurningAbyssalAction: Casting Fear on abyssal (bot={})", bot->GetName());
                    botAI->CastSpell(static_cast<uint32>(MagsLairSpells::FEAR), unit);
                    return false;
                }
            }
        }
    }

    // 3. Curse of Tongues on any channeler without it
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->GetEntry() == static_cast<uint32>(MagsLairNPCs::HELLFIRE_CHANNELER))
        {
            if (!unit->HasAura(static_cast<uint32>(MagsLairSpells::CURSE_OF_TONGUES)))
            {
                if (botAI->CanCastSpell(static_cast<uint32>(MagsLairSpells::CURSE_OF_TONGUES), unit, true))
                {
                    LOG_DEBUG("playerbots", "CCBurningAbyssalAction: Casting Curse of Tongues on channeler (bot={})", bot->GetName());
                    botAI->CastSpell(static_cast<uint32>(MagsLairSpells::CURSE_OF_TONGUES), unit);
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

bool MagtheridonHellfireChannelerWarlockAction::isUseful()
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_WARLOCK)
    {
        return false;
    }

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    return std::any_of(npcs.begin(), npcs.end(), [&](const ObjectGuid& npc) 
    {
        Unit* unit = botAI->GetUnit(npc);
        return unit && (unit->GetEntry() == static_cast<uint32>(MagsLairNPCs::BURNING_ABYSSAL) || 
                        unit->GetEntry() == static_cast<uint32>(MagsLairNPCs::HELLFIRE_CHANNELER));
    });
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
        return true;
    }

    if (magtheridon->GetVictim() == bot)
    {
        const TankSpot& spot = MagsLairTankSpots::Magtheridon;
        const float maxDistance = 3.0f;
        float distanceToMagtheridon = magtheridon->GetExactDist2d(spot.x, spot.y);

        if (distanceToMagtheridon > maxDistance)
        {
            float dX = spot.x - magtheridon->GetPositionX();
            float dY = spot.y - magtheridon->GetPositionY();
            float moveX = spot.x + (dX / distanceToMagtheridon) * maxDistance;
            float moveY = spot.y + (dY / distanceToMagtheridon) * maxDistance;
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT);
        }
        
        float orientation = atan2(magtheridon->GetPositionY() - bot->GetPositionY(),
                                  magtheridon->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }

    return false;
}

bool MagtheridonPositionBossAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();

    return magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)) && 
           botAI->IsMainTank(bot) && group;
}

bool MagtheridonSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)))
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const float roomCenterX = -19.804f;
    const float roomCenterY = 0.708f;

    Group* group = bot->GetGroup();
    std::vector<Player*> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
        {
            continue;
        }
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
        float destZ = bot->GetPositionZ();
        if (!bot->IsWithinDist2d(destX, destY, 3.0f) && 
            bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
                                                           bot->GetPositionZ(), destX, destY, destZ))
        {
            return MoveTo(bot->GetMapId(), destX, destY, destZ);
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    float magtheridonRangedRadius = 15.0f;
    float minSpreadDistance = 6.0f;
    float movementThreshold = 3.0f;

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
    if (magtheridon)
    {
        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)), 
                            lastShadowCageState, magtheridonSpreadWaitTimer);
        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)), 
                            lastShadowCageState, magtheridonBlastNovaTimer);
    }

    Group* group = bot->GetGroup();
    if (!group || !magtheridon->IsAlive() || magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)) ||
        !botAI->IsRanged(bot) || bot->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_GRASP)))
    {
        return false;
    }

    const int spreadWaitSeconds = 10;
    auto it = magtheridonSpreadWaitTimer.find(bot->GetMapId());
    if (it != magtheridonSpreadWaitTimer.end())
    {
        time_t since = time(nullptr) - it->second;
        if (since < spreadWaitSeconds)
        {
            return false;
        }
    }

    // Inline cube action exclusion logic
    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);
        time_t lastBlastNova = magtheridonBlastNovaTimer[bot->GetMapId()];

        // Exclude if 47s have passed since lastBlastNova (i.e., during the cube click window)
        if (now - lastBlastNova >= 47)
        {
            LOG_DEBUG("playerbots", "SpreadRangedAction: Disabled for cube clicker (bot={}) during cube click window", bot->GetName());
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

    float magtheridonHealerRadius = 15.0f;
    float minSpreadDistance = 8.0f;
    float movementThreshold = 2.0f;

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
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), destX2, destY2, destZ2))
    {
        return MoveTo(bot->GetMapId(), destX2, destY2, destZ2);
    }

    return false;
}


bool MagtheridonSpreadHealerAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon)
    {
        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)), 
                            lastShadowCageState, magtheridonBlastNovaTimer);
    }

    Group* group = bot->GetGroup();
    if (!group || !magtheridon->IsAlive() || magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)) || 
        !botAI->IsHeal(bot) || bot->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_GRASP)))
    {
        return false;
    }

    // Inline cube action exclusion logic
    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);
        time_t lastBlastNova = magtheridonBlastNovaTimer[bot->GetMapId()];

        // Exclude if 47s have passed since lastBlastNova (i.e., during the cube click window)
        if (now - lastBlastNova >= 47)
        {
            LOG_DEBUG("playerbots", "SpreadRangedAction: Disabled for cube clicker (bot={}) during cube click window", bot->GetName());
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
    auto it = botToCubeAssignment.find(bot->GetGUID());
    const CubeInfo& cubeInfo = it->second;
    GameObject* cube = botAI->GetGameObject(cubeInfo.guid);
    if (!cube) 
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} cannot find assigned cube, exiting", bot->GetName());
        return false;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    time_t now = time(nullptr);
    time_t lastBlastNova = magtheridonBlastNovaTimer[bot->GetMapId()];
    bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(static_cast<uint32>(MagsLairSpells::BLAST_NOVA));

    // 1. For the first 47s after activation or last Blast Nova, do nothing
    if (now - lastBlastNova < 47)
    {
        LOG_DEBUG("playerbots", "CubeAction: Bot {} idle for first 47s, elapsed={}s", bot->GetName(), now - lastBlastNova);
        return false;
    }

    // 2. If past 47s but not casting Blast Nova: move to safe wait area around cube
    if (!blastNovaActive) 
    {
        const float safeWaitDistance = 7.0f;
        float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

        LOG_DEBUG("playerbots", "CubeAction: Bot {} entering safe wait area logic, cubeDist={}", bot->GetName(), cubeDist);
        if (fabs(cubeDist - safeWaitDistance) > 0.5f) 
        {
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

    // 3. If Blast Nova is casting: move to cube and use it
    const float interactDistance = 1.0f;
    const float interactDistanceBuffer = 1.0f;
    float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);
    LOG_DEBUG("playerbots", "CubeAction: Bot {} entering cube use logic, cubeDist={}", bot->GetName(), cubeDist);

    if (cubeDist > interactDistance) 
    {
        if (cubeDist <= interactDistance + interactDistanceBuffer) 
        {
            LOG_DEBUG("playerbots", "CubeAction: Bot {} close enough to use cube, using", bot->GetName());
            bot->StopMoving();
            cube->Use(bot);
            return true;
        }
        
        LOG_DEBUG("playerbots", "CubeAction: Bot {} moving to cube at ({}, {})", bot->GetName(), cubeInfo.x, cubeInfo.y);
        float angle = atan2(cubeInfo.y - bot->GetPositionY(), cubeInfo.x - bot->GetPositionX());
        float targetX = cubeInfo.x - cos(angle) * interactDistance;
        float targetY = cubeInfo.y - sin(angle) * interactDistance;
        float targetZ = bot->GetPositionZ();

        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, 
                      MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool MagtheridonUseManticronCubeAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon)
    {
        bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(static_cast<uint32>(MagsLairSpells::BLAST_NOVA));
        UpdateTransitionTimer(magtheridon, blastNovaActive, lastBlastNovaState, magtheridonBlastNovaTimer);

        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)), 
                              lastShadowCageState, magtheridonBlastNovaTimer);
    }

    Group* group = bot->GetGroup();
    if (!group || !magtheridon->IsAlive() || magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)) || 
        bot->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_GRASP)))
    {
        return false;
    }

    // Reassign if empty OR any assigned bot is dead or missing
    bool needsReassign = botToCubeAssignment.empty();
    if (!needsReassign)
    {
        for (const auto& pair : botToCubeAssignment)
        {
            Player* assigned = ObjectAccessor::FindPlayer(pair.first);
            if (!assigned || !assigned->IsAlive())
            {
                needsReassign = true;
                break;
            }
        }
    }

    if (needsReassign)
    {
        std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);
        AssignBotsToCubesByGuidAndCoords(group, cubes, botAI);
    }

    return botToCubeAssignment.find(bot->GetGUID()) != botToCubeAssignment.end();
}
