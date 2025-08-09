#include "RaidZgActions.h"
#include "Playerbots.h"
#include "Engine.h"

bool MandokirThreateningGazeFreezeAction::Execute(Event event)
{
LOG_INFO("bot", "Executing MandokirThreateningGazeFreezeAction for {}", bot->GetName());

    botAI->Reset();

    LOG_INFO("bot", "StopMoving called for {}", GetName());
    bot->StopMoving();

    LOG_INFO("bot", "AttackStop called for {}", GetName());
    bot->AttackStop();

    LOG_INFO("bot", "SetTarget called for {}", GetName());
    bot->SetTarget(ObjectGuid::Empty);

    if (bot->IsNonMeleeSpellCast(false))
    {
        LOG_INFO("bot", "Interrupting non-melee spell for {}", GetName());
        bot->InterruptNonMeleeSpells(false);
    }

    LOG_INFO("bot", "Clearing movement for {}", _owner->GetName());
    bot->GetMotionMaster()->Clear(false);

    LOG_INFO("bot", "CombatStop called for {}", GetName());
    bot->CombatStop(true);

    LOG_INFO("bot", "Changing engine to non-combat for {}", GetName());
    botAI->ChangeEngine(BOT_STATE_NON_COMBAT);

    LOG_INFO("bot", "Strategy changing to passive for {}", GetName());
    botAI->ChangeStrategy("+passive", BOT_STATE_NON_COMBAT);

    LOG_INFO("bot", "Strategy changing to stay for {}", GetName());
    botAI->ChangeStrategy("+stay", BOT_STATE_NON_COMBAT);
    
    return true;
}

bool MandokirThreateningGazeResumeAction::Execute(Event event)
{    
    LOG_INFO("bot", "Strategy changing to active for {}", GetName());
    botAI->ChangeStrategy("-passive", BOT_STATE_NON_COMBAT);
    LOG_INFO("bot", "Strategy changing to follow for {}", GetName());
    botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);
    return true;
}
