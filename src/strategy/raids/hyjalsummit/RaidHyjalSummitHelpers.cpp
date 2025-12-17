#include "RaidHyjalSummitHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace HyjalSummitHelpers
{
    // General Helpers

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
    {
        if (!target)
            return;

        if (Group* group = bot->GetGroup())
        {
            ObjectGuid currentGuid = group->GetTargetIcon(iconId);
            if (currentGuid != target->GetGUID())
                group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
        }
    }

    void MarkTargetWithSkull(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::skullIndex);
    }

    void MarkTargetWithSquare(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
    }

    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target)
    {
        if (!target)
            return;

        std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();

        if (currentRti != rtiName || currentTarget != target)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
        }
    }

    Unit* GetNearestPlayerInRadius(Player* bot, float radius)
    {
        Unit* nearestPlayer = nullptr;
        float nearestDistance = radius;

        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || member == bot)
                    continue;

                float distance = bot->GetExactDist2d(member);
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestPlayer = member;
                }
            }
        }

        return nearestPlayer;
    }

    // Dps bot selected for marking and managing timers and trackers
    bool IsInstanceTimerManager(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && botAI->IsDps(member) &&
                    GET_PLAYERBOT_AI(member))
                    return member == bot;
            }
        }

        return false;
    }

    // Rage Winterchill

    const Position RAGE_WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f }; // Crossroads
    std::unordered_map<ObjectGuid, Position> winterchillRangedPositions;
    std::unordered_map<ObjectGuid, bool> hasReachedWinterchillPosition;

    // Anetheron

    const Position ANETHERON_MAIN_TANK_POSITION = { 5026.900f, -1757.163f, 1323.403f }; // Closer to gate
    const Position ANETHERON_INFERNAL_TANK_POSITION = { 5024.107f, -1787.967f, 1321.832f }; // Back toward ballista
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

    const Position KAZROGAL_TANK_POSITION = { 5501.381f, -2736.129f, 1487.012f }; // toward back of weird strip land between paths
    std::unordered_map<ObjectGuid, Position> kazrogalRangedPositions;

    // Azgalor

    const Position AZGALOR_MAIN_TANK_POSITION = { 5492.193f, -2726.423f, 1484.705f }; // In middleish of the weird strip of land between paths
    const Position AZGALOR_DOOMGUARD_TANK_POSITION = { 5487.0f, -2745.0f, 1484.705f };
    std::unordered_map<ObjectGuid, Position> azgalorRangedPositions;

    // Archimonde

    const Position ARCHIMONDE_TANK_POSITION = { 0.0f, 0.0f, 0.0f };
}
