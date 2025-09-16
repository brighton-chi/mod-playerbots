#ifndef RAID_GRUULSLAIR_HELPERS_H
#define RAID_GRUULSLAIR_HELPERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

enum GruulsLairSpells
{
	// High King Maulgar
	SPELL_AURA_WHIRLWIND = 33238,

	// Krosh Firehand
	SPELL_AURA_SPELL_SHIELD = 33054,

	// Olm the Summoner
	// SPELL_AURA_DARK_DECAY = 33129,

	// Wild Fel Stalker
	// SPELL_DETERMINATION = 33091,
	SPELL_THREATEN = 33096,
	SPELL_WILD_BITE = 33086,

	// Hunter
	// SPELL_AURA_MISDIRECTION = 35079,

	// Mage
	SPELL_SPELLSTEAL = 30449,

	// Warlock
	SPELL_ENSLAVE_DEMON = 11726, // Rank 3

	// Gruul the Dragonkiller
	SPELL_AURA_GROUND_SLAM_1 = 33525,
	SPELL_AURA_GROUND_SLAM_2 = 39187,
};

// Define constants for RTI indexes
inline constexpr int8 squareIcon = RtiTargetValue::squareIndex;
inline constexpr int8 starIcon = RtiTargetValue::starIndex;
inline constexpr int8 circleIcon = RtiTargetValue::circleIndex;
inline constexpr int8 diamondIcon = RtiTargetValue::diamondIndex;
inline constexpr int8 triangleIcon = RtiTargetValue::triangleIndex;

bool IsFirstTank(PlayerbotAI* botAI, Player* bot);
bool IsSecondTank(PlayerbotAI* botAI, Player* bot);
bool IsThirdTank(PlayerbotAI* botAI, Player* bot);
bool IsMageTank(PlayerbotAI* botAI, Player* bot);
bool IsMoonkinTank(PlayerbotAI* botAI, Player* bot);
bool IsPositionSafe(PlayerbotAI* botAI, Unit* bot, Position pos);
Position FindSafePosition(PlayerbotAI* botAI, Unit* bot, Unit* target, float optimalDistance);

// Gruul's Lair ogre tanking spots (X, Y, Z)
struct TankSpot 
{
	float x, y, z;
	float orientation; // in radians
};

namespace GruulsLairTankSpots 
{
	static const TankSpot Maulgar  = { 90.686f, 167.047f, -13.234f, 3.009f };
	static const TankSpot Olm      = { 99.392f, 192.834f, -10.883f, 6.265f };
	static const TankSpot Blindeye = { 100.728f, 206.389f, -10.514f, 6.218f };
}

#endif
