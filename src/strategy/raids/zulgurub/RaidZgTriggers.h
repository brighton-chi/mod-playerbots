#ifndef _PLAYERBOT_RAIDZGTRIGGERS_H
#define _PLAYERBOT_RAIDZGTRIGGERS_H

#include "PlayerbotAI.h"
#include "Trigger.h"

class MandokirThreateningGazeCastTrigger : public Trigger
{
public:
    MandokirThreateningGazeCastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mandokir threatening gaze cast") {}
    bool IsActive() override;

    static time_t lastGazeTime;
};

class MandokirThreateningGazeEndTrigger : public Trigger
{
private:
    bool hadAura = false;

public:
    MandokirThreateningGazeEndTrigger(PlayerbotAI* ai) : Trigger(ai, "mandokir threatening gaze end"), hadAura(false) {}

    bool IsActive() override;
};

#endif
