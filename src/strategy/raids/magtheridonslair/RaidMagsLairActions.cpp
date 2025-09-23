#include "RaidMagsLairActions.h"
#include "RaidMagsLairHelpers.h"
#include "ChatHelper.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool MagtheridonHellfireChannelerSouthTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    if (!channelerSquare || !channelerSquare->IsAlive())
        return false;

    // Mark with square icon
    ObjectGuid currentIconGuid = group->GetTargetIcon(squareIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channelerSquare->GetGUID())
        group->SetTargetIcon(squareIcon, bot->GetGUID(), channelerSquare->GetGUID());

    // Set RTI value and target
    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "square" ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != channelerSquare)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerSquare);
    }

    if (bot->GetVictim() != channelerSquare)
    {
        Attack(channelerSquare);

        if (!bot->IsWithinMeleeRange(channelerSquare))
            return MoveTo(channelerSquare->GetMapId(), channelerSquare->GetPositionX(), channelerSquare->GetPositionY(), channelerSquare->GetPositionZ());
    }

    return false;
}

bool MagtheridonHellfireChannelerSouthTankAction::isUseful()
{
    if (!IsSouthTank(botAI, bot))
        return false;

    Creature* channeler = GetChanneler(bot, SOUTH_CHANNELER);

    return channeler && channeler->IsAlive();
}

bool MagtheridonHellfireChannelerWestTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);

    if ((!channelerStar || !channelerStar->IsAlive()) && (!channelerCircle || !channelerCircle->IsAlive()))
        return false;

    // Target selection logic (swap every 5 seconds, or always target the alive one if the other is dead)
    static uint8 currentTargetIndex = 0;
    static time_t lastSwapTime = 0;
    time_t now = time(nullptr);

    if (!channelerStar || !channelerStar->IsAlive())
        currentTargetIndex = 1;
    else if (!channelerCircle || !channelerCircle->IsAlive())
        currentTargetIndex = 0;
    else if (now - lastSwapTime >= 5)
    {
        currentTargetIndex = (currentTargetIndex + 1) % 2;
        lastSwapTime = now;
    }

    // Mark both channelers
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

    // Set RTI and target
    Creature* currentTarget = (currentTargetIndex == 0) ? channelerStar : channelerCircle;
    std::string rtiName = (currentTargetIndex == 0) ? "star" : "circle";
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

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);

    // Check if at least one assigned Channeler is alive
    if ((channelerStar && channelerStar->IsAlive()) || (channelerCircle && channelerCircle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonHellfireChannelerEastTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channelerDiamond = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    if ((!channelerDiamond || !channelerDiamond->IsAlive()) && (!channelerTriangle || !channelerTriangle->IsAlive()))
        return false;

    // Target selection logic (swap every 5 seconds, or always target the alive one if the other is dead)
    static uint8 currentTargetIndex = 0;
    static time_t lastSwapTime = 0;
    time_t now = time(nullptr);

    if (!channelerDiamond || !channelerDiamond->IsAlive())
        currentTargetIndex = 1;
    else if (!channelerTriangle || !channelerTriangle->IsAlive())
        currentTargetIndex = 0;
    else if (now - lastSwapTime >= 5)
    {
        currentTargetIndex = (currentTargetIndex + 1) % 2;
        lastSwapTime = now;
    }

    // Mark both channelers
    if (channelerDiamond && channelerDiamond->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerDiamond->GetGUID())
            group->SetTargetIcon(diamondIcon, bot->GetGUID(), channelerDiamond->GetGUID());
    }
    if (channelerTriangle && channelerTriangle->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(triangleIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerTriangle->GetGUID())
            group->SetTargetIcon(triangleIcon, bot->GetGUID(), channelerTriangle->GetGUID());
    }

    // Set RTI and target
    Creature* currentTarget = (currentTargetIndex == 0) ? channelerDiamond : channelerTriangle;
    std::string rtiName = (currentTargetIndex == 0) ? "diamond" : "triangle";
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

bool MagtheridonHellfireChannelerEastTankAction::isUseful()
{
    if (!IsEastTank(botAI, bot))
        return false;

    Creature* channelerDiamond = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    // Check if at least one assigned Channeler is alive
    if ((channelerDiamond && channelerDiamond->IsAlive()) || (channelerTriangle && channelerTriangle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonHellfireChannelerSouthWarlockAction::Execute(Event event)
{
    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);

    std::string rtiName;
    Unit* rtiTarget = nullptr;

    if (channelerSquare && channelerSquare->IsAlive())
    {
        rtiName = "square";
        rtiTarget = channelerSquare;
    }

    // Set RTI value and target if needed
    if (!rtiName.empty() &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != rtiTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(rtiTarget);
    }

    if (channelerSquare && channelerSquare->IsAlive())
    {
        if (!channelerSquare->HasAura(SPELL_CURSE_OF_TONGUES) && botAI->CanCastSpell(SPELL_CURSE_OF_TONGUES, channelerSquare, true))
        {
            botAI->CastSpell(SPELL_CURSE_OF_TONGUES, channelerSquare);
            return false;
        }
    }

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* abyssal = nullptr;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL)
        {
            abyssal = unit;
            break;
        }
    }

    if (abyssal && abyssal->IsAlive() && !abyssal->HasAura(SPELL_BANISH) && botAI->CanCastSpell(SPELL_BANISH, abyssal, true))
    {
        botAI->CastSpell(SPELL_BANISH, abyssal);
        return false;
    }

    return false;
}

bool MagtheridonHellfireChannelerSouthWarlockAction::isUseful()
{
    if (!IsSouthWarlock(botAI, bot))
        return false;

    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    return channelerSquare && channelerSquare->IsAlive();
}

bool MagtheridonHellfireChannelerWestWarlockAction::Execute(Event event)
{
    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
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

    // Set RTI value and target if needed
    if (!rtiName.empty() &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != rtiTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(rtiTarget);
    }

    // Prefer star channeler if alive, else circle channeler
    Creature* moveTarget = nullptr;
    if (channelerStar && channelerStar->IsAlive())
        moveTarget = channelerStar;
    else if (channelerCircle && channelerCircle->IsAlive())
        moveTarget = channelerCircle;

    // Only move if farther than 30 yards from the alive channeler
    if (moveTarget && bot->GetExactDist2d(moveTarget) > 30.0f)
    {
        return MoveTo(moveTarget->GetMapId(), moveTarget->GetPositionX(), moveTarget->GetPositionY(), moveTarget->GetPositionZ());
    }

    // Keep Curse of Tongues on both channelers
    for (Creature* channeler : {channelerStar, channelerCircle})
    {
        if (channeler && channeler->IsAlive() &&
            !channeler->HasAura(SPELL_CURSE_OF_TONGUES) &&
            botAI->CanCastSpell(SPELL_CURSE_OF_TONGUES, channeler, true))
        {
            botAI->CastSpell(SPELL_CURSE_OF_TONGUES, channeler);
            return false;
        }
    }

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    // Track if we are already banishing an abyssal
    bool isAlreadyBanishing = false;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL && unit->HasAura(SPELL_BANISH))
        {
            isAlreadyBanishing = true;
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
                    botAI->CastSpell(SPELL_BANISH, unit);
                    return false;
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
                botAI->CastSpell(SPELL_FEAR, unit);
                return false;
            }
        }
    }

return false;
}

