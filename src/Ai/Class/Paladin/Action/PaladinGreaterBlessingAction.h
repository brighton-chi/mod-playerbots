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

class UntypedValue;

namespace ai::gbless
{
    struct GreaterBlessingPlayerAssignment
    {
        Player* player = nullptr;
        BlessingType blessing = BLESSING_NONE;
    };

    struct CachedBlessingBucketAssignment
    {
        uint8 classId = 0;
        RoleProfile role = ROLE_CASTER;
        bool byRole = false;
        BlessingType blessing = BLESSING_NONE;
    };

    struct CachedBlessingAssignments
    {
        uint32 groupKey = 0;
        bool valid = false;
        std::vector<CachedBlessingBucketAssignment> assignments;
    };

    UntypedValue* greater_blessing_assignments_value(PlayerbotAI* botAI);
    bool IsEligibleGroupForAutoBlessings(Group const* group);
    bool IsAutoGreaterBlessingActive(Player const* bot);
}

class CastGreaterBlessingAssignmentAction : public Action
{
public:
    CastGreaterBlessingAssignmentAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
    bool isUseful() override;
    bool HasPendingAssignment();

private:
    bool GetAssignments(
        std::vector<ai::gbless::CachedBlessingBucketAssignment>& outAssignments);
    bool FindPendingAssignment(
        ai::gbless::GreaterBlessingPlayerAssignment& outAssignment,
        ai::gbless::BlessingType& outCastType, std::string& outSpellName);
};

#endif
