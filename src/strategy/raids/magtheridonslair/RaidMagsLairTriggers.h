#ifndef _PLAYERBOT_RAIDMAGSLAIRTRIGGERS_H
#define _PLAYERBOT_RAIDMAGSLAIRTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"

class MagtheridonTrigger : public Trigger
{
public:
    MagtheridonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon") {}
    bool IsActive() override;
};

#endif
