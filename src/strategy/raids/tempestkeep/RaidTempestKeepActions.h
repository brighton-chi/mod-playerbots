#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H

#include "RaidTempestKeepHelpers.h"
#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

using namespace TempestKeepHelpers;
using namespace TempestKeepLocations;

class AlarLogDebugInfoAction : public Action
{
public:
    AlarLogDebugInfoAction(PlayerbotAI* botAI, std::string const name = "alar log debug info") : Action(botAI, name) {} 

    bool Execute(Event event) override;
};

class AlarMisdirectBossToMainTankAction : public AttackAction
{
public:
    AlarMisdirectBossToMainTankAction(PlayerbotAI* botAI, std::string const name = "alar misdirect boss to main tank") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AlarPhase1PositionBossTanksAction : public AttackAction
{
public:
    AlarPhase1PositionBossTanksAction(PlayerbotAI* botAI, std::string const name = "alar phase 1 position boss tanks") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AlarPhase1MeleeDpsFocusOnBossAction : public AttackAction
{
public:
    AlarPhase1MeleeDpsFocusOnBossAction(PlayerbotAI* botAI, std::string const name = "alar phase 1 melee dps focus on boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AlarPhase1RangedDpsPrioritizeAddsAction : public AttackAction
{
public:
    AlarPhase1RangedDpsPrioritizeAddsAction(PlayerbotAI* botAI, std::string const name = "alar phase 1 ranged dps prioritize adds") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};
class AlarPhase1PositionHealerAction : public MovementAction
{
public:
    AlarPhase1PositionHealerAction(PlayerbotAI* botAI, std::string const name = "alar phase 1 position healer") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AlarPhase1AddTankPickUpAddsAction : public AttackAction
{
public:
    AlarPhase1AddTankPickUpAddsAction(PlayerbotAI* botAI, std::string const name = "alar phase 1 add tank pick up adds") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AlarJumpFromPlatformAction : public MovementAction
{
public:
    AlarJumpFromPlatformAction(PlayerbotAI* botAI, std::string const name = "alar jump from platform") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AlarMoveAwayFromRebirthAction : public MovementAction
{
public:
    AlarMoveAwayFromRebirthAction(PlayerbotAI* botAI, std::string const name = "alar move away from rebirth") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AlarManageTimersAndTrackersAction : public Action
{
public:
    AlarManageTimersAndTrackersAction(PlayerbotAI* botAI, std::string const name = "alar manage timers and trackers") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class VoidReaverPositionBossAction : public AttackAction
{
public:
    VoidReaverPositionBossAction(PlayerbotAI* botAI, std::string const name = "void reaver position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class VoidReaverSpreadRangedAction : public MovementAction
{
public:
    VoidReaverSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "void reaver spread ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    Position GetRangedBotPosition(const TempestKeepHelpers::Location& center, float radius, uint8 botsPerRing, float offsetArc, uint8 botIndex, float botZ);
};

class HighAstromancerSolarianStackBotsAction : public MovementAction
{
public:
    HighAstromancerSolarianStackBotsAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian stack bots") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HighAstromancerSolarianMoveAwayFromGroupAction : public MovementAction
{
public:
    HighAstromancerSolarianMoveAwayFromGroupAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian move away from group") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HighAstromancerSolarianTargetSolariumPriestsAction : public AttackAction
{
public:
    HighAstromancerSolarianTargetSolariumPriestsAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian target solarium priests") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    std::vector<Unit*> GetSolariumPriests();
    std::vector<Player*> GetMeleeBots(Group* group);
    Unit* AssignSolariumPriestsToBots(const std::vector<Unit*>& solariumPriests, const std::vector<Player*>& meleeMembers);
};

class HighAstromancerSolarianTankVoidwalkerAction : public AttackAction
{
public:
    HighAstromancerSolarianTankVoidwalkerAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian tank voidwalker") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HighAstromancerSolarianCastFearWardOnMainTankAction : public Action
{
public:
    HighAstromancerSolarianCastFearWardOnMainTankAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian cast fear ward on main tank") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
