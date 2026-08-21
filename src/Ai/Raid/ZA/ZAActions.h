/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAACTIONS_H
#define PLAYERBOTS_ZAACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include <string>

// General

class ZulamanMisdirectBossToMainTankAction : public Action
{
public:
    ZulamanMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : Action(botAI, name), _bossName(bossName) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
};

// Trash

class AmanishiMedicineManMarkWardAction : public Action
{
public:
    AmanishiMedicineManMarkWardAction(PlayerbotAI* botAI)
        : Action(botAI, "amani'shi medicine man mark ward") {}
    bool Execute(Event event) override;
};

// Akil'zon <Eagle Avatar>

class AkilzonTanksPositionBossAction : public AttackAction
{
public:
    AkilzonTanksPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "akil'zon tanks position boss") {}
    bool Execute(Event event) override;
};

class AkilzonSpreadRangedAction : public MovementAction
{
public:
    AkilzonSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "akil'zon spread ranged") {}
    bool Execute(Event event) override;
};

class AkilzonMoveToEyeOfTheStormAction : public MovementAction
{
public:
    AkilzonMoveToEyeOfTheStormAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "akil'zon move to eye of the storm") {}
    bool Execute(Event event) override;
};

class AkilzonManageElectricalStormTimerAction : public Action
{
public:
    AkilzonManageElectricalStormTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "akil'zon manage electrical storm timer") {}
    bool Execute(Event event) override;
};

// Nalorakk <Bear Avatar>

class NalorakkTanksPositionBossAction : public AttackAction
{
public:
    NalorakkTanksPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "nalorakk tanks position boss") {}
    bool Execute(Event event) override;

private:
    bool MainTankPositionTrollForm(Unit* nalorakk);
    bool FirstAssistTankPositionBearForm(Unit* nalorakk);
};

class NalorakkSpreadRangedAction : public MovementAction
{
public:
    NalorakkSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "nalorakk spread ranged") {}
    bool Execute(Event event) override;
};

// Jan'alai <Dragonhawk Avatar>

class JanalaiTanksPositionBossAction : public AttackAction
{
public:
    JanalaiTanksPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "jan'alai tanks position boss") {}
    bool Execute(Event event) override;
};

class JanalaiSpreadRangedInCircleAction : public MovementAction
{
public:
    JanalaiSpreadRangedInCircleAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "jan'alai spread ranged in circle") {}
    bool Execute(Event event) override;
};

class JanalaiAvoidFireBombsAction : public MovementAction
{
public:
    JanalaiAvoidFireBombsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "jan'alai avoid fire bombs") {}
    bool Execute(Event event) override;
};

class JanalaiMarkAmanishiHatchersAction : public Action
{
public:
    JanalaiMarkAmanishiHatchersAction(PlayerbotAI* botAI)
        : Action(botAI, "jan'alai mark amani'shi hatchers") {}
    bool Execute(Event event) override;
};

// Halazzi <Lynx Avatar>

class HalazziMainTankPositionBossAction : public AttackAction
{
public:
    HalazziMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "halazzi main tank position boss") {}
    bool Execute(Event event) override;
};

class HalazziFirstAssistTankAttackSpiritLynxAction : public AttackAction
{
public:
    HalazziFirstAssistTankAttackSpiritLynxAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "halazzi first assist tank attack spirit lynx") {}
    bool Execute(Event event) override;
};

class HalazziAssignDpsPriorityAction : public AttackAction
{
public:
    HalazziAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "halazzi assign dps priority") {}
    bool Execute(Event event) override;
};

// Hex Lord Malacrass

class HexLordMalacrassAssignDpsPriorityAction : public AttackAction
{
public:
    HexLordMalacrassAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI,  "hex lord malacrass assign dps priority") {}
    bool Execute(Event event) override;
};

class HexLordMalacrassRunAwayFromWhirlwindAction : public MovementAction
{
public:
    HexLordMalacrassRunAwayFromWhirlwindAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "hex lord malacrass run away from whirlwind") {}
    bool Execute(Event event) override;
};

class HexLordMalacrassCastersStopAttackingAction : public Action
{
public:
    HexLordMalacrassCastersStopAttackingAction(PlayerbotAI* botAI)
        : Action(botAI, "hex lord malacrass casters stop attacking") {}
    bool Execute(Event event) override;
};

class HexLordMalacrassMoveAwayFromFreezingTrapAction : public MovementAction
{
public:
    HexLordMalacrassMoveAwayFromFreezingTrapAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "hex lord malacrass move away from freezing trap") {}
    bool Execute(Event event) override;
};

// Zul'jin
class ZuljinTanksPositionBossAction : public AttackAction
{
public:
    ZuljinTanksPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "zul'jin tanks position boss") {}
    bool Execute(Event event) override;
};

class ZuljinRunAwayFromWhirlwindAction : public MovementAction
{
public:
    ZuljinRunAwayFromWhirlwindAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "zul'jin run away from whirlwind") {}
    bool Execute(Event event) override;
};

class ZuljinAvoidCyclonesAction : public MovementAction
{
public:
    ZuljinAvoidCyclonesAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "zul'jin avoid cyclones") {}
    bool Execute(Event event) override;
};

class ZuljinSpreadRangedAction : public MovementAction
{
public:
    ZuljinSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "zul'jin spread ranged") {}
    bool Execute(Event event) override;
};

#endif
