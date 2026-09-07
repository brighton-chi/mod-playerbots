/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCACTIONS_H
#define PLAYERBOTS_SSCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// General

class SerpentShrineCavernResetEncounterStatesAction : public Action
{
public:
    SerpentShrineCavernResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "serpent shrine cavern reset encounter states") {}
    bool Execute(Event event) override;
};

// Trash

class UnderbogColossusEscapeToxicPoolAction : public MovementAction
{
public:
    UnderbogColossusEscapeToxicPoolAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "underbog colossus escape toxic pool") {}
    bool Execute(Event event) override;
};

class GreyheartTidecallerMarkWaterElementalTotemAction : public Action
{
public:
    GreyheartTidecallerMarkWaterElementalTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "greyheart tidecaller mark water elemental totem") {}
    bool Execute(Event event) override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstablePositionFrostTankAction : public AttackAction
{
public:
    HydrossTheUnstablePositionFrostTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "hydross the unstable position frost tank") {}
    bool Execute(Event event) override;
};

class HydrossTheUnstablePositionNatureTankAction : public AttackAction
{
public:
    HydrossTheUnstablePositionNatureTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "hydross the unstable position nature tank") {}
    bool Execute(Event event) override;
};

class HydrossTheUnstablePrioritizeElementalAddsAction : public AttackAction
{
public:
    HydrossTheUnstablePrioritizeElementalAddsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "hydross the unstable prioritize elemental adds") {}
    bool Execute(Event event) override;
};

class HydrossTheUnstableFrostPhaseSpreadOutAction : public MovementAction
{
public:
    HydrossTheUnstableFrostPhaseSpreadOutAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "hydross the unstable frost phase spread out") {}
    bool Execute(Event event) override;
};

class HydrossTheUnstableMisdirectBossToTankAction : public Action
{
public:
    HydrossTheUnstableMisdirectBossToTankAction(PlayerbotAI* botAI)
        : Action(botAI, "hydross the unstable misdirect boss to tank") {}
    bool Execute(Event event) override;

private:
    bool TryMisdirectToFrostTank(Unit* hydross);
    bool TryMisdirectToNatureTank(Unit* hydross);
};

class HydrossTheUnstableStopDpsUponPhaseChangeAction : public Action
{
public:
    HydrossTheUnstableStopDpsUponPhaseChangeAction(PlayerbotAI* botAI)
        : Action(botAI, "hydross the unstable stop dps upon phase change") {}
    bool Execute(Event event) override;
};

class HydrossTheUnstableManageTimersAction : public Action
{
public:
    HydrossTheUnstableManageTimersAction(PlayerbotAI* botAI)
        : Action(botAI, "hydross the unstable manage timers") {}
    bool Execute(Event event) override;
};

// The Lurker Below

class TheLurkerBelowRunAroundBehindBossAction : public MovementAction
{
public:
    TheLurkerBelowRunAroundBehindBossAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "the lurker below run around behind boss") {}
    bool Execute(Event event) override;
};

class TheLurkerBelowPositionMainTankAction : public AttackAction
{
public:
    TheLurkerBelowPositionMainTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "the lurker below position main tank") {}
    bool Execute(Event event) override;
};

class TheLurkerBelowSpreadRangedInArcAction : public MovementAction
{
public:
    TheLurkerBelowSpreadRangedInArcAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "the lurker below spread ranged in arc") {}
    bool Execute(Event event) override;
};

class TheLurkerBelowTanksPickUpAddsAction : public AttackAction
{
public:
    TheLurkerBelowTanksPickUpAddsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "the lurker below tanks pick up adds") {}
    bool Execute(Event event) override;
};

class TheLurkerBelowManageSpoutTimerAction : public Action
{
public:
    TheLurkerBelowManageSpoutTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "the lurker below manage spout timer") {}
    bool Execute(Event event) override;
};

// Leotheras the Blind

