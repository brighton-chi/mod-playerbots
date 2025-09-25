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

/*class MagtheridonHellfireChannelerSouthWarlockAction : public AttackAction
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
};*/

class MagtheridonHellfireChannelerMisdirectionAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerMisdirectionAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler misdirection") : AttackAction(botAI, name) {};

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

class MagtheridonCCBurningAbyssalAction : public AttackAction
{
public:
    MagtheridonCCBurningAbyssalAction(PlayerbotAI* botAI, std::string const name = "magtheridon cc burning abyssal") : AttackAction(botAI, name) {};

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

class MagtheridonSpreadHealerAction : public MovementAction
{
public:
    MagtheridonSpreadHealerAction(PlayerbotAI* botAI, std::string const name = "magtheridon spread healer") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonUseManticronCubeAction : public MovementAction
{
public:
    MagtheridonUseManticronCubeAction(PlayerbotAI* botAI, std::string const name = "magtheridon use manticron cube") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

#endif