bool MagtheridonHellfireChannelerWestWarlockAction::isUseful()
{
    if (!IsWestWarlock(botAI, bot))
        return false;

    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);

    // Check if at least one assigned Channeler is alive
    if ((channelerStar && channelerStar->IsAlive()) || (channelerCircle && channelerCircle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonHellfireChannelerEastWarlockAction::Execute(Event event)
{
    Creature* channelerDiamond  = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    // RTI logic: prioritize diamond, then triangle
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

    // Movement: move to diamond if alive, else triangle, only if farther than 30 yards
    Creature* moveTarget = nullptr;
    if (channelerDiamond && channelerDiamond->IsAlive())
        moveTarget = channelerDiamond;
    else if (channelerTriangle && channelerTriangle->IsAlive())
        moveTarget = channelerTriangle;

    if (moveTarget && bot->GetExactDist2d(moveTarget) > 30.0f)
    {
        return MoveTo(moveTarget->GetMapId(), moveTarget->GetPositionX(), moveTarget->GetPositionY(), moveTarget->GetPositionZ());
    }

    // Keep Curse of Tongues on both channelers
    for (Creature* channeler : {channelerDiamond, channelerTriangle})
    {
        if (channeler && channeler->IsAlive() &&
            !channeler->HasAura(SPELL_CURSE_OF_TONGUES) &&
            botAI->CanCastSpell(SPELL_CURSE_OF_TONGUES, channeler, true))
        {
            botAI->CastSpell(SPELL_CURSE_OF_TONGUES, channeler);
            return false;
        }
    }

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    // Track if we are already banishing an abyssal (by this bot)
    bool isAlreadyBanishing = false;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL)
        {
            Aura* banishAura = unit->GetAura(SPELL_BANISH, bot->GetGUID());
            if (banishAura)
            {
                isAlreadyBanishing = true;
                break;
            }
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
                    botAI->CastSpell(SPELL_BANISH, unit);
                    return false;
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
                botAI->CastSpell(SPELL_FEAR, unit);
                return false;
            }
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerEastWarlockAction::isUseful()
{
    if (!IsEastWarlock(botAI, bot))
        return false;

    Creature* channelerDiamond  = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    // Check if at least one assigned Channeler is alive
    if ((channelerDiamond && channelerDiamond->IsAlive()) || (channelerTriangle && channelerTriangle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonHellfireChannelerWestHealerAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

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

    if (westTank && bot->GetExactDist2d(westTank) > 30.0f)
    {
        return MoveTo(westTank->GetMapId(), westTank->GetPositionX(), westTank->GetPositionY(), westTank->GetPositionZ());
    }
    return false;
}

bool MagtheridonHellfireChannelerWestHealerAction::isUseful()
{
    if (!IsWestHealer(botAI, bot))
        return false;

    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);

    // Check if at least one assigned Channeler is alive
    if ((channelerStar && channelerStar->IsAlive()) || (channelerCircle && channelerCircle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonHellfireChannelerEastHealerAction::Execute(Event event)
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

    if (eastTank && bot->GetExactDist2d(eastTank) > 30.0f)
    {
        return MoveTo(eastTank->GetMapId(), eastTank->GetPositionX(), eastTank->GetPositionY(), eastTank->GetPositionZ());
    }
    return false;
}

bool MagtheridonHellfireChannelerEastHealerAction::isUseful()
{
    if (!IsEastHealer(botAI, bot))
        return false;

    Creature* channelerDiamond  = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    // Check if at least one assigned Channeler is alive
    if ((channelerDiamond && channelerDiamond->IsAlive()) || (channelerTriangle && channelerTriangle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonHellfireChannelerWestHunterAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

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

    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
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

    // Set RTI value and target if needed
    if (!rtiName.empty() &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != rtiTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(rtiTarget);
    }

    // Prefer circle channeler if alive, else star channeler
    Creature* moveTarget = nullptr;
    if (channelerCircle && channelerCircle->IsAlive())
        moveTarget = channelerCircle;
    else if (channelerStar && channelerStar->IsAlive())
        moveTarget = channelerStar;

    if (moveTarget && bot->GetExactDist2d(moveTarget) > 35.0f)
    {
        return MoveTo(moveTarget->GetMapId(), moveTarget->GetPositionX(), moveTarget->GetPositionY(), moveTarget->GetPositionZ());
    }

    if (channelerCircle && channelerCircle->IsAlive() && westTank && westTank->IsAlive())
    {
        if (channelerCircle->GetVictim() != westTank)
        {
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
                botAI->CastSpell("steady shot", channelerCircle);
                return false;
            }
        }
    }

    if (channelerStar && channelerStar->IsAlive() && westTank && westTank->IsAlive())
    {
        if (channelerStar->GetVictim() != westTank)
        {
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
                botAI->CastSpell("steady shot", channelerStar);
            }
        }
    }
            
    return false;
}

bool MagtheridonHellfireChannelerWestHunterAction::isUseful()
{
    if (!IsWestHunter(botAI, bot))
        return false;

    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, NORTHWEST_CHANNELER);

    // Check if at least one assigned Channeler is alive
    if ((channelerStar && channelerStar->IsAlive()) || (channelerCircle && channelerCircle->IsAlive()))
        return true;

    return false;
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

    Creature* channelerDiamond  = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

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
                return false;
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
            }
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerEastHunterAction::isUseful()
{
    if (!IsEastHunter(botAI, bot))
        return false;

    Creature* channelerDiamond  = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    // Check if at least one assigned Channeler is alive
    if ((channelerDiamond && channelerDiamond->IsAlive()) || (channelerTriangle && channelerTriangle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonHellfireChannelerDPSPriorityAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar     = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle   = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, EAST_CHANNELER);
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

    // Target priority 4: East Channeler
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
    if (magtheridon && magtheridon->IsAlive() && magtheridon->IsInCombat())
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
    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar     = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle   = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    if (!botAI->IsDps(bot) ||
        (IsWestWarlock(botAI, bot) && (!channelerStar || !channelerStar->IsAlive() || 
         !channelerCircle || !channelerCircle->IsAlive())) ||
        (IsEastWarlock(botAI, bot) && (!channelerDiamond || !channelerDiamond->IsAlive() || 
         !channelerTriangle || !channelerTriangle->IsAlive())) ||
        (IsWestHunter(botAI, bot) && (!channelerStar || !channelerStar->IsAlive() || 
         !channelerCircle || !channelerCircle->IsAlive())) ||
        (IsEastHunter(botAI, bot) && (!channelerDiamond || !channelerDiamond->IsAlive() || 
         !channelerTriangle || !channelerTriangle->IsAlive())))
        return false;
    
    if ((channelerSquare   && channelerSquare->IsAlive())   ||
        (channelerStar     && channelerStar->IsAlive())     ||
        (channelerCircle   && channelerCircle->IsAlive())   ||
        (channelerDiamond  && channelerDiamond->IsAlive())  ||
        (channelerTriangle && channelerTriangle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonPositionBossAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    static const TankSpot& tankSpot = MagtheridonTankSpot;

    const float maxDistance = 3.0f;
    float distanceToTankSpot = magtheridon->GetExactDist2d(tankSpot.x, tankSpot.y);

    if (distanceToTankSpot > maxDistance)
    {
        float dX = tankSpot.x - magtheridon->GetPositionX();
        float dY = tankSpot.y - magtheridon->GetPositionY();
        float moveX = tankSpot.x + (dX / distanceToTankSpot) * maxDistance;
        float moveY = tankSpot.y + (dY / distanceToTankSpot) * maxDistance;

        float moveDistance = bot->GetExactDist2d(moveX, moveY);
        if (moveDistance < 0.5f)
        {
            return false;
        }

        return MoveTo(bot->GetMapId(), moveX, moveY, tankSpot.z, 
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
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
        bot->SetFacingTo(tankSpot.orientation);
    }

    return false; 
}

bool MagtheridonPositionBossAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    return magtheridon && magtheridon->IsAlive() && botAI->IsTank(bot) && 
           botAI->HasAggro(magtheridon) && magtheridon->GetVictim() == bot;
}

bool MagtheridonSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->IsInCombat() && magtheridon->GetHealth() == magtheridon->GetMaxHealth())
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

        // Random radius between 25 and 45 yards
        float minRadius = 25.0f;
        float maxRadius = 45.0f;
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
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            return MoveTo(bot->GetMapId(), targetPosition.GetPositionX(), targetPosition.GetPositionY(), targetPosition.GetPositionZ());
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    float magtheridonRangedRadius = 25.0f;
    float minSpreadDistance = 10.0f;
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

    if (distanceToMagtheridon < magtheridonRangedRadius - 3.0f - movementThreshold)
    {
        return MoveTo(bot->GetMapId(), tankSpot.x, tankSpot.y, tankSpot.z);
    }

    return false;
}

bool MagtheridonSpreadRangedAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !magtheridon->IsInCombat() || !botAI->IsRanged(bot))
        return false;

    Creature* channelerSquare   = GetChanneler(bot, 43160); // South
    Creature* channelerStar     = GetChanneler(bot, 43158); // West
    Creature* channelerCircle   = GetChanneler(bot, 43161); // Northwest
    Creature* channelerDiamond  = GetChanneler(bot, 43159); // East
    Creature* channelerTriangle = GetChanneler(bot, 43157); // Northeast

    if ((channelerSquare   && channelerSquare->IsAlive())   ||
        (channelerStar     && channelerStar->IsAlive())     ||
        (channelerCircle   && channelerCircle->IsAlive())   ||
        (channelerDiamond  && channelerDiamond->IsAlive())  ||
        (channelerTriangle && channelerTriangle->IsAlive()))
        return false;

    return true;
}

// Movement action for clickers to stay near cubes
bool MagtheridonManticronCubeClickerPositionAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !magtheridon->IsAlive() || !magtheridon->IsInCombat())
    {
        cubeTimers.clear();
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Assign ranged DPS to cubes if not already assigned
    if (cubeAssignments.empty())
        AssignRangedDpsToCubes(group, botAI);

    // Make sure this bot is assigned to a cube
    auto it = cubeAssignments.find(bot->GetGUID());
    if (it == cubeAssignments.end())
        return false;

    int cubeIndex = it->second;
    const size_t cubeCount = sizeof(manticronCubes) / sizeof(manticronCubes[0]);
    if (cubeIndex < 0 || cubeIndex >= cubeCount)
        return false;

    const CubePosition& cube = manticronCubes[cubeIndex];

    // Timer logic
    time_t now = time(nullptr);

    // If Magtheridon is casting Blast Nova, reset timer for this bot
    if (magtheridon->HasUnitState(UNIT_STATE_CASTING) && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
    {
        cubeTimers[bot->GetGUID()] = now;
        return false;
    }

    // If timer hasn't started, start it now
    if (cubeTimers[bot->GetGUID()] == 0)
        cubeTimers[bot->GetGUID()] = now;

    // Wait 45 seconds after the start of the last Blast Nova cast
    if (now - cubeTimers[bot->GetGUID()] < 45)
        return false;

    // Move to 5 yards away from cube
    float offsetAngle = urand(0, 359) * M_PI / 180.0f;
    float targetX = cube.x + cos(offsetAngle) * 5.0f;
    float targetY = cube.y + sin(offsetAngle) * 5.0f;
    float targetZ = cube.z;

    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

// Action for clickers to use cube
bool MagtheridonUseManticronCubeAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !magtheridon->IsInCombat())
        return false;

    // Only act if Magtheridon is casting Blast Nova
    if (!(magtheridon->HasUnitState(UNIT_STATE_CASTING) && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA)))
        return false;

    // Make sure this bot is assigned to a cube
    auto it = cubeAssignments.find(bot->GetGUID());
    if (it == cubeAssignments.end())
        return false;

    int cubeIndex = it->second;
    const size_t cubeCount = sizeof(manticronCubes) / sizeof(manticronCubes[0]);
    if (cubeIndex < 0 || cubeIndex >= cubeCount)
        return false;

    const CubePosition& cube = manticronCubes[cubeIndex];

    // Move to within interaction distance (e.g. 1.5 yards)
    float interactionDistance = 1.5f;
    float offsetAngle = urand(0, 359) * M_PI / 180.0f;
    float targetX = cube.x + cos(offsetAngle) * interactionDistance;
    float targetY = cube.y + sin(offsetAngle) * interactionDistance;
    float targetZ = cube.z;

    // If not close enough, move closer
    if (bot->GetExactDist2d(targetX, targetY) > interactionDistance)
        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, MovementPriority::MOVEMENT_FORCED);

    // Find the actual game object for the cube (by position)
    GuidVector nearbyCubes = chat->parseGameobjects("manticron cube");
    GameObject* go = nullptr;
    for (ObjectGuid guid : nearbyCubes)
    {
        GameObject* candidate = botAI->GetGameObject(guid);
        if (candidate &&
            fabs(candidate->GetPositionX() - cube.x) < 1.0f &&
            fabs(candidate->GetPositionY() - cube.y) < 1.0f)
        {
            go = candidate;
            break;
        }
    }
    if (!go)
        return false;

    // Use the cube
    go->Use(bot);

    return true;
}
