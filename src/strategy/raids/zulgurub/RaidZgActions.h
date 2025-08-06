#ifndef _PLAYERBOT_RAIDZGACTIONS_H
#define _PLAYERBOT_RAIDZGACTIONS_H

#include "Action.h"

class MandokirThreateningGazeFreezeAction : public Action
{
public:
    MandokirThreateningGazeFreezeAction(PlayerbotAI* botAI) : Action(botAI, "mandokir threatening gaze freeze") {}
    bool Execute(Event event) override;
};

class MandokirThreateningGazeResumeAction : public Action
{
public:
    MandokirThreateningGazeResumeAction(PlayerbotAI* botAI) : Action(botAI, "mandokir threatening gaze resume") {}
    bool Execute(Event event) override;
};

#endif
