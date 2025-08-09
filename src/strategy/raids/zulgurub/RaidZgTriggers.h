#ifndef _PLAYERBOT_RAIDZGTRIGGERS_H
#define _PLAYERBOT_RAIDZGTRIGGERS_H

#include "PlayerbotAI.h"
#include "Trigger.h"

class MandokirThreateningGazeCastTrigger : public Trigger
{
public:
    MandokirThreateningGazeCastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mandokir threatening gaze cast")
    {
        LOG_INFO("playerbots", "[GazeCast] MandokirThreateningGazeCastTrigger constructed for {}", botAI->GetBot()->GetName());
    }
    bool IsActive() override;

    static time_t lastGazeTime;
};

class MandokirThreateningGazeEndTrigger : public Trigger
{
private:
    bool hadAura = false;

public:
    MandokirThreateningGazeEndTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mandokir threatening gaze end")
    {
        LOG_INFO("playerbots", "[ResumeTrigger] MandokirThreateningGazeEndTrigger constructed for {}", botAI->GetBot()->GetName());
    }
    bool IsActive() override;
};

#endif
