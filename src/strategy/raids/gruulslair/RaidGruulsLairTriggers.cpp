#include "RaidGruulsLairTriggers.h"
#include "RaidGruulsLairHelpers.h"
#include "Playerbots.h"

using namespace GruulsLairHelpers;

// High King Maulgar Triggers

bool HighKingMaulgarIsMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    return maulgar != nullptr;
}

bool HighKingMaulgarIsFirstAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    return olm != nullptr;
}

bool HighKingMaulgarIsSecondAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    return blindeye != nullptr;
}

bool HighKingMaulgarIsMageTankTrigger::IsActive()
{
    if (!IsKroshMageTank(botAI, bot))
        return false;

    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    return krosh != nullptr;
}

bool HighKingMaulgarIsMoonkinTankTrigger::IsActive()
{
    if (!IsKigglerMoonkinTank(botAI, bot))
        return false;

    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    return kiggler != nullptr;
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActive()
{
    if (botAI->IsHeal(bot) || botAI->IsMainTank(bot))
        return false;

    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (krosh && IsKroshMageTank(botAI, bot))
        return false;

    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (kiggler && IsKigglerMoonkinTank(botAI, bot))
        return false;

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (olm && botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (blindeye && botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    return maulgar != nullptr;
}

bool HighKingMaulgarHealerInDangerTrigger::IsActive()
{
    if (!botAI->IsHeal(bot))
        return false;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    return maulgar != nullptr;
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    return maulgar && maulgar->HasAura(SPELL_WHIRLWIND);
}

bool HighKingMaulgarWildFelstalkerSpawnedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");
    return felStalker != nullptr;
}

bool HighKingMaulgarPullingOlmAndBlindeyeTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
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
    if (hunterIndex == -1 || hunterIndex > 1)
        return false;

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Player* olmTank = nullptr;
    Player* blindeyeTank = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        else if (botAI->IsAssistTankOfIndex(member, 0)) olmTank = member;
        else if (botAI->IsAssistTankOfIndex(member, 1)) blindeyeTank = member;
    }

    switch (hunterIndex)
    {
    case 0:
        return olm && olm->GetHealthPct() > 98.0f && olmTank && olmTank->IsAlive() &&
               botAI->CanCastSpell("misdirection", olmTank);

    case 1:
        return blindeye && blindeye->GetHealthPct() > 90.0f && blindeyeTank &&
               blindeyeTank->IsAlive() && botAI->CanCastSpell("misdirection", blindeyeTank);

    default:
        break;
    }

    return false;
}

bool HighKingMaulgarNeedToManageDpsTimerTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar)
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsDps(member) && GET_PLAYERBOT_AI(member))
                return member == bot;
        }
    }

    return false;
}

// Gruul the Dragonkiller Triggers

bool GruulTheDragonkillerBossIsActiveTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    return gruul != nullptr;
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && (bot->HasAura(SPELL_GROUND_SLAM_1) || bot->HasAura(SPELL_GROUND_SLAM_2));
}
