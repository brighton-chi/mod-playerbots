#include "RaidMagtheridonTriggers.h"
#include "RaidMagtheridonHelpers.h"
#include "Playerbots.h"

using namespace MagtheridonHelpers;

bool MagtheridonSetBotSightTrigger::IsActive()
{
    float originalSightDistance = sPlayerbotAIConfig->sightDistance;
    sPlayerbotAIConfig->sightDistance = 150.0f;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");

    bool result = magtheridon && magtheridon->IsAlive() ||
                  channeler && channeler->IsAlive();

    if (!result)
        sPlayerbotAIConfig->sightDistance = originalSightDistance;

    return result;
}

bool MagtheridonHellfireChannelerMainTankTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();

    return group && botAI->IsMainTank(bot) && magtheridon && 
           magtheridon->HasAura(SHADOW_CAGE);
}

bool MagtheridonHellfireChannelerNWChannelerTankTrigger::IsActive()
{
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    return botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond && channelerDiamond->IsAlive();
}

bool MagtheridonHellfireChannelerNEChannelerTankTrigger::IsActive()
{
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    return botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle && channelerTriangle->IsAlive();
}

bool MagtheridonHellfireChannelerMisdirectionTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return group && bot->getClass() == CLASS_HUNTER && 
           (channelerStar && channelerStar->IsAlive() || channelerCircle && channelerCircle->IsAlive());
}

bool MagtheridonHellfireChannelerDPSPriorityTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    if (!group || botAI->IsHeal(bot) || botAI->IsMainTank(bot) ||
        botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond->IsAlive() ||
        botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle->IsAlive())
    {
        return false;
    }

    return channeler && channeler->IsAlive() || magtheridon && magtheridon->IsAlive() && 
           !magtheridon->HasAura(SHADOW_CAGE);
}

bool MagtheridonBurningAbyssalWarlockCCTrigger::IsActive()
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
        return unit && (unit->GetEntry() == BURNING_ABYSSAL || unit->GetEntry() == HELLFIRE_CHANNELER);
    });
}

bool MagtheridonPositionBossTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();

    return magtheridon && magtheridon->IsAlive() && 
           !magtheridon->HasAura(SHADOW_CAGE) && botAI->IsMainTank(bot) && group;
}

bool MagtheridonSpreadRangedTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsRanged(bot) || botAI->IsHeal(bot) || !magtheridon || 
        !magtheridon->IsAlive() || magtheridon->HasAura(SHADOW_CAGE))
    {
        return false;
    }

    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);

        auto timerIt = magtheridonBlastNovaTimer.find(bot->GetMapId());
        if (timerIt != magtheridonBlastNovaTimer.end())
        {
            time_t lastBlastNova = timerIt->second;
            if (now - lastBlastNova >= 49)
            {
                return false;
            }
        }
    }

    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    if (channeler && channeler->IsAlive())
    {
        return false;
    }

    return true;
}

bool MagtheridonSpreadHealerTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();
    if (!group || !magtheridon || !magtheridon->IsAlive() || magtheridon->HasAura(SHADOW_CAGE) || 
        !botAI->IsHeal(bot))
    {
        return false;
    }

    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);

        auto timerIt = magtheridonBlastNovaTimer.find(bot->GetMapId());
        if (timerIt != magtheridonBlastNovaTimer.end())
        {
            time_t lastBlastNova = timerIt->second;
            if (now - lastBlastNova >= 49)
            {
                return false;
            }
        }
    }

    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    if (channeler && channeler->IsAlive())
    {
        return false;
    }

    return true;
}

bool MagtheridonBlastNovaTimerTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon)
    {
        bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
                               magtheridon->FindCurrentSpellBySpellId(BLAST_NOVA);
        UpdateTransitionTimer(magtheridon, blastNovaActive, lastBlastNovaState, magtheridonBlastNovaTimer);
        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(SHADOW_CAGE), lastShadowCageState, magtheridonBlastNovaTimer);
    }

    return false;
}

bool MagtheridonUseManticronCubeTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();
    if (!group || !magtheridon || !magtheridon->IsAlive() || 
        magtheridon->HasAura(SHADOW_CAGE))
    {
        return false;
    }

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
