/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERTWINS_H
#define PLAYERBOTS_SWPENCOUNTERTWINS_H

#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include "SharedDefines.h"
#include "SWPShared.h"
#include <array>
#include <optional>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

struct EredarTwinsIncomingConflagrationState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 delayMs = 0;
    uint32 expireMs = 0;
};

// The twins whose threat cap the bot has reached; empty for a twin it may still attack.
struct EredarTwinsThreatHolds
{
    ObjectGuid sacrolash;
    ObjectGuid alythess;
};

// Alythess's tank keeps moving out of her Blazes, so a paladin, who holds threat on the move.
inline constexpr EncounterHelpers::ClassRole ALYTHESS_PALADIN_TANK_ROLE = {
    .playerClass = CLASS_PALADIN,
    .requiredSpec = PALADIN_TAB_PROTECTION,
    .role = { "class_role_alythess_paladin_tank", "Alythess paladin tank" },
    .candidate = { "class_role_class_protection_paladin", "Protection Paladin" },
};
// 60 yards covers the twins' room and the balcony above it.
inline constexpr float EREDAR_TWINS_CLASS_ROLE_ANNOUNCE_DISTANCE = 60.0f;
// 150 yards is short of the gauntlet, whose center is 210 yards from Alythess.
inline constexpr float EREDAR_TWINS_CLASS_ROLE_REARM_DISTANCE = 150.0f;

// Used to measure if a bot is on the balcony; deliberately a little below the actual balcony Z.
inline constexpr float EREDAR_TWINS_BALCONY_Z = 50.0f;
// Ranged count as on their balcony position within this distance.
inline constexpr float EREDAR_TWINS_BALCONY_ARRIVAL_DISTANCE = 1.0f;

inline constexpr uint32 EREDAR_TWINS_BLAZE_CACHE_INTERVAL_MS = 200;
// The Blaze trap GO casts 45246, dealing damage within 3y of its center.
inline constexpr float BLAZE_HAZARD_RADIUS = 3.0f;
inline constexpr float BLAZE_CLEARANCE = BLAZE_HAZARD_RADIUS + 1.5f;
inline constexpr float BLAZE_SEARCH_RADIUS = 30.0f;
// A bot's avoid-AoE flees a Blaze patch within this distance (the trap's 2.5y, the bot's 1.5y
// reach, and the object's 0.39y size), so moves to the phase 2 stack keep at least this far from
// every Alythess tank position.
inline constexpr float BLAZE_AVOID_AOE_DISTANCE = 4.4f;

// Grace period for the tanks to build threat before the rest of the raid opens fire.
inline constexpr uint32 EREDAR_TWINS_DPS_HOLD_MS = 8 * IN_MILLISECONDS;
// DPS cooldowns are held until Sacrolash is at this percent. Eredar Twins is a threat-sensitive
// fight due to Sacrolash dropping threat on tanks and Alythess targeting Conflagration based on
// Sacrolash's threat table.
inline constexpr float EREDAR_TWINS_MAX_DPS_HP_PERCENT = 80.0f;
// Don't exceed this percentage of the tank's threat. For Sacrolash, the limit is based on the
// second highest tank (unless there is only one Sacrolash tank).
inline constexpr float SACROLASH_THREAT_HOLD_RATIO = 0.8f;
inline constexpr float ALYTHESS_THREAT_HOLD_RATIO = 0.9f;
inline constexpr uint32 EREDAR_TWINS_THREAT_HOLDS_CACHE_INTERVAL_MS = 100;

// Conflagration hits players within 8y of its target, who then burn players within 8y of them.
inline constexpr float CONFLAGRATION_HAZARD_RADIUS = 8.0f;
inline constexpr float CONFLAGRATION_CLEARANCE = CONFLAGRATION_HAZARD_RADIUS + 2.0f;
// Bots wait 300ms to react to Conflagration (to make the action look less artificial).
inline constexpr uint32 CONFLAGRATION_DELAY_MS = 300;
// Conflagration is a 3.5s cast. The bot needs to hold position through the impact, so some time is
// added to account for the projectile's travel time.
inline constexpr uint32 CONFLAGRATION_WINDOW_MS = 5500;

inline constexpr uint8 FLAME_TOUCHED_PROTECT_STACKS = 5;
inline constexpr int32 FLAME_SEAR_PROTECT_WINDOW_MS = 2 * IN_MILLISECONDS;

inline Position const ALYTHESS_START_POSITION = { 1819.180f, 625.539f, 33.4038f };
// All in Alythess's melee range (4.7-5.15y from her) and within Sacrolash's Shadow Blades (20y),
// which clears the tank's Flame Touched. Visited in order; the move from the last to the first
// passes 2.56y from her center, outside the 2.375y within which she backs off her victim once
// Empower has grown her (2.0y before).
inline std::array const ALYTHESS_TANK_POSITIONS = {
    Position{ 1820.310f, 630.150f, 33.404f },
    Position{ 1815.800f, 628.800f, 33.404f },
    Position{ 1814.700f, 624.200f, 33.404f },
    Position{ 1818.100f, 621.000f, 33.404f },
    Position{ 1823.040f, 622.130f, 33.404f },
};
// The tank's Blaze patch lands where it stops, and the positions are 4.67y apart against a 4.5y
// clearance, so it must stop on the position, not short of it on the way in.
inline constexpr float ALYTHESS_TANK_ARRIVAL_DISTANCE = 0.2f;
// The Alythess tank's landing at the pull, the position nearest the balcony.
inline constexpr uint8 ALYTHESS_TANK_LANDING_INDEX = 3;

