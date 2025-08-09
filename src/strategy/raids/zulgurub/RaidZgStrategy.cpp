#include "RaidZgStrategy.h"
#include "Strategy.h"

void RaidZgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    InitMandokirTriggers(triggers);
}

void RaidZgStrategy::InitMandokirTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("mandokir threatening gaze cast",
        NextAction::array(0, new NextAction("mandokir threatening gaze freeze", ACTION_RAID), nullptr)));
    triggers.push_back(new TriggerNode("mandokir threatening gaze end",
        NextAction::array(0, new NextAction("mandokir threatening gaze resume", ACTION_RAID), nullptr)));
}
