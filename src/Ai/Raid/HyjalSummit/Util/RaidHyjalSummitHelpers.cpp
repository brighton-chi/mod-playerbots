#include "RaidHyjalSummitHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace HyjalSummitHelpers
{
    // Rage Winterchill

    const Position RAGE_WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f };
    std::unordered_map<ObjectGuid, bool> hasReachedWinterchillPosition;

    // Anetheron

    const Position ANETHERON_MAIN_TANK_POSITION = { 5033.177f, -1765.996f, 1324.195f };
    const Position ANETHERON_E_INFERNAL_TANK_POSITION = { 5022.587f, -1802.805f, 1322.026f };
    const Position ANETHERON_W_INFERNAL_TANK_POSITION = { 5048.911f, -1722.164f, 1321.408f };
    std::unordered_map<ObjectGuid, bool> hasReachedAnetheronPosition;

    Player* GetInfernoTarget(Unit* anetheron)
    {
        if (!anetheron)
            return nullptr;

        Spell* spell = anetheron->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell && spell->m_spellInfo->Id == SPELL_INFERNO)
        {
            Unit* spellTarget = spell->m_targets.GetUnitTarget();
            if (spellTarget && spellTarget->GetTypeId() == TYPEID_PLAYER)
                return static_cast<Player*>(spellTarget);
        }

        return nullptr;
    }

    const Position& GetClosestInfernalTankPosition(Player* bot)
    {
        const Position& east = ANETHERON_E_INFERNAL_TANK_POSITION;
        const Position& west = ANETHERON_W_INFERNAL_TANK_POSITION;
        return (bot->GetExactDist2d(east.GetPositionX(), east.GetPositionY()) <=
                bot->GetExactDist2d(west.GetPositionX(), west.GetPositionY()))
                ? east : west;
    }

    // Kaz'rogal

    const Position KAZROGAL_TANK_POSITION = { 5494.809f, -2701.391f, 1481.861f };
    std::unordered_map<ObjectGuid, bool> isBelowManaThreshold;

    // Azgalor

    const Position AZGALOR_MAIN_TANK_TRANSITION_POSITION = { 5496.044f, -2688.533f, 1480.367f };
    const Position AZGALOR_MAIN_TANK_FINAL_POSITION = { 5498.800f, -2663.021f, 1480.806f };
    // const Position AZGALOR_DOOMGUARD_TANK_POSITION = { 5513.604f, -2722.173f, 1483.091f };
    const Position AZGALOR_DOOMGUARD_TANK_POSITION = { 5453.381f, -2725.236f, 1485.468f }; // Thrall position test
    const Position AZGALOR_WAITING_POSITION = { 5484.535f, -2723.090f, 1483.676f };
    std::unordered_map<ObjectGuid, uint8> azgalorTankStep;

    int GetAzgalorTankStep(PlayerbotAI* botAI, Player* bot)
    {
        Player* mainTank = GetGroupMainTank(botAI, bot);
        if (!mainTank) return -1;

        auto it = azgalorTankStep.find(mainTank->GetGUID());
        if (it != azgalorTankStep.end())
            return it->second;

        return -1;
    }

    bool AnyGroupMemberHasDoom(Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->HasAura(SPELL_DOOM))
                    return true;
            }
        }

        return false;
    }
}
