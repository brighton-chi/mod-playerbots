#include "RaidZgActions.h"
#include "Playerbots.h"
#include "Engine.h"

bool MandokirThreateningGazeFreezeAction::Execute(Event event)
{
    botAI->Reset();
    bot->StopMoving();
    
    bot->AttackStop();
    bot->SetTarget(ObjectGuid::Empty);
    
    if (bot->IsNonMeleeSpellCast(false))
        bot->InterruptNonMeleeSpells(false);
    
    bot->GetMotionMaster()->Clear(false);
    bot->CombatStop(true);
    
    botAI->ChangeEngine(BOT_STATE_NON_COMBAT);

    botAI->ChangeStrategy("+passive", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+stay", BOT_STATE_NON_COMBAT);
    
    return true;
}

bool MandokirThreateningGazeResumeAction::Execute(Event event)
{    
    botAI->ChangeStrategy("-passive", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);
    return true;
}
