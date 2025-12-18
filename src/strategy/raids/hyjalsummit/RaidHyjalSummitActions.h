#ifndef _PLAYERBOT_RAIDHYJALSUMMITACTIONS_H
#define _PLAYERBOT_RAIDHYJALSUMMITACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// Rage Winterchill

class RageWinterchillMisdirectBossToMainTankAction : public AttackAction
{
public:
    RageWinterchillMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "rage winterchill misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class RageWinterchillMainTankPositionBossAction : public AttackAction
{
public:
    RageWinterchillMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "rage winterchill main tank position boss") {}
    bool Execute(Event event) override;
};

class RageWinterchillSpreadRangedInCircleAction : public MovementAction
{
public:
    RageWinterchillSpreadRangedInCircleAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "rage winterchill spread ranged in circle") {}
    bool Execute(Event event) override;
};

// Anetheron

class AnetheronMisdirectBossToMainTankAction : public AttackAction
{
public:
    AnetheronMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class AnetheronMainTankPositionBossAction : public AttackAction
{
public:
    AnetheronMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron main tank position boss") {}
    bool Execute(Event event) override;
};

class AnetheronSpreadRangedInArcAction : public MovementAction
{
public:
    AnetheronSpreadRangedInArcAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "anetheron spread ranged in arc") {}
    bool Execute(Event event) override;
};

class AnetheronBringInfernalToInfernalTankAction : public MovementAction
{
public:
    AnetheronBringInfernalToInfernalTankAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "anetheron bring infernal to infernal tank") {}
    bool Execute(Event event) override;
};

class AnetheronFirstAssistTankPickUpInfernalsAction : public AttackAction
{
public:
    AnetheronFirstAssistTankPickUpInfernalsAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron first assist tank pick up infernals") {}
    bool Execute(Event event) override;
};

class AnetheronFocusAllDpsOnBossAction : public AttackAction
{
public:
    AnetheronFocusAllDpsOnBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron focus all dps on boss") {}
    bool Execute(Event event) override;
};

// Kaz'rogal

class KazrogalMisdirectBossToMainTankAction : public AttackAction
{
public:
    KazrogalMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kaz'rogal misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class KazrogalMainTankPositionBossAction : public AttackAction
{
public:
    KazrogalMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kaz'rogal main tank position boss") {}
    bool Execute(Event event) override;
};

class KazrogalAssistTanksMoveInFrontOfBossAction : public AttackAction
{
public:
    KazrogalAssistTanksMoveInFrontOfBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kaz'rogal assist tanks move in front of boss") {}
    bool Execute(Event event) override;
};

class KazrogalSpreadRangedInArcAction : public MovementAction
{
public:
    KazrogalSpreadRangedInArcAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kaz'rogal spread ranged in arc") {}
    bool Execute(Event event) override;
};

class KazrogalLowManaBotMoveFromGroupAction : public MovementAction
{
public:
    KazrogalLowManaBotMoveFromGroupAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kaz'rogal low mana bot move from group") {}
    bool Execute(Event event) override;
};

class KazrogalRemoveMarkOfKazrogalAction : public Action
{
public:
    KazrogalRemoveMarkOfKazrogalAction(
        PlayerbotAI* botAI) : Action(botAI, "kaz'rogal remove mark of kaz'rogal") {}
    bool Execute(Event event) override;
};

// Azgalor

class AzgalorMisdirectBossToMainTankAction : public AttackAction
{
public:
    AzgalorMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "azgalor misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class AzgalorMainTankPositionBossAction : public AttackAction
{
public:
    AzgalorMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "azgalor main tank position boss") {}
    bool Execute(Event event) override;
};

class AzgalorSpreadRangedInArcAction : public MovementAction
{
public:
    AzgalorSpreadRangedInArcAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "azgalor spread ranged in arc") {}
    bool Execute(Event event) override;
};

class AzgalorMoveToDoomguardTankAction : public MovementAction
{
public:
    AzgalorMoveToDoomguardTankAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "azgalor move to doomguard tank") {}
    bool Execute(Event event) override;
};

class AzgalorFirstAssistTankPositionDoomguardAction : public AttackAction
{
public:
    AzgalorFirstAssistTankPositionDoomguardAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "azgalor first assist tank position doomguard") {}
    bool Execute(Event event) override;
};

class AzgalorMeleeDpsPrioritizeDoomguardsAction : public AttackAction
{
public:
    AzgalorMeleeDpsPrioritizeDoomguardsAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "azgalor melee dps prioritize doomguards") {}
    bool Execute(Event event) override;
};

// Archimonde

class ArchimondeMisdirectBossToMainTankAction : public AttackAction
{
public:
    ArchimondeMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "archimonde misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class ArchimondeMainTankPositionBossAction : public AttackAction
{
public:
    ArchimondeMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "archimonde main tank position boss") {}
    bool Execute(Event event) override;
};

class ArchimondeCastFearWardOnMainTankAction : public Action
{
public:
    ArchimondeCastFearWardOnMainTankAction(
        PlayerbotAI* botAI) : Action(botAI, "archimonde cast fear ward on main tank") {}
    bool Execute(Event event) override;
};

class ArchimondeDisperseRangedBotsAction : public MovementAction
{
public:
    ArchimondeDisperseRangedBotsAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "archimonde disperse ranged bots") {}
    bool Execute(Event event) override;
};

class ArchimondeAvoidDoomfireAction : public MovementAction
{
public:
    ArchimondeAvoidDoomfireAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "archimonde avoid doomfire") {}
    bool Execute(Event event) override;

private:
    std::vector<Unit*> GetAllDoomfires(PlayerbotAI* botAI, Player* bot);
    bool IsPathSafeFromDoomfires(const Position& start, const Position& end,
        const std::vector<Unit*>& doomfires, float hazardRadius);
    Position FindSafestNearbyPosition(const std::vector<Unit*>& doomfires,
        const Position& archimondeCenter, float maxRadius, float hazardRadius);
};

class ArchimondeRemoveDoomfireDotAction : public Action
{
public:
    ArchimondeRemoveDoomfireDotAction(
        PlayerbotAI* botAI) : Action(botAI, "archimonde remove doomfire dot") {}
    bool Execute(Event event) override;
};

#endif
