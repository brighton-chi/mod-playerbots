#include <algorithm>
#include <map>

#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"


namespace TempestKeepHelpers
{

    namespace TempestKeepLocations
    {
        const Location VoidReaverTankPosition   = { 423.845f,  371.733f, 14.897f }; // middle of room
        const Location AstromancerStackPosition = { 448.398f, -346.526f, 16.802f }; // directly from entrance, on platform, <25 yards from boss
    }

    /*
    // From boss_alar.cpp
    const Position alarPoints[9] =
    {
        {335.638f, 59.4879f, 17.9319f, 4.60f}, //first platform
        {388.751007f, 31.731199f, 20.263599f, 1.61f},
        {388.790985f, -33.105900f, 20.263599f, 0.52f},
        {332.722992f, -61.159f, 17.979099f, 5.71f},
        {258.959015f, -38.687099f, 20.262899f, 5.21f}, //pre-nerf only
        {259.2277997, 35.879002f, 20.263f, 4.81f}, //pre-nerf only
        {332.0f, 0.01f, 43.0f, 0.0f}, //quill
        {331.0f, 0.01f, -2.38f, 0.0f}, //middle (p2)
        {332.0f, 0.01f, 43.0f, 0.0f} // dive
    };

    */

    std::unordered_map<ObjectGuid, Position> initialPositions;
    std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
    {
        if (!target)
            return;

        if (Group* group = bot->GetGroup())
        {
            ObjectGuid currentGuid = group->GetTargetIcon(iconId);
            if (currentGuid != target->GetGUID())
            {
                group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
            }
        }
    }

    void MarkTargetWithSquare(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
    }

    void MarkTargetWithStar(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
    }

    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target)
    {
        if (!target)
            return;

        std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();

        if (currentRti != rtiName || currentTarget != target)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
        }
    }

}
