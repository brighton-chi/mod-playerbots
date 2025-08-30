#include "RaidKarazhanStrategy.h"

void RaidKarazhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
        triggers.push_back(new TriggerNode(
            "karazhan moroes", NextAction::array(0,
            new NextAction("karazhan moroes mark target", ACTION_RAID + 1),
            nullptr)));

        triggers.push_back(new TriggerNode(
            "karazhan maiden of virtue", NextAction::array(0,
            new NextAction("karazhan maiden of virtue position boss", ACTION_MOVE + 1),
            new NextAction("karazhan maiden of virtue position ranged", ACTION_MOVE + 2),
            nullptr)));

        triggers.push_back(new TriggerNode(
            "karazhan big bad wolf", NextAction::array(0,
            new NextAction("karazhan big bad wolf run away", ACTION_EMERGENCY + 1),
            nullptr)));

        triggers.push_back(new TriggerNode(
            "karazhan romulo julianne", NextAction::array(0,
            new NextAction("karazhan romulo julianne mark target", ACTION_RAID + 1),
            nullptr)));

        triggers.push_back(new TriggerNode(
            "karazhan wizard of oz", NextAction::array(0,
            new NextAction("karazhan wizard of oz mark target", ACTION_RAID + 1), nullptr)));

        triggers.push_back(new TriggerNode(
            "karazhan the curator", NextAction::array(0,
            new NextAction("karazhan the curator mark target", ACTION_RAID + 1),
            new NextAction("karazhan the curator position boss", ACTION_MOVE + 1),
            new NextAction("karazhan the curator spread ranged", ACTION_MOVE + 2),
            nullptr)));

        triggers.push_back(new TriggerNode(
            "karazhan terestian illhoof", NextAction::array(0,
            new NextAction("karazhan terestian illhoof mark target", ACTION_RAID + 1),
            nullptr)));

    triggers.push_back(new TriggerNode(
            "karazhan shade of aran", NextAction::array(0,
            new NextAction("karazhan shade of aran arcane explosion", ACTION_EMERGENCY + 1),
            new NextAction("karazhan shade of aran flame wreath", ACTION_EMERGENCY +2),
            new NextAction("karazhan shade of aran spread ranged", ACTION_MOVE + 1),
            new NextAction("karazhan shade of aran mark target", ACTION_RAID +1),
            nullptr)));

    /*triggers.push_back(new TriggerNode(
            "karazhan netherspite", NextAction::array(0,
            new NextAction("karazhan netherspite avoid void zone", ACTION_EMERGENCY + 1),
            new NextAction("karazhan netherspite soak beams", ACTION_MOVE + 1),
            nullptr)));*/
}

void RaidKarazhanStrategy::InitMultipliers(std::vector<Multiplier*>& /*multipliers*/)
{
}
