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

class RancidMushroomMoveAwayFromMushroomSporeCloudAction : public MovementAction
{
public:
    RancidMushroomMoveAwayFromMushroomSporeCloudAction(PlayerbotAI* botAI, std::string const name = "rancid mushroom move away from mushroom spore cloud") : MovementAction(botAI, name) {}

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

class HydrossTheUnstableMisdirectBossToTankAction : public Action
{
public:
    HydrossTheUnstableMisdirectBossToTankAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable misdirect boss to tank") : Action(botAI, name) {}

    bool Execute(Event event) override;

private:
    bool TryMisdirectToFrostTank(Unit* hydross, Group* group);
    bool TryMisdirectToNatureTank(Unit* hydross, Group* group);
};

class HydrossTheUnstableManagePhaseChangeTimersAction : public Action
{
public:
    HydrossTheUnstableManagePhaseChangeTimersAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable manage phase change timers") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableManageDpsTimersAction : public Action
{
public:
    HydrossTheUnstableManageDpsTimersAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable manage dps timers") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowPositionMainTankAction : public AttackAction
{
public:
    TheLurkerBelowPositionMainTankAction(PlayerbotAI* botAI, std::string const name = "the lurker below position main tank") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowPositionOtherMeleeAction : public MovementAction
{
public:
    TheLurkerBelowPositionOtherMeleeAction(PlayerbotAI* botAI, std::string const name = "the lurker below position other melee") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowMeleeRunAroundBehindBossAction : public MovementAction
{
public:
    TheLurkerBelowMeleeRunAroundBehindBossAction(PlayerbotAI* botAI, std::string const name = "the lurker below melee run around behind boss") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowPositionRangedDpsAction : public MovementAction
{
public:
    TheLurkerBelowPositionRangedDpsAction(PlayerbotAI* botAI, std::string const name = "the lurker below position ranged dps") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowPositionHealerAction : public MovementAction
{
public:
    TheLurkerBelowPositionHealerAction(PlayerbotAI* botAI, std::string const name = "the lurker below position healer") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowManageSpoutTimerAction : public Action
{
public:
    TheLurkerBelowManageSpoutTimerAction(PlayerbotAI* botAI, std::string const name = "the lurker below manage spout timer") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindTargetSpellbindersAction : public Action
{
public:
    LeotherasTheBlindTargetSpellbindersAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind target spellbinders") : Action(botAI, name) {}

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

/* class LeotherasTheBlindDemonFormPositionMeleeAction : public MovementAction
{
public:
    LeotherasTheBlindDemonFormPositionMeleeAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind demon form position melee") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
}; */

class LeotherasTheBlindInnerDemonCheatAction : public AttackAction
{
public:
    LeotherasTheBlindInnerDemonCheatAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind inner demon cheat") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindFinalPhaseAssignDpsPriorityAction : public AttackAction
{
public:
    LeotherasTheBlindFinalPhaseAssignDpsPriorityAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind final phase assign dps priority") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class LeotherasTheBlindMisdirectBossToDemonFormTankAction : public AttackAction
{
public:
    LeotherasTheBlindMisdirectBossToDemonFormTankAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind misdirect boss to demon form tank") : AttackAction(botAI, name) {}

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

class FathomLordKarathressMisdirectBossesToTanksAction : public AttackAction
{
public:
    FathomLordKarathressMisdirectBossesToTanksAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress misdirect bosses to tanks") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressAssignDpsPriorityAction : public AttackAction
{
public:
    FathomLordKarathressAssignDpsPriorityAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress assign dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressPositionCaribdisTankHealerAction : public MovementAction
{
public:
    FathomLordKarathressPositionCaribdisTankHealerAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress position caribdis tank healer") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FathomLordKarathressManageDpsTimerAction : public Action
{
public:
    FathomLordKarathressManageDpsTimerAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress manage dps timer") : Action(botAI, name) {}
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

private:
    bool MoveToPhase1TankPosition(Unit* tidewalker);
    bool MoveToPhase2TankPosition(Unit* tidewalker);
};

class MorogrimTidewalkerPhase2RepositionRangedAction : public MovementAction
{
public:
    MorogrimTidewalkerPhase2RepositionRangedAction(PlayerbotAI* botAI, std::string const name = "morogrim tidewalker phase 2 reposition ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MorogrimTidewalkerResetPhaseTransitionStepsAction : public Action
{
public:
    MorogrimTidewalkerResetPhaseTransitionStepsAction(PlayerbotAI* botAI, std::string const name = "morogrim tidewalker reset phase transition steps") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjMainTankPositionBossAction : public AttackAction
{
public:
    LadyVashjMainTankPositionBossAction(PlayerbotAI* botAI, std::string const name = "lady vashj main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjPhase1PositionRangedAction : public MovementAction
{
public:
    LadyVashjPhase1PositionRangedAction(PlayerbotAI* botAI, std::string const name = "lady vashj phase 1 position ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjSetGroundingTotemInMainTankGroupAction : public MovementAction
{
public:
    LadyVashjSetGroundingTotemInMainTankGroupAction(PlayerbotAI* botAI, std::string const name = "lady vashj set grounding totem in main tank group") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjStaticChargeMoveAwayFromGroupAction : public MovementAction
{
public:
    LadyVashjStaticChargeMoveAwayFromGroupAction(PlayerbotAI* botAI, std::string const name = "lady vashj static charge move away from group") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjMisdirectBossToMainTankAction : public AttackAction
{
public:
    LadyVashjMisdirectBossToMainTankAction(PlayerbotAI* botAI, std::string const name = "lady vashj misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjAttackAndMoveAwayFromStriderAction : public AttackAction
{
public:
    LadyVashjAttackAndMoveAwayFromStriderAction(PlayerbotAI* botAI, std::string const name = "lady vashj attack and move away from strider") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjAssignPhase2DpsPriorityAction : public AttackAction
{
public:
    LadyVashjAssignPhase2DpsPriorityAction(PlayerbotAI* botAI, std::string const name = "lady vashj assign phase 2 dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjAssistantsFollowMasterInPhase2Action : public MovementAction
{
public:
    LadyVashjAssistantsFollowMasterInPhase2Action(PlayerbotAI* botAI, std::string const name = "lady vashj assistants follow master in phase 2") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjTaintedElementalCheatAction : public AttackAction
{
public:
    LadyVashjTaintedElementalCheatAction(PlayerbotAI* botAI, std::string const name = "lady vashj tainted elemental cheat") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    ObjectGuid lastTaintedGuid;
};

class LadyVashjPassTheTaintedCoreAction : public MovementAction
{
public:
    LadyVashjPassTheTaintedCoreAction(PlayerbotAI* botAI, std::string const name = "lady vashj pass the tainted core") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    void LineUpFirstCorePasser(Player* master, Unit* closestTrigger);
    void LineUpSecondCorePasser(Player* firstCorePasser, Unit* closestTrigger);
    bool IsFirstCorePasserInIntendedPosition(Player* designatedMaster, Player* firstCorePasser, Unit* closestTrigger);
    bool IsSecondCorePasserInIntendedPosition(Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger);
    void LineUpThirdCorePasser(Player* secondCorePasser, Unit* closestTrigger);
    bool CanUseGenerator();
    bool UseCoreOnNearestGenerator();
};

class LadyVashjAvoidToxicSporesAction : public MovementAction
{
public:
    LadyVashjAvoidToxicSporesAction(PlayerbotAI* botAI, std::string const name = "lady vashj avoid toxic spores") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

    /* static constexpr float HAZARD_RADIUS = 10.0f;
    static constexpr float STEP_SIZE = 1.0f;
    static constexpr uint8 NUM_ANGLES = 16;
    static constexpr float MAX_SAMPLE_DIST = 30.0f; */
    static constexpr float HAZARD_RADIUS = 13.0f;
    static constexpr float STEP_SIZE = 1.0f;
    static constexpr float MAX_SAMPLE_DIST = 30.0f;
    static constexpr uint32 MAX_NODES = 500;

private:
    std::vector<Unit*> GetAllSporeDropTriggers(PlayerbotAI* botAI, Player* bot);
    /* bool IsStraightPathSafe(const Position& start, const Position& target, const std::vector<Unit*>& hazards);
    bool TryFindSafePositionWithSafePath(
            Player* bot, const std::vector<Unit*>& hazards, bool requireSafePath, float& bestDestX, float& bestDestY, float& bestDestZ); */
    bool FindBestSafePositionBFS(Player* bot, const std::vector<Unit*>& hazards, float& bestX, float& bestY, float& bestZ);
};

class LadyVashjManageTrackersAction : public Action
{
public:
    LadyVashjManageTrackersAction(PlayerbotAI* botAI, std::string const name = "lady vashj manage trackers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
