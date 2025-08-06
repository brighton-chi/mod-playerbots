#include "RaidZgTriggers.h"
#include "Playerbots.h"

time_t MandokirThreateningGazeCastTrigger::lastGazeTime = 0;

bool MandokirThreateningGazeCastTrigger::IsActive()
{
    if (bot->HasAura(24314))
    {
        lastGazeTime = time(nullptr);  // Record the time of activation
        return true;
    }
    return false;
}

bool MandokirThreateningGazeEndTrigger::IsActive()
{
    const int delaySeconds = 7;
    time_t now = time(nullptr);

    if (MandokirThreateningGazeCastTrigger::lastGazeTime == 0)
        return false;

    if (now - MandokirThreateningGazeCastTrigger::lastGazeTime >= delaySeconds)
    {
        MandokirThreateningGazeCastTrigger::lastGazeTime = 0;  // Reset to avoid repeat firing
        return true;
    }

    return false;
}

