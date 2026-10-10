/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTACTIONS_H
#define PLAYERBOTS_BTACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "BTHelpers.h"
#include "MovementActions.h"
#include "Unit.h"
#include <string>
#include <vector>

// Shared

class BlackTempleResetEncounterStatesAction : public Action
{
public:
    BlackTempleResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "black temple reset encounter states") {}
    bool Execute(Event event) override;
};

// Used for High Warlord Naj'entus, Supremus, Teron Gorefiend, Gurtogg Bloodboil and Mother
// Shahraz.
class BlackTempleMisdirectToMainTankAction : public AttackAction
{
public:
    BlackTempleMisdirectToMainTankAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : AttackAction(botAI, name), _bossName(bossName) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
};

// Used for High Warlord Naj'entus, Teron Gorefiend and Gurtogg Bloodboil, including his Fel Rage
// target. The bot the boss is on walks it to the spot. With _allTanks, every tank does while the
// boss is on any tank. A tank walks only while in melee range; a non-tank leads from anywhere.
class BlackTemplePositionBossAction : public AttackAction
{
public:
    BlackTemplePositionBossAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName,
        Position const& position, float arrivalDist = 3.0f, bool allTanks = false)
        : AttackAction(botAI, name), _bossName(bossName), _position(position),
          _arrivalDist(arrivalDist), _allTanks(allTanks) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
    Position const _position;
    float const _arrivalDist;
    bool const _allTanks;
};

// Used for Reliquary of Souls and Illidari Council.
class BlackTempleCastSpellReflectionAction : public Action
{
public:
    BlackTempleCastSpellReflectionAction(PlayerbotAI* botAI, std::string const& name)
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Trash

class FocusSisterOfPleasureAction : public Action
{
public:
    FocusSisterOfPleasureAction(PlayerbotAI* botAI) : Action(botAI, "focus sister of pleasure") {}
    bool Execute(Event event) override;
};

class ShadowmoonReaverStopWandAction : public Action
{
public:
    ShadowmoonReaverStopWandAction(PlayerbotAI* botAI)
        : Action(botAI, "shadowmoon reaver stop wand") {}
    bool Execute(Event event) override;
};

class ShadowmoonReaverControlCasterPetAction : public Action
{
public:
    ShadowmoonReaverControlCasterPetAction(PlayerbotAI* botAI)
        : Action(botAI, "shadowmoon reaver control caster pet") {}
    bool Execute(Event event) override;

private:
    bool RestoreReactState(Guardian* pet);

    ReactStates _previousReactState = REACT_DEFENSIVE;
    bool _setPassive = false;
};

// High Warlord Naj'entus

class HighWarlordNajentusDisperseRangedAction : public MovementAction
{
public:
    HighWarlordNajentusDisperseRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "high warlord naj'entus disperse ranged") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusAssignSpineRemoverAction : public Action
{
public:
    HighWarlordNajentusAssignSpineRemoverAction(PlayerbotAI* botAI)
        : Action(botAI, "high warlord naj'entus assign spine remover") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusRemoveImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusRemoveImpalingSpineAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "high warlord naj'entus remove impaling spine") {}
    bool Execute(Event event) override;

private:
    ObjectGuid _spineGuid;
    bool _usedSpine = false;
    uint32 _reactionStartTime = 0;
    uint32 _reactionDelay = 0;
};

class HighWarlordNajentusAssignSpineThrowerAction : public Action
{
public:
    HighWarlordNajentusAssignSpineThrowerAction(PlayerbotAI* botAI)
        : Action(botAI, "high warlord naj'entus assign spine thrower") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusThrowImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusThrowImpalingSpineAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "high warlord naj'entus throw impaling spine") {}
    bool Execute(Event event) override;

private:
    uint32 _throwDelay = 0;
};

// Supremus

class SupremusDisperseRangedAction : public MovementAction
{
public:
    SupremusDisperseRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "supremus disperse ranged") {}
    bool Execute(Event event) override;
};

class SupremusKiteBossAction : public MovementAction
{
public:
    SupremusKiteBossAction(PlayerbotAI* botAI) : MovementAction(botAI, "supremus kite boss") {}
    bool Execute(Event event) override;

private:
    struct KitePlan
    {
        float angle;
        float runDistance;
        float fireExitDistance;
        float turn;
        float timeUntilCaught;
    };

