#include "RaidZgTriggers.h"
#include "Playerbots.h"
#include "Spell.h"
#include "Unit.h"
#include <unordered_map>
#include <ctime>

namespace MandokirGazeState
{
std::unordered_map<ObjectGuid, time_t> lastGazeTimePerBot;
}

bool MandokirThreateningGazeCastTrigger::IsActive()
{
    if (!bot || !bot->IsInWorld())
        return false;

    auto context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    Creature* mandokir = bot->FindNearestCreature(11382, 100.0f); // Bloodlord Mandokir
    if (!mandokir || !mandokir->IsAlive())
        return false;

    Spell* currentSpell = mandokir->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell || !currentSpell->m_spellInfo)
        return false;

    if (currentSpell->m_spellInfo->Id != 24314) // Threatening Gaze
        return false;

    Unit* target = currentSpell->m_targets.GetUnitTarget();
    if (!target || target->GetGUID() != bot->GetGUID())
        return false;

    MandokirGazeState::lastGazeTimePerBot[bot->GetGUID()] = time(nullptr);
    return true;
}

bool MandokirThreateningGazeEndTrigger::IsActive()
{
    if (!bot || !bot->IsInWorld())
        return false;

    auto context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    Value<std::list<Unit*>>* value = context->GetValue<std::list<Unit*>>("nearest hostile unit");
    if (!value)
        return false;

    std::list<Unit*> nearby = value->Get();

    bool mandokirAlive = false;
    for (Unit* unit : nearby)
    {
        if (unit && unit->GetEntry() == 11382 && unit->IsAlive())
        {
            mandokirAlive = true;
            break;
        }
    }

    if (!mandokirAlive)
    {
        LOG_INFO("playerbots", "[ResumeTrigger] Mandokir is dead, skipping resume check for {}", bot->GetName());
        MandokirGazeState::lastGazeTimePerBot.erase(bot->GetGUID());  // Clean up stale entry
        return false;
    }

    constexpr int delaySeconds = 7;
    constexpr int expirySeconds = 15;

    auto it = MandokirGazeState::lastGazeTimePerBot.find(bot->GetGUID());
    if (it == MandokirGazeState::lastGazeTimePerBot.end())
    {
        LOG_INFO("playerbots", "[ResumeTrigger] No gaze entry for {}", bot->GetName());
        return false;
    }

    time_t elapsed = time(nullptr) - it->second;

    LOG_INFO("playerbots", "[ResumeTrigger] Elapsed time for {}: {}", bot->GetName(), elapsed);

    if (elapsed >= expirySeconds)
    {
        LOG_INFO("playerbots", "[ResumeTrigger] Gaze expired for {}", bot->GetName());
        MandokirGazeState::lastGazeTimePerBot.erase(it);
        return false;
    }

    if (elapsed >= delaySeconds)
    {
        LOG_INFO("playerbots", "[ResumeTrigger] Triggering resume for {}", bot->GetName());
        MandokirGazeState::lastGazeTimePerBot.erase(it);
        return true;
    }

    return false;
}