// Phase 1 positions (ranged up top, melee on Sacrolash).
// Both Sacrolash tanks stand here. Her tank backs along the line from her spawn, so she ends
// 4.5y short of it, 16.7y from Alythess, facing away: melee behind her (up to 4.75y) stay 12y
// from Alythess, out of reach of their area attacks (8y plus her 2.5y reach).
inline Position const SACROLASH_TANK_POSITION  =             { 1799.000f, 618.900f, 33.404f };
inline constexpr float SACROLASH_TANK_ARRIVAL_DISTANCE = 1.0f;
// Melee land at the pull on the line from Sacrolash's spawn to her tanks' position.
inline Position const EREDAR_TWINS_MELEE_LANDING_POSITION =  { 1807.400f, 622.800f, 33.404f };
inline Position const EREDAR_TWINS_P1_RANGED_POSITION =      { 1808.076f, 603.460f, 51.684f };
// A ranged bot that pulls aggro on Alythess waits here, out of the stack, so Blaze doesn't hit the
// ranged stack. The twins are in LoS from only ~10 yd of the balcony edge; this is its east end.
inline Position const EREDAR_TWINS_P1_RANGED_ASIDE_POSITION = { 1813.000f, 602.000f, 51.610f };
inline Position const EREDAR_TWINS_MELEE_CONFLAG_POSITION =  { 1807.400f, 635.800f, 33.404f };
inline Position const EREDAR_TWINS_RANGED_CONFLAG_POSITION = { 1801.133f, 584.456f, 50.696f };

// A bot (not the Alythess tank) holding Alythess waits here on the floor, where her Blazes land
// away from everyone.
inline Position const EREDAR_TWINS_FLOOR_ASIDE_POSITION =    { 1817.200f, 616.000f, 33.404f };
inline constexpr float EREDAR_TWINS_STEP_ASIDE_ARRIVAL_DISTANCE = 1.0f;

// Phase 2: everybody but the Alythess tank in one stack, in her melee range on her far side from
// Sacrolash, 4.8y from the nearest Alythess tank position, so a Shadow Nova on it reaches the tank.
inline Position const EREDAR_TWINS_P2_STACK_POSITION =       { 1823.900f, 626.900f, 33.404f };
inline constexpr float EREDAR_TWINS_P2_STACK_ARRIVAL_DISTANCE = 0.3f;
// When Sacrolash dies, her melee and tanks reach the stack round the end of Alythess tank
// position 0, clear of the patches.
inline std::array const EREDAR_TWINS_PHASE_CHANGE_WAYPOINTS = {
    Position{ 1816.300f, 639.800f, 33.404f },
    Position{ 1828.800f, 628.400f, 33.404f },
};
inline constexpr float EREDAR_TWINS_WAYPOINT_ARRIVAL_DISTANCE = 1.0f;
inline constexpr float EREDAR_TWINS_ROUTE_LEG_TOLERANCE = 0.5f;

Position GetAlythessTankPosition(Unit* alythess, uint8 index);
Position GetEredarTwinsP2StackPosition(Unit* alythess);
// Where a bot holding Alythess waits: on the balcony, or on the floor.
Position GetEredarTwinsStepAsidePosition(Unit* alythess, bool onBalcony);
// The phase 2 stack, or the next waypoint while the straight line to the stack crosses the
// Alythess tank positions.
Position GetEredarTwinsP2StackRouteTarget(Player* bot, Unit* alythess);
bool UpdateEredarTwinsClassRoleAnnouncement(PlayerbotAI* botAI);
Player* GetAlythessTank(Player* bot);
bool IsAlythessTank(Player* player);
// Alythess is attacking the player, who isn't her tank.
bool IsHoldingAlythessAggro(Player* player, Unit* alythess);
Player* GetSacrolashTank(Player* bot, uint8 index);
bool IsAnySacrolashTank(Player* bot);
bool ShouldHoldAlythessThreat(Player* bot, Unit* alythess);
bool ShouldHoldSacrolashThreat(Player* bot, Unit* sacrolash);
// The first EREDAR_TWINS_DPS_HOLD_MS of Sacrolash's fight, which the tanks spend taking aggro.
bool IsEredarTwinsStartHoldActive(PlayerbotAI* botAI);
// For the "eredar twins threat holds" value.
EredarTwinsThreatHolds FindEredarTwinsThreatHolds(Player* bot);
std::vector<Position> FindEredarTwinsBlazePositions(Player* bot);
bool IsAlythessTankPositionSafe(PlayerbotAI* botAI, Position const& position);
void RecordIncomingEredarTwinsConflagrationTarget(Player* target);
// Empty until the reaction delay has passed and after the window has ended.
ObjectGuid GetEredarTwinsConflagrationTargetGuid(uint32 instanceId);
bool IsEredarTwinsConflagrationTarget(Player* bot);

// Shared encounter state

struct EredarTwinsEncounterState
{
    std::optional<EredarTwinsIncomingConflagrationState> incomingConflagration;
    std::optional<uint32> dpsHoldStartMs;
};

EredarTwinsEncounterState& EredarTwinsState(uint32 instanceId);
EredarTwinsEncounterState* FindEredarTwinsState(uint32 instanceId);

}

#endif
