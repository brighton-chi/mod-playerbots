#ifndef _PLAYERBOT_RAIDSSCACTIONS_H
#define _PLAYERBOT_RAIDSSCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class UnderbogColossusEscapeToxicPoolAction : public MovementAction
{
public:
    UnderbogColossusEscapeToxicPoolAction(PlayerbotAI* botAI, std::string const name = "underbog colossus escape toxic pool") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

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

class HydrossTheUnstablePrioritizeElementalAddsAction : public AttackAction
{
public:
    HydrossTheUnstablePrioritizeElementalAddsAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable prioritize elemental adds") : AttackAction(botAI, name) {}

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

class HydrossTheUnstableStopDpsUponPhaseChangeAction : public Action
{
public:
    HydrossTheUnstableStopDpsUponPhaseChangeAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable stop dps upon phase change") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableManageTimersAction : public Action
{
public:
    HydrossTheUnstableManageTimersAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable manage timers") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowRunAroundBehindBossAction : public MovementAction
{
public:
    TheLurkerBelowRunAroundBehindBossAction(PlayerbotAI* botAI, std::string const name = "the lurker below run around behind boss") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowPositionMainTankAction : public AttackAction
{
public:
    TheLurkerBelowPositionMainTankAction(PlayerbotAI* botAI, std::string const name = "the lurker below position main tank") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TheLurkerBelowSpreadRangedAction : public MovementAction
{
public:
    TheLurkerBelowSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "the lurker below spread ranged") : MovementAction(botAI, name) {}

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

class LeotherasTheBlindDemonFormTankAttackBossAction : public AttackAction
{
public:
    LeotherasTheBlindDemonFormTankAttackBossAction(PlayerbotAI* botAI, std::string const name = "leotheras the blind demon form tank attack boss") : AttackAction(botAI, name) {}

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

class FathomLordKarathressAssignDpsPriorityAction : public AttackAction
{
public:
    FathomLordKarathressAssignDpsPriorityAction(PlayerbotAI* botAI, std::string const name = "fathom-lord karathress assign dps priority") : AttackAction(botAI, name) {}
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

class LadyVashjMisdirectStriderToFirstAssistTankAction : public AttackAction
{
public:
    LadyVashjMisdirectStriderToFirstAssistTankAction(PlayerbotAI* botAI, std::string const name = "lady vashj misdirect strider to first assist tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjTankAttackAndMoveAwayStriderAction : public AttackAction
{
public:
    LadyVashjTankAttackAndMoveAwayStriderAction(PlayerbotAI* botAI, std::string const name = "lady vashj tank attack and move away strider") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjAssignDpsPriorityAction : public AttackAction
{
public:
    LadyVashjAssignDpsPriorityAction(PlayerbotAI* botAI, std::string const name = "lady vashj assign dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjTeleportToTaintedElementalAction : public AttackAction
{
public:
    LadyVashjTeleportToTaintedElementalAction(PlayerbotAI* botAI, std::string const name = "lady vashj teleport to tainted elemental") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    ObjectGuid lastTaintedGuid;
};

class LadyVashjLootTaintedCoreAction : public MovementAction
{
public:
    LadyVashjLootTaintedCoreAction(PlayerbotAI* botAI, std::string const name = "lady vashj loot tainted core") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjPassTheTaintedCoreAction : public MovementAction
{
public:
    LadyVashjPassTheTaintedCoreAction(PlayerbotAI* botAI, std::string const name = "lady vashj pass the tainted core") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool LineUpFirstCorePasser(Player* designatedLooter, Unit* closestTrigger);
    bool LineUpSecondCorePasser(Player* firstCorePasser, Unit* closestTrigger);
    bool LineUpThirdCorePasser(Player* secondCorePasser, Unit* closestTrigger);
    bool LineUpFourthCorePasser(Player* thirdCorePasser, Unit* closestTrigger);
    bool IsFirstCorePasserInIntendedPosition(Player* designatedLooter, Player* firstCorePasser, Unit* closestTrigger);
    bool IsSecondCorePasserInIntendedPosition(Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger);
    bool IsThirdCorePasserInIntendedPosition(Player* secondCorePasser, Player* thirdCorePasser, Unit* closestTrigger);
    bool IsFourthCorePasserInIntendedPosition(Player* thirdCorePasser, Player* fourthCorePasser, Unit* closestTrigger);
    void ScheduleStoreCoreAfterImbue(PlayerbotAI* botAI, Player* giver, Player* receiver);
    bool UseCoreOnNearestGenerator();
};

class LadyVashjDestroyTaintedCoreAction : public Action
{
public:
    LadyVashjDestroyTaintedCoreAction(PlayerbotAI* botAI, std::string const name = "lady vashj destroy tainted core") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjAvoidToxicSporesAction : public MovementAction
{
public:
    LadyVashjAvoidToxicSporesAction(PlayerbotAI* botAI, std::string const name = "lady vashj avoid toxic spores") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
    static std::vector<Unit*> GetAllSporeDropTriggers(PlayerbotAI* botAI, Player* bot);

private:
    Position FindSafestNearbyPosition(const std::vector<Unit*>& spores, const Position& position, float maxRadius, float hazardRadius);
    bool IsPathSafeFromSpores(const Position& start, const Position& end, const std::vector<Unit*>& spores, float hazardRadius);
};

class LadyVashjUseFreeActionAbilitiesAction : public Action
{
public:
    LadyVashjUseFreeActionAbilitiesAction(PlayerbotAI* botAI, std::string const name = "lady vashj use free action abilities") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class LadyVashjManageTrackersAction : public Action
{
public:
    LadyVashjManageTrackersAction(PlayerbotAI* botAI, std::string const name = "lady vashj manage trackers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
