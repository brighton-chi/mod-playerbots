#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H

#include "RaidTempestKeepHelpers.h"
#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

using namespace TempestKeepHelpers;
using namespace TempestKeepPositions;

class CrimsonHandCenturionCastPolymorphAction : public Action
{
public:
    CrimsonHandCenturionCastPolymorphAction(PlayerbotAI* botAI, std::string const name = "crimson hand centurion cast polymorph") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarMisdirectBossToMainTankAction : public AttackAction
{
public:
    AlarMisdirectBossToMainTankAction(PlayerbotAI* botAI, std::string const name = "alar misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarBossTanksMoveBetweenPlatformsAction : public AttackAction
{
public:
    AlarBossTanksMoveBetweenPlatformsAction(PlayerbotAI* botAI, std::string const name = "alar boss tanks move between platforms") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool PositionMainTank(Player* mainTank, Unit* alar, int8 alarPlatform, const std::vector<Position>& platforms);
    bool PositionAssistTank(Player* assistTank, Unit* alar, int8 alarPlatform, const std::vector<Position>& platforms);
};

class AlarMeleeDpsPrioritizeAddsAction : public AttackAction
{
public:
    AlarMeleeDpsPrioritizeAddsAction(PlayerbotAI* botAI, std::string const name = "alar melee dps prioritize adds") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarRangedDpsPrioritizeBossAction : public AttackAction
{
public:
    AlarRangedDpsPrioritizeBossAction(PlayerbotAI* botAI, std::string const name = "alar ranged dps prioritize boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarPositionHealerAction : public MovementAction
{
public:
    AlarPositionHealerAction(PlayerbotAI* botAI, std::string const name = "alar position healer") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarAddTankPickUpEmbersAction : public AttackAction
{
public:
    AlarAddTankPickUpEmbersAction(PlayerbotAI* botAI, std::string const name = "alar add tank pick up embers") : AttackAction(botAI, name) {}
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

class AlarSwapTanksOnBossAction : public AttackAction
{
public:
    AlarSwapTanksOnBossAction(PlayerbotAI* botAI, std::string const name = "alar swap tanks on boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarReturnToRoomCenterAction : public MovementAction
{
public:
    AlarReturnToRoomCenterAction(PlayerbotAI* botAI, std::string const name = "alar return to room center") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarDiveBombSpreadAction : public MovementAction
{
public:
    AlarDiveBombSpreadAction(PlayerbotAI* botAI, std::string const name = "alar dive bomb spread") : MovementAction(botAI, name) {}
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
    Position GetRangedBotPosition(const Position& center, float radius, uint8 botsPerRing, float offsetArc, uint8 botIndex, float botZ);
};

class VoidReaverUseAggroDumpAbilityAction : public Action
{
public:
    VoidReaverUseAggroDumpAbilityAction(PlayerbotAI* botAI, std::string const name = "void reaver use aggro dump ability") : Action(botAI, name) {}
    bool Execute(Event event) override;
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

class KaelthasSunstriderLogForTestingAction : public Action
{
public:
    KaelthasSunstriderLogForTestingAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider log for testing") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderKiteThaladredAction : public MovementAction
{
public:
    KaelthasSunstriderKiteThaladredAction(PlayerbotAI* botAI) : MovementAction(botAI, "kaelthas sunstrider kite thaladred") {}
    bool Execute(Event event) override;

private:
    Position GetTargetPosition(uint8 relayPhase);
};

class KaelthasSunstriderMainTankPositionSanguinarAction : public AttackAction
{
public:
    KaelthasSunstriderMainTankPositionSanguinarAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider main tank position sanguinar") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderCastFearWardOnSanguinarTankAction : public Action
{
public:
    KaelthasSunstriderCastFearWardOnSanguinarTankAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider cast fear ward on sanguinar tank") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderWarlockTankPositionCapernianAction : public AttackAction
{
public:
    KaelthasSunstriderWarlockTankPositionCapernianAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider warlock tank position capernian") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMoveAwayFromCapernianAction : public MovementAction
{
public:
    KaelthasSunstriderMoveAwayFromCapernianAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider move away from capernian") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderFirstAssistTankPositionTelonicusAction : public AttackAction
{
public:
    KaelthasSunstriderFirstAssistTankPositionTelonicusAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider first assist tank position telonicus") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMisdirectAdvisorsToTanksAction : public AttackAction
{
public:
    KaelthasSunstriderMisdirectAdvisorsToTanksAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider misdirect advisors to tanks") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderManageAdvisorDpsTimerAction : public Action
{
public:
    KaelthasSunstriderManageAdvisorDpsTimerAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider manage advisor dps timer") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderGroupUpLegendaryWeaponsAction : public MovementAction
{
public:
    KaelthasSunstriderGroupUpLegendaryWeaponsAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider group up legendary weapons") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderMoveDevastationAwayAction : public AttackAction
{
public:
    KaelthasSunstriderMoveDevastationAwayAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider move devastation away") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderHunterTurnAwayNetherstrandLongbowAction : public AttackAction
{
public:
    KaelthasSunstriderHunterTurnAwayNetherstrandLongbowAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider hunter turn away netherstrand longbow") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderLootLegendaryWeaponsAction : public Action
{
public:
    KaelthasSunstriderLootLegendaryWeaponsAction(PlayerbotAI* botAI) : Action(botAI, "kaelthas sunstrider loot legendary weapons") {}
    bool Execute(Event event) override;

private:
    bool ShouldBotLootWeapon(uint32 weaponEntry);
    bool LootWeapon(uint32 weaponEntry, uint32 itemId, const char* weaponName);
};

class KaelthasSunstriderUseLegendaryWeaponsAction : public Action
{
public:
    KaelthasSunstriderUseLegendaryWeaponsAction(PlayerbotAI* botAI) : Action(botAI, "kaelthas sunstrider use legendary weapons") {}
    bool Execute(Event event) override;

private:
    bool UsePhaseshiftBulwark();
    bool UseStaffOfDisintegration();
    bool UseNetherstrandLongbow();
    bool UseEquippedItemWithPacket(Item* item);
};

class KaelthasSunstriderPhase3AssignDpsPriorityAction : public AttackAction
{
public:
    KaelthasSunstriderPhase3AssignDpsPriorityAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider phase 3 assign dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderCheatToTestAction : public Action
{
public:
    KaelthasSunstriderCheatToTestAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider cheat to test") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
