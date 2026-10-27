/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ENCOUNTERHELPERS_H
#define PLAYERBOTS_ENCOUNTERHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <initializer_list>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class Action;
class Player;
class PlayerbotAI;
class Unit;

namespace EncounterHelpers
{

// Cheap, rough proxies for how far along an encounter is. 95% HP means the boss and raid are
// positioned, the tank has threat, and the fight proper has started, so it's time to use cooldowns.
// 10% means the boss is almost dead, so ignore adds and finish off the boss.
inline constexpr float BOSS_ENGAGED_HEALTH_PCT = 95.0f;
inline constexpr float BOSS_BURN_HEALTH_PCT = 10.0f;

bool IsEncounterInProgress(Player* bot, uint32 mapId);
bool CanTakeStepTowards(
    Player* bot, float destinationX, float destinationY, float moveDist,
    float& stepX, float& stepY, float& stepZ);
bool GetStepToPosition(
    Player* bot, Position const& position, float arrivalDist, Unit* facing, float& stepX,
    float& stepY, bool& backwards);
bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
bool MarkTargetWithSkull(Player* bot, Unit* target);
bool MarkTargetWithSquare(Player* bot, Unit* target);
bool MarkTargetWithStar(Player* bot, Unit* target);
bool MarkTargetWithCircle(Player* bot, Unit* target);
bool MarkTargetWithDiamond(Player* bot, Unit* target);
bool MarkTargetWithTriangle(Player* bot, Unit* target);
bool MarkTargetWithCross(Player* bot, Unit* target);
bool MarkTargetWithMoon(Player* bot, Unit* target);
bool ClearTargetIcon(Player* bot, uint8 iconId);
bool ClearStrategyTargetIcon(Player* bot, uint8 iconId, std::initializer_list<uint32> entries);
void SetRtiTarget(PlayerbotAI* botAI, std::string const& rtiName);
bool IsMechanicTrackerBot(Player* bot, uint32 mapId);
Player* GetGroupMainTank(Player* bot);
Player* GetGroupAssistTank(Player* bot, uint8 index);
Player* GetNearestPlayerInRadius(Player* bot, float radius);
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId);
uint32 GetSelfImmunitySpell(Player* bot);
bool IsDpsCooldownAction(Player* bot, Action* action);
bool IsTauntAction(Player* bot, Action* action);
bool IsAoeThreatAction(Player* bot, Action* action);

// Class roles: a raid job given to one member of a class, such as a mage tank.

// In rank order, lowest first.
enum class ClassRoleSource : uint8
{
    None,
    Bot,
    PreferredSpecBot,
    Assistant,
};

struct ClassRoleHolder
{
    ObjectGuid guid;
    ClassRoleSource source = ClassRoleSource::None;

    bool operator==(ClassRoleHolder const&) const = default;
};

// A bot text name and its English default.
struct ClassRoleText
{
    char const* name = nullptr;
    char const* defaultText = nullptr;
};

struct ClassRole
{
    uint8 playerClass;
    std::optional<uint8> requiredSpec;
    std::optional<uint8> preferredSpec;
    ClassRoleText role;
    ClassRoleText candidate;
    // Used only with preferredSpec.
    ClassRoleText preferredCandidate;
};

// Kept in the raid's instance state and not cleared by its reset: the mechanic tracker re-ranks the
// holder every second out of encounter, and clearing the announced holder would announce it again.
struct ClassRoleAssignment
{
    std::optional<ClassRoleHolder> holder;
    std::optional<ClassRoleHolder> announced;
};

struct ClassRoleSlot
{
    ClassRole const& role;
    ClassRoleAssignment& assignment;
};

// An assistant of the class beats any bot, and a bot on the preferred spec beats any other bot.
// Then the highest gear score, then the lowest GUID.
ClassRoleHolder FindClassRoleHolder(PlayerbotAI* botAI, uint32 mapId, ClassRole const& role);
// Out of encounter, re-ranks every role. In an encounter, a role passes on only once its holder
// has died or left. Returns true if a holder changed.
bool UpdateClassRoleAssignments(
    PlayerbotAI* botAI, uint32 mapId, std::initializer_list<ClassRoleSlot> slots);
// Announces each role whose holder the raid has not been told about, once the bot is within
// announceDistance of the boss's spawn, and forgets what it announced beyond rearmDistance. Both
// are 2D. Nothing is announced unless the boss is found alive, and the blocker is not, within
// rearmDistance in 3D, so it must reach the boss from inside announceDistance.
bool UpdateClassRoleAnnouncement(
    PlayerbotAI* botAI, uint32 bossEntry, Position const& bossSpawn, float announceDistance,
    float rearmDistance, std::initializer_list<ClassRoleSlot> slots,
    std::optional<uint32> blockerEntry = std::nullopt);
// Null while the holder is dead.
Player* GetClassRoleHolder(Player* bot, ClassRoleAssignment const& assignment);
ObjectGuid GetClassRoleHolderGuid(ClassRoleAssignment const& assignment);
bool IsClassRoleHolder(Player* bot, ClassRoleAssignment const& assignment);

// Sets the field if it is empty. Returns true if it did.
template <typename T, typename U>
bool EmplaceIfUnset(std::optional<T>& field, U&& value)
{
    if (field)
        return false;

    field.emplace(std::forward<U>(value));
    return true;
}

// Clears the field. Returns true if it had a value.
template <typename T>
bool ResetIfSet(std::optional<T>& field)
{
    bool const wasSet = field.has_value();
    field.reset();
    return wasSet;
}

}

#endif