class LeotherasTheBlindPositionRangedAction : public MovementAction
{
public:
    LeotherasTheBlindPositionRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "leotheras the blind position ranged") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindDemonFormTankAttackBossAction : public AttackAction
{
public:
    LeotherasTheBlindDemonFormTankAttackBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "leotheras the blind demon form tank attack boss") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindMeleeTanksDontAttackDemonFormAction : public Action
{
public:
    LeotherasTheBlindMeleeTanksDontAttackDemonFormAction(PlayerbotAI* botAI)
        : Action(botAI, "leotheras the blind melee tanks don't attack demon form") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindRunAwayFromWhirlwindAction : public MovementAction
{
public:
    LeotherasTheBlindRunAwayFromWhirlwindAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "leotheras the blind run away from whirlwind") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindMeleeDpsRunAwayFromBossAction : public MovementAction
{
public:
    LeotherasTheBlindMeleeDpsRunAwayFromBossAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "leotheras the blind melee dps run away from boss") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindDestroyInnerDemonAction : public AttackAction
{
public:
    LeotherasTheBlindDestroyInnerDemonAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "leotheras the blind destroy inner demon") {}
    bool Execute(Event event) override;

private:
    bool HandleFeralTankStrategy(Unit* innerDemon);
    bool HandleHealerStrategy(Unit* innerDemon);
};

class LeotherasTheBlindFinalPhaseAssignDpsPriorityAction : public AttackAction
{
public:
    LeotherasTheBlindFinalPhaseAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "leotheras the blind final phase assign dps priority") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindMisdirectBossToDemonFormTankAction : public AttackAction
{
public:
    LeotherasTheBlindMisdirectBossToDemonFormTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "leotheras the blind misdirect boss to demon form tank") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindManageDpsWaitTimersAction : public Action
{
public:
    LeotherasTheBlindManageDpsWaitTimersAction(PlayerbotAI* botAI)
        : Action(botAI, "leotheras the blind manage dps wait timers") {}
    bool Execute(Event event) override;
};

// Fathom-Lord Karathress

