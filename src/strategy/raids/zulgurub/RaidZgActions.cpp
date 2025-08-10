#include "RaidZgActions.h"
#include "Playerbots.h"
#include "Engine.h"

bool MandokirThreateningGazeFreezeAction::Execute(Event event)
{
LOG_INFO("playerbots", "Executing MandokirThreateningGazeFreezeAction for {}", bot->GetName());

    /*//LOG_INFO("playerbots", "Reset called for {}", bot->GetName());    
    botAI->Reset();

    //LOG_INFO("playerbots", "AttackStop called for {}", bot->GetName());
    bot->AttackStop();

    //LOG_INFO("playerbots", "Target cleared for {}", bot->GetName());
    bot->SetTarget(ObjectGuid::Empty);

    if (bot->IsNonMeleeSpellCast(false))
    {
        //LOG_INFO("playerbots", "Interrupting non-melee spell for {}", bot->GetName());
        bot->InterruptNonMeleeSpells(false);
    }

    //LOG_INFO("playerbots", "Clearing movement for {}", bot->GetName());
    bot->GetMotionMaster()->Clear(false);

    //LOG_INFO("playerbots", "CombatStop called for {}", bot->GetName());
    bot->CombatStop(true);

    //LOG_INFO("playerbots", "Changing engine to non-combat for {}", bot->GetName());
    botAI->ChangeEngine(BOT_STATE_NON_COMBAT);

    //  LOG_INFO("playerbots", "Strategy changing to passive for {}", bot->GetName());
    botAI->ChangeStrategy("+passive", BOT_STATE_NON_COMBAT);

    //  LOG_INFO("playerbots", "Strategy changing to stay for {}", bot->GetName());
    botAI->ChangeStrategy("+stay", BOT_STATE_NON_COMBAT); */
    
    return true;
}

bool MandokirThreateningGazeResumeAction::Execute(Event event)
{
    LOG_INFO("playerbots", "[ResumeAction] Executing resume for {}", bot->GetName());

    botAI->ChangeStrategy("-passive", BOT_STATE_NON_COMBAT);
    LOG_INFO("playerbots", "[ResumeAction] Strategy '-passive' removed for {}", bot->GetName());

    botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);
    LOG_INFO("playerbots", "[ResumeAction] Strategy '+follow' added for {}", bot->GetName());

    return true;
}
