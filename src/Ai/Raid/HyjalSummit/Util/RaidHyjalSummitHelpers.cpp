#include "RaidHyjalSummitHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace HyjalSummitHelpers
{
    // Rage Winterchill

    const Position RAGE_WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f };
    std::unordered_map<ObjectGuid, Position> winterchillRangedPositions;
    std::unordered_map<ObjectGuid, bool> hasReachedWinterchillPosition;

    // Anetheron

    const Position ANETHERON_MAIN_TANK_POSITION = { 5026.900f, -1757.163f, 1323.403f };
    const Position ANETHERON_INFERNAL_TANK_POSITION = { 5021.879f, -1776.768f, 1322.390f };
    // const Position ANETHERON_INFERNAL_TANK_POSITION = { 5024.107f, -1787.967f, 1321.832f }; original medium location
    // const Position ANETHERON_INFERNAL_TANK_POSITION = { 5028.010f, -1800.134f, 1321.557f }; farther location
    std::unordered_map<ObjectGuid, Position> anetheronRangedPositions;

    bool IsBotTargetedByInferno(Unit* anetheron, Player* bot)
    {
        if (!anetheron)
            return false;

        Spell* spell = anetheron->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell && spell->m_spellInfo->Id == SPELL_INFERNO)
        {
            Unit* spellTarget = spell->m_targets.GetUnitTarget();
            if (spellTarget && spellTarget->GetGUID() == bot->GetGUID())
                return true;
        }

        return false;
    }

    // Kaz'rogal

    const Position KAZROGAL_TANK_POSITION = { 5503.475f, -2660.343f, 1480.398f };
    std::unordered_map<ObjectGuid, Position> kazrogalRangedPositions;

    // Azgalor

    /* const Position AZGALOR_MAIN_TANK_POSITION = { 5504.281f, -2682.223f, 1480.610f };
    const Position AZGALOR_DOOMGUARD_TANK_POSITION = { 5467.278f, -2699.219f, 1485.328f }; */ // Tested positions
    const Position AZGALOR_MAIN_TANK_TRANSITION_POSITION = { 5504.281f, -2682.223f, 1480.610f }; // this was the single tank location, trying it as transition
    const Position AZGALOR_MAIN_TANK_FINAL_POSITION = { 5493.910f, -2678.707f, 1480.795f };
    // const Position AZGALOR_DOOMGUARD_TANK_POSITION = { 5509.107f, -2695.247f, 1480.408f }; // this is very close to the tank position
    const Position AZGALOR_DOOMGUARD_TANK_POSITION = { 5513.604f, -2722.173f, 1483.091f };
    std::unordered_map<ObjectGuid, uint8> azgalorTankStep;

    bool AnyGroupMemberHasDoom(Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() &&
                    member->HasAura(SPELL_DOOM))
                {
                    return true;
                }
            }
        }

        return false;
    }

    // Archimonde
    // N/A
}
