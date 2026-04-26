/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINGREATERBLESSINGACTION_H
#define _PLAYERBOT_PALADINGREATERBLESSINGACTION_H

#include <vector>

#include "Action.h"
#include "PaladinBlessingPriorityData.h"
#include "Playerbots.h"

class CastGreaterBlessingAssignmentAction : public Action
{
public:
    CastGreaterBlessingAssignmentAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
    bool isUseful() override;
    bool HasPendingAssignment();

private:
    struct PlayerAssignment
    {
        Player* player = nullptr;
        ai::gbless::BlessingType blessing = ai::gbless::BLESSING_NONE;
    };

    bool GetAssignments(std::vector<PlayerAssignment>& outAssignments);
    bool FindPendingAssignment(PlayerAssignment& outAssignment,
                               ai::gbless::BlessingType& outCastType,
                               std::string& outSpellName);
};

namespace ai::gbless
{
    bool IsEligibleGroupForAutoBlessings(Group const* group);
    bool IsAutoGreaterBlessingActive(Player const* bot);
}

#endif
