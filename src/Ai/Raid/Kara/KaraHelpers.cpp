#include "KaraHelpers.h"
#include "Playerbots.h"

namespace KarazhanHelpers
{

// Attumen the Huntsman
std::unordered_map<uint32, time_t> attumenDpsWaitTimer;
// Netherspite
std::unordered_map<uint32, time_t> netherspiteDpsWaitTimer;
std::unordered_map<ObjectGuid, time_t> redBeamMoveTimer;
std::unordered_map<ObjectGuid, bool> lastBeamMoveSideways;
// Nightbane
std::unordered_map<uint32, time_t> nightbaneDpsWaitTimer;
std::unordered_map<uint32, time_t> nightbaneFlightPhaseStartTimer;

const Position MAIDEN_OF_VIRTUE_RANGED_POSITION[8] =
{
    { -10931.178f, -2116.580f, 92.179f },
    { -10925.828f, -2102.425f, 92.180f },
    { -10933.089f, -2088.502f, 92.180f },
    { -10947.590f, -2082.815f, 92.180f },
    { -10960.912f, -2090.437f, 92.179f },
    { -10966.017f, -2105.288f, 92.175f },
    { -10959.242f, -2119.617f, 92.180f },
    { -10944.495f, -2123.857f, 92.180f },
};

const Position BIG_BAD_WOLF_RUN_POSITION[4] =
{
    { -10875.456f, -1779.036f, 90.477f },
    { -10872.281f, -1751.638f, 90.477f },
    { -10910.492f, -1747.401f, 90.477f },
    { -10913.391f, -1773.508f, 90.477f },
};

const Position NIGHTBANE_TRANSITION_BOSS_POSITION = { -11160.646f, -1932.773f, 91.473f }; // near some ribs
const Position NIGHTBANE_FINAL_BOSS_POSITION = { -11173.530f, -1940.707f, 91.473f };
const Position NIGHTBANE_RANGED_POSITION1 = { -11145.949f, -1970.927f, 91.473f };
const Position NIGHTBANE_RANGED_POSITION2 = { -11143.594f, -1954.981f, 91.473f };
const Position NIGHTBANE_RANGED_POSITION3 = { -11159.778f, -1961.031f, 91.473f };
const Position NIGHTBANE_FLIGHT_STACK_POSITION = { -11159.555f, -1893.526f, 91.473f }; // Broken Barrel
const Position NIGHTBANE_RAIN_OF_BONES_POSITION = { -11165.233f, -1911.123f, 91.473f };

bool IsCastingArcaneExplosion(Unit* aran)
{
    return aran && aran->HasUnitState(UNIT_STATE_CASTING) && aran->FindCurrentSpellBySpellId(
        static_cast<uint32>(KarazhanSpells::SPELL_ARCANE_EXPLOSION));
}

bool IsFlameWreathActive(PlayerbotAI* botAI, Player* bot)
{
    Unit* aran =
        botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "shade of aran")->Get();
    Spell* currentSpell = aran ? aran->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;

    if (currentSpell && currentSpell->m_spellInfo &&
        currentSpell->m_spellInfo->Id ==
            static_cast<uint32>(KarazhanSpells::SPELL_FLAME_WREATH_CAST))
    {
        return true;
    }

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_FLAME_WREATH_AURA)))
                return true;
        }
    }

    return false;
}

bool IsBanishPhase(Unit* netherspite)
{
    return netherspite && netherspite->HasAura(
        static_cast<uint32>(KarazhanSpells::SPELL_NETHERSPITE_BANISHED));
}

// Red beam blockers: tank bots, no Nether Exhaustion Red
std::vector<Player*> GetRedBlockers(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> redBlockers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsTank(member) ||
            !GET_PLAYERBOT_AI(member) || member->HasAura(
                static_cast<uint32>(KarazhanSpells::SPELL_NETHER_EXHAUSTION_RED)))
        {
            continue;
        }

        redBlockers.push_back(member);
    }

    return redBlockers;
}

// Blue beam blockers: DPS bots, excluding Warrior/Rogue/DK
// no Nether Exhaustion Blue and <25 stacks of Blue Beam debuff
std::vector<Player*> GetBlueBlockers(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> blueBlockers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || !botAI->IsDps(member))
            continue;

        if (member->getClass() == CLASS_WARRIOR || member->getClass() == CLASS_ROGUE ||
            member->getClass() == CLASS_DEATH_KNIGHT)
        {
            continue;
        }

        if (member->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_NETHER_EXHAUSTION_BLUE)))
            continue;

        Aura* blueBuff = member->GetAura(
            static_cast<uint32>(KarazhanSpells::SPELL_BLUE_BEAM_DEBUFF));
        if (!blueBuff || blueBuff->GetStackAmount() < 25)
            blueBlockers.push_back(member);
    }

    return blueBlockers;
}

// Green beam blockers:
// (1) Prioritize Rogues and non-tank Warrior and DK bots, no Nether Exhaustion Green
// (2) Then assign Healer bots, no Nether Exhaustion Green and <25 stacks of Green Beam debuff
std::vector<Player*> GetGreenBlockers(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> greenBlockers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || !botAI->IsDps(member))
            continue;

        if (member->getClass() != CLASS_WARRIOR && member->getClass() != CLASS_ROGUE &&
            member->getClass() != CLASS_DEATH_KNIGHT)
        {
            continue;
        }

        if (!member->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_NETHER_EXHAUSTION_GREEN)))
            greenBlockers.push_back(member);
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || !botAI->IsHeal(member))
            continue;

        Aura* greenBuff = member->GetAura(
            static_cast<uint32>(KarazhanSpells::SPELL_GREEN_BEAM_DEBUFF));
        if (!member->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_NETHER_EXHAUSTION_GREEN)) &&
            (!greenBuff || greenBuff->GetStackAmount() < 25))
        {
            greenBlockers.push_back(member);
        }
    }

    return greenBlockers;
}