    float GetCollisionFreeDistance(float angle, float maxDistance);
    float SimulateChase(
        Unit* supremus, float timeRemaining, float catchDistance, float angle, float runDistance);

    float _lastAngle = 0.0f;
    uint32 _lastStepTime = 0;
};

class SupremusMoveAwayFromFireAction : public MovementAction
{
public:
    SupremusMoveAwayFromFireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "supremus move away from fire") {}
    bool Execute(Event event) override;

private:
    bool FindSafestNearbyPosition(
        std::vector<BtHelpers::SupremusHazard> const& hazards, Position& destination);
    bool CanRunStraightTo(Position const& destination);

    uint32 _lastFailedSearchTime = 0;
};

class SupremusReachAroundFireAction : public MovementAction
{
public:
    SupremusReachAroundFireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "supremus reach around fire") {}
    bool Execute(Event event) override;

private:
    int8 _lastSide = 1;
    uint32 _lastStepTime = 0;
};

// Shade of Akama

class ShadeOfAkamaMeleeDpsPrioritizeChannelersAction : public AttackAction
{
public:
    ShadeOfAkamaMeleeDpsPrioritizeChannelersAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "shade of akama melee dps prioritize channelers") {}
    bool Execute(Event event) override;
};

// Teron Gorefiend

class TeronGorefiendPositionRangedOnBalconyAction : public MovementAction
{
public:
    TeronGorefiendPositionRangedOnBalconyAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "teron gorefiend position ranged on balcony") {}
    bool Execute(Event event) override;
};

class TeronGorefiendAvoidShadowOfDeathAction : public Action
{
public:
    TeronGorefiendAvoidShadowOfDeathAction(PlayerbotAI* botAI)
        : Action(botAI, "teron gorefiend avoid shadow of death") {}
    bool Execute(Event event) override;
};

class TeronGorefiendMoveToCornerToDieAction : public MovementAction
{
public:
    TeronGorefiendMoveToCornerToDieAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "teron gorefiend move to corner to die") {}
    bool Execute(Event event) override;
};

class TeronGorefiendControlAndDestroyShadowyConstructsAction : public MovementAction
{
public:
    TeronGorefiendControlAndDestroyShadowyConstructsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "teron gorefiend control and destroy shadowy constructs") {}
    bool Execute(Event event) override;

private:
    std::vector<Unit*> GetConstructs(Unit* gorefiend) const;
    Unit* GetLeadConstruct(Unit* gorefiend, std::vector<Unit*> const& constructs) const;
    Unit* GetLanceTarget(Unit* gorefiend, std::vector<Unit*> const& constructs) const;
};

// Gurtogg Bloodboil

class GurtoggBloodboilRotateRangedGroupsAction : public MovementAction
{
public:
    GurtoggBloodboilRotateRangedGroupsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "gurtogg bloodboil rotate ranged groups") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilMeleeMoveBehindAction : public MovementAction
{
public:
    GurtoggBloodboilMeleeMoveBehindAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "gurtogg bloodboil melee move behind") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilAvoidFelRageTargetAction : public MovementAction
{
public:
    GurtoggBloodboilAvoidFelRageTargetAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "gurtogg bloodboil avoid fel rage target") {}
    bool Execute(Event event) override;
};

// Reliquary of Souls

class ReliquaryOfSoulsMisdirectToMainTankAction : public AttackAction
{
public:
    ReliquaryOfSoulsMisdirectToMainTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "reliquary of souls misdirect to main tank") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsAdjustDistanceFromSufferingAction : public MovementAction
{
public:
    ReliquaryOfSoulsAdjustDistanceFromSufferingAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "reliquary of souls adjust distance from suffering") {}
    bool Execute(Event event) override;

private:
    bool MoveToDistanceFromSuffering(Unit* suffering, float distance);
    bool RangedMoveAwayFromBoss(Unit* suffering);
};

