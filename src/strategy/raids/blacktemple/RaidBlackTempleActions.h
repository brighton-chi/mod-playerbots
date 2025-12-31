#ifndef _PLAYERBOT_RAIDBLACKTEMPLEACTIONS_H
#define _PLAYERBOT_RAIDBLACKTEMPLEACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// High Warlord Naj'entus

class HighWarlordNajentusMisdirectBossToMainTankAction : public AttackAction
{
public:
    HighWarlordNajentusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "high warlord naj'entus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusMainTankPositionBossAction : public AttackAction
{
public:
    HighWarlordNajentusMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "high warlord naj'entus main tank position boss") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusDisperseRangedAction : public MovementAction
{
public:
    HighWarlordNajentusDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "naj'entus disperse ranged") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusRemoveImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusRemoveImpalingSpineAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "high warlord naj'entus remove impaling spine") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusThrowImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusThrowImpalingSpineAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "high warlord naj'entus throw impaling spine") {}
    bool Execute(Event event) override;
};

// Supremus

class SupremusMisdirectBossToMainTankAction : public AttackAction
{
public:
    SupremusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "supremus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class SupremusDisperseRangedAction : public MovementAction
{
public:
    SupremusDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus disperse ranged") {}
    bool Execute(Event event) override;
};

class SupremusKiteBossAction : public MovementAction
{
public:
    SupremusKiteBossAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus kite boss") {}
    bool Execute(Event event) override;
};

class SupremusMoveAwayFromVolcanosAction : public MovementAction
{
public:
    SupremusMoveAwayFromVolcanosAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus move away from volcanos") {}
    bool Execute(Event event) override;

private:
    Position FindSafestNearbyPosition(
        const std::vector<Unit*>& volcanos, float maxRadius, float hazardRadius);
    bool IsPathSafeFromVolcanos(const Position& start,
        const Position& end, const std::vector<Unit*>& volcanos, float hazardRadius);
    std::vector<Unit*> GetAllSupremusVolcanos(PlayerbotAI* botAI, Player* bot);
};

class SupremusManagePhaseTimerAction : public Action
{
public:
    SupremusManagePhaseTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "supremus manage phase timer") {}
    bool Execute(Event event) override;
};

// Shade of Akama
// N/A

// Teron Gorefiend
class TeronGorefiendMisdirectBossToMainTankAction : public AttackAction
{
public:
    TeronGorefiendMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "teron gorefiend misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class TeronGorefiendMainTankPositionBossAction : public AttackAction
{
public:
    TeronGorefiendMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "teron gorefiend main tank position boss") {}
    bool Execute(Event event) override;
};

class TeronGorefiendPositionRangedOnBalconyAction : public MovementAction
{
public:
    TeronGorefiendPositionRangedOnBalconyAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "teron gorefiend position ranged on balcony") {}
    bool Execute(Event event) override;
};

class TeronGorefiendAvoidShadowOfDeathAction : public Action
{
public:
    TeronGorefiendAvoidShadowOfDeathAction(
        PlayerbotAI* botAI) : Action(botAI, "teron gorefiend avoid shadow of death") {}
    bool Execute(Event event) override;
};

class TeronGorefiendMoveToCornerToDieAction : public MovementAction
{
public:
    TeronGorefiendMoveToCornerToDieAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "teron gorefiend move to corner to die") {}
    bool Execute(Event event) override;
};

class TeronGorefiendControlAndDestroyShadowyConstructsAction : public MovementAction
{
public:
    TeronGorefiendControlAndDestroyShadowyConstructsAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "teron gorefiend control and destroy shadowy constructs") {}
    bool Execute(Event event) override;
};

// Gurtogg Bloodboil

// Reliquary of Souls

class ReliquaryOfSoulsSpellstealRuneShieldAction : public Action
{
public:
    ReliquaryOfSoulsSpellstealRuneShieldAction(
        PlayerbotAI* botAI) : Action(botAI, "reliquary of souls spellsteal rune shield") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsSpellReflectDeadenAction : public Action
{
public:
    ReliquaryOfSoulsSpellReflectDeadenAction(
        PlayerbotAI* botAI) : Action(botAI, "reliquary of souls spell reflect deaden") {}
    bool Execute(Event event) override;
};

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>

#endif
