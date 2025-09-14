#ifndef _PLAYERBOT_RAIDGRUULSLAIRACTIONS_H
#define _PLAYERBOT_RAIDGRUULSLAIRACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class HighKingMaulgarMaulgarTankAction : public AttackAction
{
public:
    HighKingMaulgarMaulgarTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar maulgar tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarOlmTankAction : public AttackAction
{
public:
    HighKingMaulgarOlmTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar olm tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarBlindeyeTankAction : public AttackAction
{
public:
    HighKingMaulgarBlindeyeTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar blindeye tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarMageTankAction : public AttackAction
{
public:
    HighKingMaulgarMageTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar mage tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarBoomkinTankAction : public AttackAction
{
public:
    HighKingMaulgarBoomkinTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar boomkin tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarMeleeDPSAction : public AttackAction
{
public:
    HighKingMaulgarMeleeDPSAction(PlayerbotAI* botAI, std::string const name = "high king maulgar melee dps") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarRangedDPSAction : public AttackAction
{
public:
    HighKingMaulgarRangedDPSAction(PlayerbotAI* botAI, std::string const name = "high king maulgar ranged dps") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

/* class HighKingMaulgarAvoidBlastWaveAction : public MovementAction
{
public:
    HighKingMaulgarAvoidBlastWaveAction(PlayerbotAI* botAI, std::string const name = "high king maulgar avoid blast wave") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful();
};

class HighKingMaulgarControlFelstalkerAction : public AttackAction
{
public:
    HighKingMaulgarControlFelstalkerAction(PlayerbotAI* botAI, std::string const name = "high king maulgar control felstalker") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful();
};

class HighKingMaulgarAvoidWhirlwindAction : public MovementAction
{
public:
    HighKingMaulgarAvoidWhirlwindAction(PlayerbotAI* botAI, std::string const name = "high king maulgar avoid whirlwind") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful();
};

class HighKingMaulgarHunterMisdirectionAction : public AttackAction
{
public:
    HighKingMaulgarHunterMisdirectionAction(PlayerbotAI* botAI, std::string const name = "high king maulgar hunter misdirection") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
}; */

#endif