class ReliquaryOfSoulsHealersDpsSufferingAction : public Action
{
public:
    ReliquaryOfSoulsHealersDpsSufferingAction(PlayerbotAI* botAI)
        : Action(botAI, "reliquary of souls healers dps suffering") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsSpellstealRuneShieldAction : public Action
{
public:
    ReliquaryOfSoulsSpellstealRuneShieldAction(PlayerbotAI* botAI)
        : Action(botAI, "reliquary of souls spellsteal rune shield") {}
    bool Execute(Event event) override;
};

// Mother Shahraz

class MotherShahrazTanksPositionBossUnderPillarAction : public AttackAction
{
public:
    MotherShahrazTanksPositionBossUnderPillarAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "mother shahraz tanks position boss under pillar") {}
    bool Execute(Event event) override;
};

class MotherShahrazMeleeDpsWaitAtSafePositionAction : public MovementAction
{
public:
    MotherShahrazMeleeDpsWaitAtSafePositionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "mother shahraz melee dps wait at safe position") {}
    bool Execute(Event event) override;
};

class MotherShahrazPositionRangedUnderPillarAction : public MovementAction
{
public:
    MotherShahrazPositionRangedUnderPillarAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "mother shahraz position ranged under pillar") {}
    bool Execute(Event event) override;
};

class MotherShahrazBreakFatalAttractionAction : public MovementAction
{
public:
    MotherShahrazBreakFatalAttractionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "mother shahraz break fatal attraction") {}
    bool Execute(Event event) override;

private:
    std::vector<Player*> GetAttractedPlayers();
};

// Illidari Council

class IllidariCouncilMisdirectToTanksAction : public AttackAction
{
public:
    IllidariCouncilMisdirectToTanksAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council misdirect to tanks") {}
    bool Execute(Event event) override;
};

class IllidariCouncilMainTankPositionGathiosAction : public AttackAction
{
public:
    IllidariCouncilMainTankPositionGathiosAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council main tank position gathios") {}
    bool Execute(Event event) override;
};

class IllidariCouncilInterruptCircleOfHealingAction : public Action
{
public:
    IllidariCouncilInterruptCircleOfHealingAction(PlayerbotAI* botAI)
        : Action(botAI, "illidari council interrupt circle of healing") {}
    bool Execute(Event event) override;
};

class IllidariCouncilFirstAssistTankFocusMalandeAction : public AttackAction
{
public:
    IllidariCouncilFirstAssistTankFocusMalandeAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council first assist tank focus malande") {}
    bool Execute(Event event) override;
};

class IllidariCouncilSecondAssistTankPositionVerasAction : public AttackAction
{
public:
    IllidariCouncilSecondAssistTankPositionVerasAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council second assist tank position veras") {}
    bool Execute(Event event) override;
};

class IllidariCouncilMageTankPositionZerevorAction : public AttackAction
{
public:
    IllidariCouncilMageTankPositionZerevorAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council mage tank position zerevor") {}
    bool Execute(Event event) override;
};

class IllidariCouncilPositionMageTankHealerAction : public AttackAction
{
public:
    IllidariCouncilPositionMageTankHealerAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council position mage tank healer") {}
    bool Execute(Event event) override;
};

class IllidariCouncilAssignDpsTargetsAction : public AttackAction
{
public:
    IllidariCouncilAssignDpsTargetsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council assign dps targets") {}
    bool Execute(Event event) override;

private:
    bool ShouldAttackMalande(Unit* malande, Unit* zerevor) const;
};

class IllidariCouncilDisperseRangedAction : public MovementAction
{
public:
    IllidariCouncilDisperseRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidari council disperse ranged") {}
    bool Execute(Event event) override;
};

class IllidariCouncilCommandPetTargetAction : public AttackAction
{
public:
    IllidariCouncilCommandPetTargetAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidari council command pet target") {}
    bool Execute(Event event) override;
};

class IllidariCouncilManageDpsTimerAction : public Action
{
public:
    IllidariCouncilManageDpsTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "illidari council manage dps timer") {}
    bool Execute(Event event) override;
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageMisdirectToTanksAction : public Action
{
public:
    IllidanStormrageMisdirectToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage misdirect to tanks") {}
    bool Execute(Event event) override;

private:
    bool TryMisdirectToFlameTanks(Group* group);
    bool TryMisdirectToWarlockTank(Unit* illidan);
};

