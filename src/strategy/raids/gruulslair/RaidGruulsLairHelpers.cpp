#include "RaidGruulsLairHelpers.h"
#include "AiFactory.h"
#include "PlayerbotAI.h"
#include "Group.h"
#include "GroupReference.h"
#include "Unit.h"

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

// Returns true if the bot is the tank with the highest max HP in the group
bool IsFirstTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
        return false;

    Player* firstTank = nullptr;
    uint32 firstHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            uint32 hp = member->GetMaxHealth();
            if (!firstTank || hp > firstHp)
            {
                firstTank = member;
                firstHp = hp;
            }
        }
    }
    return firstTank == bot;
}

// Returns true if the bot is the tank with the second highest max HP in the group
bool IsSecondTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
        return false;

    std::vector<std::pair<Player*, uint32>> tanks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
            tanks.push_back({member, member->GetMaxHealth()});
    }
    if (tanks.size() < 2)
        return false;
    std::sort(tanks.begin(), tanks.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    return tanks[1].first == bot;
}

// Returns true if the bot is the tank with the third highest max HP in the group
bool IsThirdTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
        return false;

    std::vector<std::pair<Player*, uint32>> tanks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
            tanks.push_back({member, member->GetMaxHealth()});
    }
    if (tanks.size() < 3)
        return false;
    std::sort(tanks.begin(), tanks.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    return tanks[2].first == bot;
}
