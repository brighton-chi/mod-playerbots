#ifndef _PLAYERBOT_RAIDSSCACTIONS_H
#define _PLAYERBOT_RAIDSSCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class GreyheartTidecallerMarkWaterElementalTotemAction : public Action
{
public:
    GreyheartTidecallerMarkWaterElementalTotemAction(PlayerbotAI* botAI, std::string const name = "greyheart tidecaller mark water elemental totem") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstablePositionFrostTankAction : public AttackAction
{
public:
    HydrossTheUnstablePositionFrostTankAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable position frost tank") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstablePositionNatureTankAction : public AttackAction
{
public:
    HydrossTheUnstablePositionNatureTankAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable position nature tank") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableManageDPSTimerAction : public Action
{
public:
    HydrossTheUnstableManageDPSTimerAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable manage dps timer") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableMarkElementalAddsAction : public Action
{
public:
    HydrossTheUnstableMarkElementalAddsAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable mark elemental adds") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableFrostPhaseSpreadOutAction : public MovementAction
{
public:
    HydrossTheUnstableFrostPhaseSpreadOutAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable frost phase spread out") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowRunAroundBehindBossAction : public MovementAction
{
public:
    TheLurkerBelowRunAroundBehindBossAction(PlayerbotAI* botAI, std::string const name = "the lurker below run around behind boss") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindHumanFormPositionBossAction : public AttackAction
{
public:
    LeotherasTheBlindHumanFormPositionBossAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind human form position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindPositionRangedAction : public MovementAction
{
public:
    LeotherasTheBlindPositionRangedAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind position ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindDemonFormPositionBossAction : public AttackAction
{
public:
    LeotherasTheBlindDemonFormPositionBossAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind demon form position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindRunAwayFromWhirlwindAction : public MovementAction
{
public:
    LeotherasTheBlindRunAwayFromWhirlwindAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind run away from whirlwind") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindDemonFormPositionMeleeAction : public MovementAction
{
public:
    LeotherasTheBlindDemonFormPositionMeleeAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind demon form position melee") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindMarkAndAttackInnerDemonAction : public AttackAction
{
public:
    LeotherasTheBlindMarkAndAttackInnerDemonAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind mark and attack inner demon") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindFinalPhaseAssignDPSPriorityAction : public AttackAction
{
public:
    LeotherasTheBlindFinalPhaseAssignDPSPriorityAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind final phase assign dps priority") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindManageTimersAndTrackersAction : public Action
{
public:
    LeotherasTheBlindManageTimersAndTrackersAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind manage timers and trackers") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class FathomLordKarathressMainTankPositionBossAction : public AttackAction
{
public:
    FathomLordKarathressMainTankPositionBossAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressFirstAssistTankPositionSharkkisAction : public AttackAction
{
public:
    FathomLordKarathressFirstAssistTankPositionSharkkisAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress first assist tank position sharkkis") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressSecondAssistTankPositionTidalvessAction : public AttackAction
{
public:
    FathomLordKarathressSecondAssistTankPositionTidalvessAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress second assist tank position tidalvess") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressThirdAssistTankPositionCaribdisAction : public AttackAction
{
public:
    FathomLordKarathressThirdAssistTankPositionCaribdisAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress third assist tank position caribdis") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressPositionCaribdisTankHealerAction : public MovementAction
{
public:
    FathomLordKarathressPositionCaribdisTankHealerAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress position caribdis tank healer") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressMisdirectBossesToTanksAction : public AttackAction
{
public:
    FathomLordKarathressMisdirectBossesToTanksAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress misdirect bosses to tanks") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressAssignMeleeDPSPriorityAction : public AttackAction
{
public:
    FathomLordKarathressAssignMeleeDPSPriorityAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress assign melee dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressAssignRangedDPSPriorityAction : public AttackAction
{
public:
    FathomLordKarathressAssignRangedDPSPriorityAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress assign ranged dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MorogrimTidewalkerMisdirectBossToMainTankAction : public AttackAction
{
public:
    MorogrimTidewalkerMisdirectBossToMainTankAction(PlayerbotAI* botAI, std::string const name = "morogrim tidewalker misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MorogrimTidewalkerMoveBossToTankPositionAction : public AttackAction
{
public:
    MorogrimTidewalkerMoveBossToTankPositionAction(PlayerbotAI* botAI, std::string const name = "morogrim tidewalker move boss to tank position") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MorogrimTidewalkerPhase2RepositionDPSAndHealersAction : public MovementAction
{
public:
    MorogrimTidewalkerPhase2RepositionDPSAndHealersAction(PlayerbotAI* botAI, std::string const name = "morogrim tidewalker phase 2 reposition dps and healers") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
