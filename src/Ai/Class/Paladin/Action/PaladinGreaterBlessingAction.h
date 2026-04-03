/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINGREATERBLESSINGACTION_H
#define _PLAYERBOT_PALADINGREATERBLESSINGACTION_H

#include <map>
#include <vector>

#include "Action.h"
#include "PaladinBlessingPriorityData.h"
#include "Playerbots.h"

// Casts one blessing per Execute() call based on the gblessing
// raid-coordination algorithm. Returns true on successful cast,
// false if nothing to do.
class CastGreaterBlessingAssignmentAction : public Action
{
public:
    CastGreaterBlessingAssignmentAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    // Per-player assignment: what blessing THIS bot should cast on them.
    struct PlayerAssignment
    {
        Player* player = nullptr;
        ai::gbless::BlessingType blessing = ai::gbless::BLESSING_NONE;
    };

    // Build the full assignment table and find the first unbuffed target.
    bool ComputeAssignments(std::vector<PlayerAssignment>& outAssignments);
};

#endif