class IllidanStormrageMainTankRepositionBossAction : public AttackAction
{
public:
    IllidanStormrageMainTankRepositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidan stormrage main tank reposition boss") {}
    bool Execute(Event event) override;

private:
    bool MoveToShadowTrap(Unit* illidan, GameObject* trap);
    Position FindSafestNearbyPosition(
        std::vector<Unit*> const& flameCrashes, float maxRadius, float hazardRadius);
    bool IsPathSafeFromFlameCrashes(Position const& start,
        Position const& end, std::vector<Unit*> const& flameCrashes, float hazardRadius);
};

class IllidanStormrageIsolateBotWithParasiteAction : public MovementAction
{
public:
    IllidanStormrageIsolateBotWithParasiteAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage isolate bot with parasite") {}
    bool Execute(Event event) override;

private:
    bool InfectedBotMoveFromGroup(Position const& targetPos);
    bool FreezeTrapShadowfiend(Position const& targetPos);
};

class IllidanStormrageSetEarthbindTotemAction : public Action
{
public:
    IllidanStormrageSetEarthbindTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage set earthbind totem") {}
    bool Execute(Event event) override;
};

namespace BtHelpers
{
    struct EyeBlastDangerArea;
}

class IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction : public AttackAction
{
public:
    IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidan stormrage assist tanks handle flames of azzinoth") {}
    bool Execute(Event event) override;

private:
    bool RepositionToAvoidEyeBlast(BtHelpers::EyeBlastDangerArea const& dangerArea);
    bool RepositionToAvoidBlaze(Unit* eastFlame, Unit* westFlame);
};

class IllidanStormrageControlPetAggressionAction : public Action
{
public:
    IllidanStormrageControlPetAggressionAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage control pet aggression") {}
    bool Execute(Event event) override;
};

class IllidanStormragePositionAboveGrateAction : public MovementAction
{
public:
    IllidanStormragePositionAboveGrateAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage position above grate") {}
    bool Execute(Event event) override;
};

class IllidanStormrageRemoveDarkBarrageAction : public Action
{
public:
    IllidanStormrageRemoveDarkBarrageAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage remove dark barrage") {}
    bool Execute(Event event) override;
};

class IllidanStormrageMoveAwayFromLandingPointAction : public MovementAction
{
public:
    IllidanStormrageMoveAwayFromLandingPointAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage move away from landing point") {}
    bool Execute(Event event) override;
};

class IllidanStormrageDisperseRangedAction : public MovementAction
{
public:
    IllidanStormrageDisperseRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage disperse ranged") {}
    bool Execute(Event event) override;

private:
    bool FanOutBehindInHumanPhase(Unit* illidan, Group* group);
    bool SpreadInCircleInDemonPhase(Unit* illidan, Group* group);
};

class IllidanStormrageMeleeGoSomewhereToNotDieAction : public MovementAction
{
public:
    IllidanStormrageMeleeGoSomewhereToNotDieAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage melee go somewhere to not die") {}
    bool Execute(Event event) override;
};

class IllidanStormrageWarlockTankHandleDemonBossAction : public AttackAction
{
public:
    IllidanStormrageWarlockTankHandleDemonBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidan stormrage warlock tank handle demon boss") {}
    bool Execute(Event event) override;
};

class IllidanStormrageDpsPrioritizeAddsAction : public AttackAction
{
public:
    IllidanStormrageDpsPrioritizeAddsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "illidan stormrage dps prioritize adds") {}
    bool Execute(Event event) override;
};

class IllidanStormrageUseShadowTrapAction : public MovementAction
{
public:
    IllidanStormrageUseShadowTrapAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "illidan stormrage use shadow trap") {}
    bool Execute(Event event) override;
};

class IllidanStormrageManageDpsTimerAndRtiAction : public Action
{
public:
    IllidanStormrageManageDpsTimerAndRtiAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage manage dps timer and rti") {}
    bool Execute(Event event) override;
};

class IllidanStormrageDestroyHazardsAction : public Action
{
public:
    IllidanStormrageDestroyHazardsAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage destroy hazards") {}
    bool Execute(Event event) override;
};

class IllidanStormrageHandleAddsCheatAction : public Action
{
public:
    IllidanStormrageHandleAddsCheatAction(PlayerbotAI* botAI)
        : Action(botAI, "illidan stormrage handle adds cheat") {}
    bool Execute(Event event) override;
};

#endif
