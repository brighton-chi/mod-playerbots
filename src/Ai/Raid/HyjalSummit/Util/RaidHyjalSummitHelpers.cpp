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

    const Position KAZROGAL_TANK_POSITION = { 5500.721f, -2666.152f, 1480.911f };
    // std::unordered_map<ObjectGuid, Position> kazrogalRangedPositions;
    // std::unordered_map<ObjectGuid, bool> hasReachedKazrogalPosition;
    std::unordered_map<ObjectGuid, bool> isBelowManaThreshold;

    // Azgalor

    const Position AZGALOR_MAIN_TANK_TRANSITION_POSITION = { 5504.281f, -2682.223f, 1480.610f };
    const Position AZGALOR_MAIN_TANK_FINAL_POSITION = { 5493.910f, -2678.707f, 1480.795f };
    const Position AZGALOR_DOOMGUARD_TANK_POSITION = { 5513.604f, -2722.173f, 1483.091f };
    std::unordered_map<ObjectGuid, uint8> azgalorTankStep;

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
