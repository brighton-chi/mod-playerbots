#include "RaidGruulsLairHelpers.h"
#include "PlayerbotAI.h"
#include "Group.h"
#include "GroupReference.h"
#include "Unit.h"

bool IsFirstOffTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot) || botAI->IsMainTank(bot) || !group->IsAssistant(bot->GetGUID()))
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member) && !botAI->IsMainTank(member) && group->IsAssistant(member->GetGUID()))
            return member == bot;
    }
    return false;
}

// Returns true if the bot is the mage with the highest max HP in the group (mage tank)
bool IsMageTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* highestHpMage = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;
        if (member->getClass() == CLASS_MAGE)
        {
            uint32 hp = member->GetMaxHealth();
            if (!highestHpMage || hp > highestHp)
            {
                highestHpMage = member;
                highestHp = hp;
            }
        }
    }
    return highestHpMage == bot;
}

// Returns true if the bot is the balance druid with the highest max HP in the group (boomkin tank)
bool IsBoomkinTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* highestHpBoomkin = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;
        // Check if member is a balance druid (tab == DRUID_TAB_BALANCE)
        if (member->getClass() == CLASS_DRUID)
        {
            int tab = AiFactory::GetPlayerSpecTab(member);
            if (tab == DRUID_TAB_BALANCE)
            {
                uint32 hp = member->GetMaxHealth();
                if (!highestHpBoomkin || hp > highestHp)
                {
                    highestHpBoomkin = member;
                    highestHp = hp;
                }
            }
        }
    }
    return highestHpBoomkin == bot;
}

bool IsWarlockTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* highestHpWarlock = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;
        if (member->getClass() == CLASS_WARLOCK)
        {
            uint32 hp = member->GetMaxHealth();
            if (!highestHpWarlock || hp > highestHp)
            {
                highestHpWarlock = member;
                highestHp = hp;
            }
        }
    }
    return highestHpWarlock == bot;
}
