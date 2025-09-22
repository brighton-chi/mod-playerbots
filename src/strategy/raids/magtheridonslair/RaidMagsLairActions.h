#ifndef _PLAYERBOT_RAIDMAGSLAIRACTIONS_H
#define _PLAYERBOT_RAIDMAGSLAIRACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class MagtheridonHellfireChannelerSouthTankAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerSouthTankAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler south tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerWestTankAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerWestTankAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler west tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerEastTankAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerEastTankAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler east tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerSouthWarlockAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerSouthWarlockAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler south warlock") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerWestWarlockAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerWestWarlockAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler west warlock") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerEastWarlockAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerEastWarlockAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler east warlock") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerWestHealerAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerWestHealerAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler west healer") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerEastHealerAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerEastHealerAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler east healer") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerWestHunterAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerWestHunterAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler west hunter") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerEastHunterAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerEastHunterAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler east hunter") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerDPSPriorityAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerDPSPriorityAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler dps priority") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonPositionBossAction : public MovementAction
{
public:
    MagtheridonPositionBossAction(PlayerbotAI* botAI, std::string const name = "magtheridon position boss") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonSpreadRangedAction : public MovementAction
{
public:
    MagtheridonSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "magtheridon spread ranged") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonManticronCubeClickerPositionAction : public MovementAction
{
public:
    MagtheridonManticronCubeClickerPositionAction(PlayerbotAI* botAI, std::string const name = "magtheridon manticron cube clicker position") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonUseManticronCubeAction : public MovementAction
{
public:
    MagtheridonUseManticronCubeAction(PlayerbotAI* botAI, std::string const name = "magtheridon use manticron cube") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

#endif
