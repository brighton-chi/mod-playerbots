#ifndef RAID_GRUULSLAIR_HELPERS_H
#define RAID_GRUULSLAIR_HELPERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"

enum GruulsLairSpells
{
	// Hunter
	SPELL_MISDIRECTION = 34477,
	SPELL_AIMED_SHOT = 27065, // Rank 7
	SPELL_STEADY_SHOT = 34120, // Rank 2
	SPELL_ARCANE_SHOT = 27019, // Rank 9

	// Mage
	SPELL_SPELLSTEAL = 30449,
	SPELL_FIRE_WARD = 27128, // Rank 6

	// Warlock
	SPELL_SUBJUGATE_DEMON = 11726, // Rank 3

	// High King Maulgar
	SPELL_WHIRLWIND = 33238,

	// Krosh Firehand
	SPELL_SPELL_SHIELD = 33054,

	// Olm the Summoner
	SPELL_DARK_DECAY = 33129,

	// Wild Fel Stalker
	SPELL_DETERMINATION = 33091,
	SPELL_THREATEN = 33096,
	SPELL_WILD_BITE = 33086,
};

enum GruulsLairNPCs
{
	NPC_WILD_FEL_STALKER = 18847,
};

constexpr uint8_t RTI_STAR    = 0;
constexpr uint8_t RTI_CIRCLE  = 1;
constexpr uint8_t RTI_DIAMOND = 2;
constexpr uint8_t RTI_TRIANGLE= 3;
constexpr uint8_t RTI_MOON    = 4;
constexpr uint8_t RTI_SQUARE  = 5;
constexpr uint8_t RTI_CROSS   = 6;
constexpr uint8_t RTI_SKULL   = 7;

bool IsFirstOffTank(PlayerbotAI* botAI, Player* bot);
bool IsBoomkinTank(PlayerbotAI* botAI, Player* bot);
bool IsMageTank(PlayerbotAI* botAI, Player* bot);
bool IsWarlockTank(PlayerbotAI* botAI, Player* bot);

// Gruul's Lair ogre tanking spots (X, Y, Z)
struct TankSpot 
{
	float x, y, z;
	float orientation; // in radians
};

namespace GruulsLairTankSpots 
{
	// static const TankSpot Krosh    = { 142.614f, 173.980f, 10.986f, 0.000f };
	static const TankSpot Maulgar  = { 89.492f, 167.161f, -13.109f, 3.263f };
	static const TankSpot Olm      = { 100.590f, 167.417f, -13.150f, 2.759f };
	static const TankSpot Blindeye = { 102.835f, 189.854f, -11.117f, 3.015f };
	// static const TankSpot Kiggler  = { 130.439f, 201.504f, -10.115f, 0.000f };
}

#endif
