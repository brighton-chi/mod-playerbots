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

class AlarMeleeDpsPrioritizeBossAction : public AttackAction
{
public:
    AlarMeleeDpsPrioritizeBossAction(PlayerbotAI* botAI, std::string const name = "alar melee dps prioritize boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlarRangedDpsPrioritizeAddsAction : public AttackAction
{
public:
    AlarRangedDpsPrioritizeAddsAction(PlayerbotAI* botAI, std::string const name = "alar ranged dps prioritize adds") : AttackAction(botAI, name) {}
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

class KaelthasSunstriderManageWarlockTankStrategyAction : public Action
{
public:
    KaelthasSunstriderManageWarlockTankStrategyAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider manage warlock tank strategy") : Action(botAI, name) {}
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

class KaelthasSunstriderGroupUpLegendaryWeaponsAction : public AttackAction
{
public:
    KaelthasSunstriderGroupUpLegendaryWeaponsAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider group up legendary weapons") : AttackAction(botAI, name) {}
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

class KaelthasSunstriderLootLegendaryWeaponsAction : public MovementAction
{
public:
    KaelthasSunstriderLootLegendaryWeaponsAction(PlayerbotAI* botAI) : MovementAction(botAI, "kaelthas sunstrider loot legendary weapons") {}
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

class KaelthasSunstriderReequipGearAction : public Action
{
public:
    KaelthasSunstriderReequipGearAction(PlayerbotAI* botAI) : Action(botAI, "kaelthas reequip gear") {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderAssignAdvisorDpsPriorityAction : public AttackAction
{
public:
    KaelthasSunstriderAssignAdvisorDpsPriorityAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider assign advisor dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderAvoidFlameStrikeAction : public MovementAction
{
public:
    KaelthasSunstriderAvoidFlameStrikeAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider avoid flame strike") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    std::vector<Unit*> GetAllFlameStrikeTriggers(PlayerbotAI* botAI, Player* bot);
    Position FindSafestNearbyPosition(const std::vector<Unit*>& flameStrikes, float hazardRadius);
    bool IsPathSafeFromFlameStrikes(const Position& start, const Position& end, const std::vector<Unit*>& flameStrikes, float hazardRadius);
};

class KaelthasSunstriderRoundUpPhoenixesAndFocusDownEggsAction : public AttackAction
{
public:
    KaelthasSunstriderRoundUpPhoenixesAndFocusDownEggsAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider round up phoenixes and focus down eggs") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderBreakMindControlWithInfinityBladeAction : public AttackAction
{
public:
    KaelthasSunstriderBreakMindControlWithInfinityBladeAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider break mind control with infinity blade") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderBreakThroughShockBarrierAction : public AttackAction
{
public:
    KaelthasSunstriderBreakThroughShockBarrierAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider break through shock barrier") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KaelthasSunstriderSpreadOutInMidairAction : public MovementAction
{
public:
    KaelthasSunstriderSpreadOutInMidairAction(PlayerbotAI* botAI, std::string const name = "kaelthas sunstrider spread out in midair") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