class FathomLordKarathressMainTankPositionBossAction : public AttackAction
{
public:
    FathomLordKarathressMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fathom-lord karathress main tank position boss") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressFirstAssistTankPositionCaribdisAction : public AttackAction
{
public:
    FathomLordKarathressFirstAssistTankPositionCaribdisAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fathom-lord karathress first assist tank position caribdis") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressSecondAssistTankPositionSharkkisAction : public AttackAction
{
public:
    FathomLordKarathressSecondAssistTankPositionSharkkisAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fathom-lord karathress second assist tank position sharkkis") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressThirdAssistTankPositionTidalvessAction : public AttackAction
{
public:
    FathomLordKarathressThirdAssistTankPositionTidalvessAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fathom-lord karathress third assist tank position tidalvess") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressPositionCaribdisTankHealerAction : public MovementAction
{
public:
    FathomLordKarathressPositionCaribdisTankHealerAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "fathom-lord karathress position caribdis tank healer") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressMisdirectBossesToTanksAction : public AttackAction
{
public:
    FathomLordKarathressMisdirectBossesToTanksAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fathom-lord karathress misdirect bosses to tanks") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressAssignDpsPriorityAction : public AttackAction
{
public:
    FathomLordKarathressAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fathom-lord karathress assign dps priority") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressManageDpsTimerAction : public Action
{
public:
    FathomLordKarathressManageDpsTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "fathom-lord karathress manage dps timer") {}
    bool Execute(Event event) override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerMisdirectBossToMainTankAction : public AttackAction
{
public:
    MorogrimTidewalkerMisdirectBossToMainTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "morogrim tidewalker misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class MorogrimTidewalkerMoveBossToTankPositionAction : public AttackAction
{
public:
    MorogrimTidewalkerMoveBossToTankPositionAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "morogrim tidewalker move boss to tank position") {}
    bool Execute(Event event) override;

private:
    bool MoveToPhase1TankPosition();
    bool MoveToPhase2TankPosition();
};

class MorogrimTidewalkerPhase2RepositionRangedAction : public MovementAction
{
public:
    MorogrimTidewalkerPhase2RepositionRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "morogrim tidewalker phase 2 reposition ranged") {}
    bool Execute(Event event) override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjMainTankPositionBossAction : public AttackAction
{
public:
    LadyVashjMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "lady vashj main tank position boss") {}
    bool Execute(Event event) override;
};

class LadyVashjPhase1SpreadRangedInArcAction : public MovementAction
{
public:
    LadyVashjPhase1SpreadRangedInArcAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj phase 1 spread ranged in arc") {}
    bool Execute(Event event) override;
};

class LadyVashjSetGroundingTotemInMainTankGroupAction : public MovementAction
{
public:
    LadyVashjSetGroundingTotemInMainTankGroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj set grounding totem in main tank group") {}
    bool Execute(Event event) override;
};

class LadyVashjStaticChargeMoveAwayFromGroupAction : public MovementAction
{
public:
    LadyVashjStaticChargeMoveAwayFromGroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj static charge move away from group") {}
    bool Execute(Event event) override;
};

class LadyVashjMisdirectBossToMainTankAction : public AttackAction
{
public:
    LadyVashjMisdirectBossToMainTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "lady vashj misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class LadyVashjAssignPhase2AndPhase3DpsPriorityAction : public AttackAction
{
public:
    LadyVashjAssignPhase2AndPhase3DpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "lady vashj assign phase 2 and phase 3 dps priority") {}
    bool Execute(Event event) override;
};

class LadyVashjMisdirectStriderToFirstAssistTankAction : public AttackAction
{
public:
    LadyVashjMisdirectStriderToFirstAssistTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "lady vashj misdirect strider to first assist tank") {}
    bool Execute(Event event) override;
};

class LadyVashjTankAttackAndMoveAwayStriderAction : public AttackAction
{
public:
    LadyVashjTankAttackAndMoveAwayStriderAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "lady vashj tank attack and move away strider") {}
    bool Execute(Event event) override;
};

class LadyVashjTeleportToTaintedElementalAction : public AttackAction
{
public:
    LadyVashjTeleportToTaintedElementalAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "lady vashj teleport to tainted elemental") {}
    bool Execute(Event event) override;
};

class LadyVashjLootTaintedCoreAction : public MovementAction
{
public:
    LadyVashjLootTaintedCoreAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj loot tainted core") {}
    bool Execute(Event event) override;
};

class LadyVashjPassTheTaintedCoreAction : public MovementAction
{
public:
    LadyVashjPassTheTaintedCoreAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj pass the tainted core") {}
    bool Execute(Event event) override;

private:
    bool LineUpFirstCorePasser(Player* designatedLooter);
    bool LineUpSecondCorePasser(Player* firstCorePasser, Unit* closestTrigger);
    bool LineUpThirdCorePasser(
        Player* designatedLooter, Player* firstCorePasser,
        Player* secondCorePasser, Unit* closestTrigger);
    bool LineUpFourthCorePasser(
        Player* firstCorePasser, Player* secondCorePasser,
        Player* thirdCorePasser, Unit* closestTrigger);
    bool IsFirstCorePasserInPosition(Player* firstCorePasser);
    bool IsSecondCorePasserInPosition(Player* secondCorePasser);
    bool IsThirdCorePasserInPosition(Player* thirdCorePasser);
    bool IsFourthCorePasserInPosition(Player* fourthCorePasser);
    bool UseCoreOnNearestGenerator(const uint32 instanceId);
};

class LadyVashjAvoidToxicSporesAction : public MovementAction
{
public:
    LadyVashjAvoidToxicSporesAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj avoid toxic spores") {}
    bool Execute(Event event) override;
    static std::vector<Unit*> GetAllSporeDropTriggers(Player* bot);

private:
    Position FindSafestNearbyPosition(
        std::vector<Unit*> const& spores, Position const& position,
        float maxRadius, float hazardRadius);
    bool IsPathSafeFromSpores(
        Position const& start, Position const& end,
        std::vector<Unit*> const& spores, float hazardRadius);
};

class LadyVashjUseFreeActionAbilitiesAction : public Action
{
public:
    LadyVashjUseFreeActionAbilitiesAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj use free action abilities") {}
    bool Execute(Event event) override;
};

#endif
