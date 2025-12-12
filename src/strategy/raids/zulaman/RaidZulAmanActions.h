#ifndef _PLAYERBOT_RAIDZULAMANACTIONS_H
#define _PLAYERBOT_RAIDZULAMANACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class AmanishiMedicineManMarkWardAction : public Action
{
public:
    AmanishiMedicineManMarkWardAction(
        PlayerbotAI* botAI, std::string const name = "amanishi medicine man mark ward") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class AkilzonMisdirectBossToMainTankAction : public AttackAction
{
public:
    AkilzonMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "akilzon misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AkilzonMainTankPositionBossAction : public AttackAction
{
public:
    AkilzonMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "akilzon main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AkilzonSpreadRangedAction : public MovementAction
{
public:
    AkilzonSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "akilzon spread ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AkilzonMoveToEyeOfTheStormAction : public MovementAction
{
public:
    AkilzonMoveToEyeOfTheStormAction(
        PlayerbotAI* botAI, std::string const name = "akilzon move to eye of the storm") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class NalorakkMisdirectBossToMainTankAction : public AttackAction
{
public:
    NalorakkMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "nalorakk misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class NalorakkTanksPositionBossAction : public AttackAction
{
public:
    NalorakkTanksPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "nalorakk tanks position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool MainTankPositionTrollForm(Player* mainTank, Unit* nalorakk);
    bool FirstAssistTankPositionBearForm(Player* assistTank, Unit* nalorakk);
};

class NalorakkSpreadRangedAction : public MovementAction
{
public:
    NalorakkSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "nalorakk spread ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class JanalaiMisdirectBossToMainTankAction : public AttackAction
{
public:
    JanalaiMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "janalai misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class JanalaiMainTankPositionBossAction : public AttackAction
{
public:
    JanalaiMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "janalai main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class JanalaiSpreadRangedInCircleAction : public MovementAction
{
public:
    JanalaiSpreadRangedInCircleAction(
        PlayerbotAI* botAI, std::string const name = "janalai spread ranged in circle") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class JanalaiMoveAwayFromFireBombsAction : public MovementAction
{
public:
    JanalaiMoveAwayFromFireBombsAction(PlayerbotAI* botAI, std::string const name = "janalai move away from fire bombs") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    Position FindSafestNearbyPosition(const std::vector<Unit*>& fireBombs, const Position& position, float maxRadius, float hazardRadius);
    bool IsPathSafeFromFireBombs(const Position& start, const Position& end, const std::vector<Unit*>& fireBombs, float hazardRadius);
    std::vector<Unit*> GetAllFireBombTriggers(PlayerbotAI* botAI, Player* bot);
};

class JanalaiMarkAmaniHatchersAction : public Action
{
public:
    JanalaiMarkAmaniHatchersAction(
        PlayerbotAI* botAI, std::string const name = "janalai mark amani hatchers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class HalazziMisdirectBossToMainTankAction : public AttackAction
{
public:
    HalazziMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "halazzi misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HalazziMainTankPositionBossAction : public AttackAction
{
public:
    HalazziMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "halazzi main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HalazziFirstAssistTankAttackSpiritLynxAction : public AttackAction
{
public:
    HalazziFirstAssistTankAttackSpiritLynxAction(
        PlayerbotAI* botAI, std::string const name = "halazzi first assist tank attack spirit lynx") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HalazziAssignDpsPriorityAction : public AttackAction
{
public:
    HalazziAssignDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "halazzi assign dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HexLordMalacrassMisdirectBossToMainTankAction : public AttackAction
{
public:
    HexLordMalacrassMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HexLordMalacrassAssignDpsPriorityAction : public AttackAction
{
public:
    HexLordMalacrassAssignDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass assign dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HexLordMalacrassPurgeBuffFromBossAction : public Action
{
public:
    HexLordMalacrassPurgeBuffFromBossAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass purge buff from boss") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class HexLordMalacrassDispelMindControlAction : public Action
{
public:
    HexLordMalacrassDispelMindControlAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass dispel mind control") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class HexLordMalacrassMainTankPositionBossAction : public AttackAction
{
public:
    HexLordMalacrassMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ZuljinMisdirectBossToMainTankAction : public AttackAction
{
public:
    ZuljinMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "zuljin misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ZuljinMainTankPositionBossAction : public AttackAction
{
public:
    ZuljinMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "zuljin main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ZuljinRunAwayFromWhirlwindAction : public MovementAction
{
public:
    ZuljinRunAwayFromWhirlwindAction(
        PlayerbotAI* botAI, std::string const name = "zuljin run away from whirlwind") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ZuljinSpreadRangedAction : public MovementAction
{
public:
    ZuljinSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "zuljin spread ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ZuljinMoveNearGroupAction : public MovementAction
{
public:
    ZuljinMoveNearGroupAction(
        PlayerbotAI* botAI, std::string const name = "zuljin move near group") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