std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(PlayerbotAI* botAI, Player* bot)
{
    static ObjectGuid currentRedBlocker;
    static ObjectGuid currentGreenBlocker;
    static ObjectGuid currentBlueBlocker;

    Player* redBlocker = nullptr;
    Player* greenBlocker = nullptr;
    Player* blueBlocker = nullptr;

    std::vector<Player*> redBlockers = GetRedBlockers(botAI, bot);
    if (!redBlockers.empty())
    {
        auto it = std::find_if(redBlockers.begin(), redBlockers.end(), [](Player* player)
        {
            return player && player->GetGUID() == currentRedBlocker;
        });

        if (it != redBlockers.end())
            redBlocker = *it;
        else
            redBlocker = redBlockers.front();

        currentRedBlocker = redBlocker ? redBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        currentRedBlocker = ObjectGuid::Empty;
        redBlocker = nullptr;
    }

    std::vector<Player*> greenBlockers = GetGreenBlockers(botAI, bot);
    if (!greenBlockers.empty())
    {
        auto it = std::find_if(greenBlockers.begin(), greenBlockers.end(), [](Player* player)
        {
            return player && player->GetGUID() == currentGreenBlocker;
        });

        if (it != greenBlockers.end())
            greenBlocker = *it;
        else
            greenBlocker = greenBlockers.front();

        currentGreenBlocker = greenBlocker ? greenBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        currentGreenBlocker = ObjectGuid::Empty;
        greenBlocker = nullptr;
    }

    std::vector<Player*> blueBlockers = GetBlueBlockers(botAI, bot);
    if (!blueBlockers.empty())
    {
        auto it = std::find_if(blueBlockers.begin(), blueBlockers.end(), [](Player* player)
        {
            return player && player->GetGUID() == currentBlueBlocker;
        });

        if (it != blueBlockers.end())
            blueBlocker = *it;
        else
            blueBlocker = blueBlockers.front();

        currentBlueBlocker = blueBlocker ? blueBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        currentBlueBlocker = ObjectGuid::Empty;
        blueBlocker = nullptr;
    }

    return std::make_tuple(redBlocker, greenBlocker, blueBlocker);
}

std::vector<Unit*> GetAllVoidZones(PlayerbotAI *botAI, Player* bot)
{
    std::vector<Unit*> voidZones;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 30.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(KarazhanNpcs::NPC_VOID_ZONE), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            voidZones.push_back(creature);
    }

    return voidZones;
}

bool IsSafePosition(float x, float y, const std::vector<Unit*>& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        float dist = hazard->GetDistance2d(x, y);
        if (dist < hazardRadius)
            return false;
    }

    return true;
}

std::vector<Unit*> GetSpawnedInfernals(Player* bot)
{
    std::vector<Unit*> infernals;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 100.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(KarazhanNpcs::NPC_NETHERSPITE_INFERNAL), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            infernals.push_back(creature);
    }

    return infernals;
}

bool IsStraightPathSafe(
    Position const& start, Position const& target, std::vector<Unit*> const& hazards,
    float hazardRadius, float stepSize)
{
    float sx = start.GetPositionX();
    float sy = start.GetPositionY();
    float tx = target.GetPositionX();
    float ty = target.GetPositionY();

    const float totalDist = start.GetExactDist2d(target.GetPositionX(), target.GetPositionY());
    if (totalDist == 0.0f)
        return true;

    for (float checkDist = 0.0f; checkDist <= totalDist; checkDist += stepSize)
    {
        float t = checkDist / totalDist;
        float checkX = sx + (tx - sx) * t;
        float checkY = sy + (ty - sy) * t;
        for (Unit* hazard : hazards)
        {
            const float hx = checkX - hazard->GetPositionX();
            const float hy = checkY - hazard->GetPositionY();
            if ((hx*hx + hy*hy) < hazardRadius * hazardRadius)
                return false;
        }
    }

    return true;
}

bool TryFindSafePositionWithSafePath(
    Player* bot, float originX, float originY, float originZ, float centerX, float centerY, float centerZ,
    const std::vector<Unit*>& hazards, float safeDistance, float stepSize, uint8 numAngles,
    float maxSampleDist, bool requireSafePath, float& bestDestX, float& bestDestY, float& bestDestZ)
{
    float bestMoveDist = std::numeric_limits<float>::max();
    bool found = false;

    for (int i = 0; i < numAngles; ++i)
    {
        float angle = (2.0f * M_PI * i) / numAngles;
        float dx = cos(angle);
        float dy = sin(angle);

        for (float dist = stepSize; dist <= maxSampleDist; dist += stepSize)
        {
            float x = centerX + dx * dist;
            float y = centerY + dy * dist;
            float z = centerZ;
            float destX = x, destY = y, destZ = z;
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, centerX, centerY, centerZ,
                                                                destX, destY, destZ, true))
                continue;

            if (!IsSafePosition(destX, destY, hazards, safeDistance))
                continue;

            if (requireSafePath)
            {
                if (!IsStraightPathSafe(Position(originX, originY, originZ), Position(destX, destY, destZ),
                                        hazards, safeDistance, stepSize))
                    continue;
            }

            const float moveDist = Position(originX, originY, originZ).GetExactDist2d(destX, destY);
            if (moveDist < bestMoveDist)
            {
                bestMoveDist = moveDist;
                bestDestX = destX;
                bestDestY = destY;
                bestDestZ = destZ;
                found = true;
            }
        }
    }

    return found;
}

}
