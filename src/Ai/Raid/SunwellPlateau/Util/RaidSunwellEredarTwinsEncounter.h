/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLEREDARTWINSENCOUNTER_H
#define _PLAYERBOT_RAIDSUNWELLEREDARTWINSENCOUNTER_H

#include <array>
#include <unordered_map>

#include "ObjectGuid.h"
#include "Position.h"
#include "RaidSunwellData.h"

class Player;
class PlayerbotAI;
class Unit;

namespace SunwellHelpers
{

constexpr float EREDAR_TWINS_BALCONY_Z = 50.0f;

extern const Position SACROLASH_TANK_POSITION;
extern const std::array<Position, 5> ALYTHESS_TANK_POSITIONS;
extern const Position EREDAR_TWINS_P1_RANGED_POSITION;
extern const Position EREDAR_TWINS_P2_MELEE_STACK_POSITION;
extern const Position EREDAR_TWINS_P2_RANGED_STACK_POSITION;
extern const Position EREDAR_TWINS_RANGED_CONFLAG_POSITION;
extern const Position EREDAR_TWINS_MELEE_CONFLAG_POSITION;

extern std::unordered_map<ObjectGuid, uint8> alythessTankStep;

bool IsSacrolashTank(PlayerbotAI* botAI, Player* bot);
bool IsAlythessTank(PlayerbotAI* botAI, Player* bot);
bool ShouldHoldSacrolashThreat(PlayerbotAI* botAI, Player* bot, Unit* alythess, Unit* sacrolash);
bool IsAlythessTankPositionSafe(Player* bot, Position const& position);
bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot);
bool IsEredarTwinsConflagrationTarget(Unit* alythess, Player* bot);

}

#endif
