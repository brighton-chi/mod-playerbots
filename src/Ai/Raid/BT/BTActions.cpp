/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTActions.h"
#include "BTHelpers.h"
#include "CreatureAI.h"
#include "EncounterHelpers.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include <algorithm>
#include <utility>
#include <vector>

using namespace BtHelpers;
using namespace EncounterHelpers;

// General

bool BlackTempleResetEncounterStatesAction::Execute(Event /*event*/)
{
    ObjectGuid const guid = bot->GetGUID();
    uint32 const instanceId = bot->GetInstanceId();

    bool reset = false;

    reset |= flameTankWaypointIndex.erase(guid) > 0;
    reset |= illidanShadowTrapGuid.erase(guid) > 0;
    reset |= illidanShadowTrapDestination.erase(guid) > 0;

    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer") &&
        !AI_VALUE2(bool, "combat", "self target"))
    {
        if (councilDpsWaitTimer.erase(instanceId) > 0)
            reset = true;
        if (gathiosTankStep.erase(guid) > 0)
            reset = true;
        if (zerevorHealStep.erase(guid) > 0)
            reset = true;
    }

    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return reset;

    reset |= najentusSpineAssignments.erase(instanceId) > 0;
    reset |= najentusSpineThrower.erase(instanceId) > 0;

    if (!AI_VALUE2(bool, "combat", "self target"))
        reset |= shadowmoonReaverAbsorptionStart.erase(instanceId) > 0;

    reset |= illidanBossDpsWaitTimer.erase(instanceId) > 0;
    reset |= illidanFlameDpsWaitTimer.erase(instanceId) > 0;
    reset |= illidanLastPhase.erase(instanceId) > 0;
    reset |= westFlameGuid.erase(instanceId) > 0;
    reset |= eastFlameGuid.erase(instanceId) > 0;

    return reset;
}

// Shared Bosses

bool BlackTempleMisdirectToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, boss, mainTank);
}

bool BlackTemplePositionBossAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    if (AI_VALUE(Unit*, "current target") != boss)
        return Attack(boss);

    Unit* victim = boss->GetVictim();
    if (!victim)
        return false;

    if (victim != bot)
    {
        Player* playerVictim = victim->ToPlayer();
        if (!_allTanks || !playerVictim || !PlayerbotAI::IsTank(playerVictim))
            return false;
    }

    if (PlayerbotAI::IsTank(bot) && !bot->IsWithinMeleeRange(boss))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, _position, _arrivalDist, boss, moveX, moveY, backwards))
        return false;

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Trash

// Damage on the Sister of Pleasure is split evenly with her Sister of Pain, and they have equal
// health, so killing Pleasure kills both without anyone hitting Pain through Shell of Pain.
bool MarkSisterOfPleasureAction::Execute(Event /*event*/)
{
    Unit* pleasure = FindLinkedSisterOfPleasure(botAI);
    return pleasure && MarkTargetWithSkull(bot, pleasure);
}

bool ShadowmoonReaverStopWandAction::Execute(Event /*event*/)
{
    bot->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
    return true;
}

// Imps, water elementals, succubi and felhunters are kept off Reavers while anything else can be
// attacked. With only Reavers left, the pet waits passive by its owner while one is unsafe, and
// may attack them otherwise.
bool ShadowmoonReaverControlCasterPetAction::Execute(Event /*event*/)
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet)
        return false;

    if (context->GetValue<GuidVector>("shadowmoon reavers")->RefGet().empty())
        return RestoreReactState(pet);

    Unit* target = FindPetTargetOtherThanReaver(botAI);
    if (!target && IsAnyShadowmoonReaverUnsafeForMagic(botAI))
    {
        if (pet->HasReactState(REACT_PASSIVE))
            return false;

        _previousReactState = pet->GetReactState();
        _setPassive = true;
        pet->AttackStop();
        pet->InterruptNonMeleeSpells(false);
        pet->SetReactState(REACT_PASSIVE);
        if (CharmInfo* charmInfo = pet->GetCharmInfo())
        {
            charmInfo->SetIsCommandAttack(false);
            charmInfo->SetIsCommandFollow(true);
        }

        pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
        return true;
    }

    bool const restored = RestoreReactState(pet);
    Unit* victim = pet->GetVictim();
    if (!target || !victim || !victim->IsAlive() ||
        victim->GetEntry() != Id(BtNpcs::NPC_SHADOWMOON_REAVER))
    {
        return restored;
    }

    pet->AttackStop();
    pet->InterruptNonMeleeSpells(false);
    pet->ClearUnitState(UNIT_STATE_FOLLOW);
    pet->SetTarget(target->GetGUID());
    if (CharmInfo* charmInfo = pet->GetCharmInfo())
    {
        charmInfo->SetIsCommandAttack(true);
        charmInfo->SetIsAtStay(false);
        charmInfo->SetIsFollowing(false);
        charmInfo->SetIsCommandFollow(false);
        charmInfo->SetIsReturning(false);
    }

    pet->AI()->AttackStart(target);
    return true;
}

bool ShadowmoonReaverControlCasterPetAction::RestoreReactState(Guardian* pet)
{
    if (!_setPassive)
        return false;

    pet->SetReactState(_previousReactState);
    _setPassive = false;
    return true;
}

// High Warlord Naj'entus

bool HighWarlordNajentusDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    constexpr uint32 minInterval = 0;
    if (bot->GetExactDist2d(najentus) < NAJENTUS_RANGED_DISTANCE_FROM_BOSS &&
        FleePosition(najentus->GetPosition(), NAJENTUS_RANGED_DISTANCE_FROM_BOSS, minInterval))
    {
        return true;
    }

    Player* nearestPlayer = GetNearestPlayerInRadius(bot, NAJENTUS_RANGED_SPREAD_DISTANCE);
    return nearestPlayer &&
        FleePosition(nearestPlayer->GetPosition(), NAJENTUS_RANGED_SPREAD_DISTANCE);
}

bool HighWarlordNajentusAssignSpineRemoverAction::Execute(Event /*event*/)
{
    Player* impaled = FindNajentusUnassignedImpaledPlayer(bot);
    if (!impaled)
        return false;

    // Drop entries for impales that have ended, and any earlier one for this impaled player.
    std::vector<NajentusSpineAssignment>& assignments =
        najentusSpineAssignments[bot->GetInstanceId()];
    std::erase_if(assignments,
        [this, impaled](NajentusSpineAssignment const& assignment)
        {
            return assignment.impaled == impaled->GetGUID() ||
                !IsNajentusImpaled(ObjectAccessor::GetPlayer(*bot, assignment.impaled));
        });

    Player* remover = FindNajentusSpineRemover(bot, impaled);
    if (!remover)
        return false;

    assignments.push_back({ impaled->GetGUID(), remover->GetGUID() });
    return true;
}

bool HighWarlordNajentusRemoveImpalingSpineAction::Execute(Event /*event*/)
{
    Player* impaled = GetNajentusImpaledPlayerToFree(bot);
    if (!impaled)
        return false;

    // The spine is summoned where the impaled player stands.
    constexpr float searchRadius = 5.0f;
    GameObject* spineGo = impaled->FindNearestGameObject(
        Id(BtObjects::GO_NAJENTUS_SPINE), searchRadius, true);
    if (!spineGo)
        return false;

    bool const atSpine = bot->GetExactDist2d(spineGo) <= 3.0f;

    // One reaction delay per spine. If already in range, delay between 1 and 2s to click. If not
    // yet in range, delay between 2 and 3s to move and click immediately when in range.
    if (spineGo->GetGUID() != _spineGuid)
    {
        _spineGuid = spineGo->GetGUID();
        _usedSpine = false;
        _reactionStartTime = getMSTime();
        _reactionDelay = atSpine ? urand(1000, 2000) : urand(2000, 3000);
    }

    if (_usedSpine || GetMSTimeDiffToNow(_reactionStartTime) < _reactionDelay)
        return false;

    if (!atSpine)
    {
        return MoveTo(
            BT_MAP_ID, spineGo->GetPositionX(), spineGo->GetPositionY(), bot->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    spineGo->Use(bot);
    _usedSpine = true;
    return true;
}

bool HighWarlordNajentusAssignSpineThrowerAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    Player* thrower = FindNajentusSpineThrower(bot, najentus);
    if (!thrower)
        return false;

    najentusSpineThrower[bot->GetInstanceId()] = thrower->GetGUID();
    return true;
}

bool HighWarlordNajentusThrowImpalingSpineAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    if (!bot->IsWithinCombatRange(najentus, NAJENTUS_HURL_SPINE_RANGE))
    {
        float const approachDist = NAJENTUS_HURL_SPINE_RANGE - NAJENTUS_HURL_SPINE_APPROACH_MARGIN +
            bot->GetCombatReach() + najentus->GetCombatReach();
        float const angle = najentus->GetAngle(bot);
        float const targetX = najentus->GetPositionX() + approachDist * std::cos(angle);
        float const targetY = najentus->GetPositionY() + approachDist * std::sin(angle);

        return MoveTo(
            BT_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    Aura* shield = najentus->GetAura(Id(BtSpells::SPELL_TIDAL_SHIELD));
    Item* spine = bot->GetItemByEntry(Id(BtItems::ITEM_NAJENTUS_SPINE));
    if (!shield || !spine)
        return false;

    // Reaction delay of 0.5 to 1.5 s, counted from the shield going up.
    if (!_throwDelay)
        _throwDelay = urand(500, 1500);

    if (static_cast<uint32>(shield->GetMaxDuration() - shield->GetDuration()) < _throwDelay)
        return false;

    botAI->ImbueItem(spine, najentus);
    _throwDelay = 0;
    return true;
}

// Supremus

bool SupremusDisperseRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, SUPREMUS_RANGED_SPREAD_DISTANCE);
    return nearestPlayer &&
        FleePosition(nearestPlayer->GetPosition(), SUPREMUS_RANGED_SPREAD_DISTANCE);
}

// Steps around him, never into an erupting volcano or out of his room, taking the step that leaves
// the bot farthest from him. Molten Flame is ignored.
bool SupremusKiteBossAction::Execute(Event /*event*/)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus || bot->GetDistance2d(supremus) >= SUPREMUS_KITE_DISTANCE)
        return false;

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    std::vector<Unit*> const volcanoes = GetSupremusVolcanoes(botAI);
    constexpr uint8 numAngles = 16;
    constexpr float angleStep = 2.0f * M_PI / numAngles;
    std::vector<std::pair<float, Position>> candidates;
    for (uint8 i = 0; i < numAngles; ++i)
    {
        float const angle = i * angleStep;
        float const x = bot->GetPositionX() + SUPREMUS_KITE_STEP_DISTANCE * std::cos(angle);
        float const y = bot->GetPositionY() + SUPREMUS_KITE_STEP_DISTANCE * std::sin(angle);

        if (!IsInsideSupremusKiteBoundary(x, y) || IsInEruptingSupremusVolcano(volcanoes, x, y))
            continue;

        candidates.emplace_back(
            supremus->GetExactDist2d(x, y), Position(x, y, bot->GetPositionZ()));
    }

    // Farthest first, so the collision check runs only until one step passes.
    std::sort(candidates.begin(), candidates.end(),
        [](auto const& a, auto const& b) { return a.first > b.first; });

    for (auto const& candidate : candidates)
    {
        float stepX;
        float stepY;
        float stepZ;
        if (CanTakeStepTowards(
                bot, candidate.second.GetPositionX(), candidate.second.GetPositionY(),
                SUPREMUS_KITE_STEP_DISTANCE, stepX, stepY, stepZ))
        {
            return MoveTo(
                BT_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, false);
        }
    }

    return false;
}

bool SupremusMoveAwayFromVolcanosAction::Execute(Event /*event*/)
{
    std::vector<Unit*> const volcanoes = GetSupremusVolcanoes(botAI);
    if (!IsInEruptingSupremusVolcano(volcanoes, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    Position destination;
    if (!FindSafestNearbyPosition(volcanoes, destination))
        return false;

    return MoveTo(
        BT_MAP_ID, destination.GetPositionX(), destination.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// The nearest spot a yard clear of the trigger's radius from every erupting volcano, so reach has
// room before avoidance pushes the bot back out. A spot whose path crosses no other volcano is
// preferred; otherwise the nearest clear spot.
bool SupremusMoveAwayFromVolcanosAction::FindSafestNearbyPosition(
    std::vector<Unit*> const& volcanoes, Position& destination)
{
    constexpr float maxRadius = 40.0f;
    constexpr float distanceStep = 1.0f;
    constexpr uint8 numAngles = 16;
    constexpr float angleStep = 2.0f * M_PI / numAngles;
    constexpr float clearance = SUPREMUS_VOLCANO_SAFE_DISTANCE + 1.0f;
    constexpr uint32 numDistances = static_cast<uint32>(maxRadius / distanceStep);

    bool found = false;
    for (uint32 i = 1; i <= numDistances; ++i)
    {
        float const distance = i * distanceStep;
        for (uint8 j = 0; j < numAngles; ++j)
        {
            float const angle = j * angleStep;
            float const x = bot->GetPositionX() + distance * std::cos(angle);
            float const y = bot->GetPositionY() + distance * std::sin(angle);
            if (IsInEruptingSupremusVolcano(volcanoes, x, y, clearance))
                continue;

            Position const candidate(x, y, bot->GetPositionZ());
            if (IsPathSafeFromVolcanos(bot->GetPosition(), candidate, volcanoes))
            {
                destination = candidate;
                return true;
            }

            if (!found)
            {
                destination = candidate;
                found = true;
            }
        }
    }

    return found;
}

// A path is unsafe only through a volcano the bot isn't already in; leaving the one it stands in
// means crossing part of it.
bool SupremusMoveAwayFromVolcanosAction::IsPathSafeFromVolcanos(
    Position const& start, Position const& end, std::vector<Unit*> const& volcanoes)
{
    constexpr uint8 numChecks = 10;
    float const dx = end.GetPositionX() - start.GetPositionX();
    float const dy = end.GetPositionY() - start.GetPositionY();

    for (Unit* volcano : volcanoes)
    {
        if (!IsSupremusVolcanoErupting(volcano) ||
            volcano->GetExactDist2d(&start) < SUPREMUS_VOLCANO_SAFE_DISTANCE)
        {
            continue;
        }

        for (uint8 i = 1; i <= numChecks; ++i)
        {
            float const ratio = static_cast<float>(i) / numChecks;
            float const checkX = start.GetPositionX() + dx * ratio;
            float const checkY = start.GetPositionY() + dy * ratio;

            if (volcano->GetExactDist2d(checkX, checkY) < SUPREMUS_VOLCANO_SAFE_DISTANCE)
                return false;
        }
    }

    return true;
}

// Shade of Akama

// Channelers stand on a platform out of sight from below, and Attack refuses a target out of
// sight, so a bot walks the path toward its target until it can see it.
bool ShadeOfAkamaMeleeDpsPrioritizeChannelersAction::Execute(Event /*event*/)
{
    Unit* target = GetShadeOfAkamaKillTarget(botAI);
    if (!target)
        return false;

    if (MarkTargetWithSkull(bot, target))
        return true;

    if (!bot->IsWithinLOSInMap(target))
    {
        constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
        if (IsWaitingForLastMove(priority))
            return false;

        constexpr float stopDistance = 5.0f;
        float stepX;
        float stepY;
        if (!GetPathStepTowardUnit(bot, target, stopDistance, stepX, stepY))
            return false;

        return MoveTo(
            BT_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false, priority,
            true, false);
    }

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    return false;
}

// Teron Gorefiend

// Assume positions in arc at the edge of the balcony (farthest from Constructs)
bool TeronGorefiendPositionRangedOnBalconyAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    size_t const count = rangedMembers.size();
    auto const findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t const botIndex = (findIt != rangedMembers.end()) ?
        std::distance(rangedMembers.begin(), findIt) : 0;

    constexpr float arcSpan = 2.0f * M_PI / 5.0f;
    constexpr float arcCenter = 6.279f;
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    constexpr float radius = 12.0f;
    float const angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float const targetX = GOREFIEND_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
    float const targetY = GOREFIEND_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) <= GOREFIEND_POSITION_TOLERANCE)
        return false;

    return MoveTo(
        BT_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool TeronGorefiendAvoidShadowOfDeathAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_HUNTER)
    {
        return botAI->CanCastSpell(Id(BtSpells::SPELL_FEIGN_DEATH), bot) &&
            botAI->CastSpell(Id(BtSpells::SPELL_FEIGN_DEATH), bot);
    }
    else
    {
        return botAI->CanCastSpell("vanish", bot) && botAI->CastSpell("vanish", bot);
    }
}

bool TeronGorefiendMoveToCornerToDieAction::Execute(Event /*event*/)
{
    Position const& position = GOREFIEND_DIE_POSITION;
    if (bot->GetExactDist2d(position) <= GOREFIEND_POSITION_TOLERANCE)
        return false;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool TeronGorefiendControlAndDestroyShadowyConstructsAction::Execute(Event /*event*/)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    Unit* spirit = bot->GetCharm();
    if (!spirit)
        return false;

    // The construct nearest Teron leads the way to the raid. Lances break Chains, so they free
    // the constructs from the back, keeping the rest together.
    Unit* leadConstruct = nullptr;
    Unit* lastConstruct = nullptr;
    float leadDistance = 0.0f;
    float lastDistance = 0.0f;
    for (ObjectGuid const& guid : AI_VALUE(GuidVector, "possible targets no los"))
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() ||
            unit->GetEntry() != Id(BtNpcs::NPC_SHADOWY_CONSTRUCT))
        {
            continue;
        }

        float const distance = gorefiend->GetExactDist2d(unit);
        if (!leadConstruct || distance < leadDistance)
        {
            leadConstruct = unit;
            leadDistance = distance;
        }

        if (!lastConstruct || distance > lastDistance)
        {
            lastConstruct = unit;
            lastDistance = distance;
        }
    }

    if (!leadConstruct)
    {
        Unit* victim = gorefiend->GetVictim();
        if (victim && CastVengefulSpiritSpell(
                          spirit, victim, Id(BtSpells::SPELL_SPIRIT_SHIELD)))
        {
            return true;
        }

        bool moving = false;
        if (!spirit->IsWithinMeleeRange(gorefiend))
        {
            float const distance = spirit->GetExactDist2d(gorefiend);
            float const moveDistance = distance - gorefiend->GetCombatReach();
            float const dX = gorefiend->GetPositionX() - spirit->GetPositionX();
            float const dY = gorefiend->GetPositionY() - spirit->GetPositionY();
            spirit->GetMotionMaster()->MovePoint(
                0, spirit->GetPositionX() + dX / distance * moveDistance,
                spirit->GetPositionY() + dY / distance * moveDistance, spirit->GetPositionZ());
            moving = true;
        }

        return CastVengefulSpiritSpell(
            spirit, gorefiend, Id(BtSpells::SPELL_SPIRIT_STRIKE)) || moving;
    }

    bool moving = false;
    float const distanceToLead = spirit->GetExactDist2d(leadConstruct);
    if (distanceToLead > GOREFIEND_SPIRIT_AOE_DISTANCE)
    {
        float const moveDistance = distanceToLead - GOREFIEND_SPIRIT_AOE_DISTANCE + 2.0f;
        float const dX = leadConstruct->GetPositionX() - spirit->GetPositionX();
        float const dY = leadConstruct->GetPositionY() - spirit->GetPositionY();
        spirit->GetMotionMaster()->MovePoint(
            0, spirit->GetPositionX() + dX / distanceToLead * moveDistance,
            spirit->GetPositionY() + dY / distanceToLead * moveDistance, spirit->GetPositionZ());
        moving = true;
    }
    else if (CastVengefulSpiritSpell(
                 spirit, leadConstruct, Id(BtSpells::SPELL_SPIRIT_VOLLEY)) ||
             CastVengefulSpiritSpell(
                 spirit, leadConstruct, Id(BtSpells::SPELL_SPIRIT_CHAINS)))
    {
        return true;
    }

    return CastVengefulSpiritSpell(
        spirit, lastConstruct, Id(BtSpells::SPELL_SPIRIT_LANCE)) || moving;
}

// Gurtogg Bloodboil

bool GurtoggBloodboilRotateRangedGroupsAction::Execute(Event /*event*/)
{
    Position const& position = GetGurtoggBloodboilPosition(bot);
    return MoveInside(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        GURTOGG_POSITION_TOLERANCE, MovementPriority::MOVEMENT_FORCED);
}

// Reliquary of Souls

bool ReliquaryOfSoulsMisdirectToMainTankAction::Execute(Event /*event*/)
{
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");

    if (!desire && !anger)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    Unit* target = desire ? desire : anger;

    return target->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT &&
        MisdirectTargetToTank(botAI, target, mainTank);
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::Execute(Event /*event*/)
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering)
        return false;

    if (IsSufferingFixateTank(bot))
        return TanksMoveToMinimumRange(suffering);

    if (PlayerbotAI::IsMelee(bot))
        return MeleeDpsStayAtMaximumRange(suffering);

    return RangedMoveAwayFromBoss(suffering);
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::TanksMoveToMinimumRange(Unit* suffering)
{
    float const distanceToBoss = bot->GetExactDist2d(suffering);
    if (distanceToBoss <= SUFFERING_TANK_DISTANCE)
        return false;

    float const dX = suffering->GetPositionX() - bot->GetPositionX();
    float const dY = suffering->GetPositionY() - bot->GetPositionY();
    float const targetX = bot->GetPositionX() + (dX / distanceToBoss);
    float const targetY = bot->GetPositionY() + (dY / distanceToBoss);

    return MoveTo(
        BT_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::MeleeDpsStayAtMaximumRange(Unit* suffering)
{
    Position const position = GetSufferingMeleePosition(bot, suffering);
    if (bot->GetExactDist2d(position) <= SUFFERING_MELEE_POSITION_TOLERANCE)
        return false;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::RangedMoveAwayFromBoss(Unit* suffering)
{
    if (bot->GetExactDist2d(suffering) >= SUFFERING_RANGED_DISTANCE)
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(suffering->GetPosition(), SUFFERING_RANGED_DISTANCE, minInterval);
}

bool ReliquaryOfSoulsHealersDpsSufferingAction::Execute(Event /*event*/)
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering)
        return false;

    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            if (bot->HasAura(Id(BtSpells::SPELL_TREE_OF_LIFE)))
            {
                bot->RemoveOwnedAura(
                    Id(BtSpells::SPELL_TREE_OF_LIFE), ObjectGuid::Empty, 0,
                    AURA_REMOVE_BY_CANCEL);
            }

            if (botAI->CanCastSpell("barkskin", bot) && botAI->CastSpell("barkskin", bot))
                return true;

            return botAI->CanCastSpell("wrath", suffering) && botAI->CastSpell("wrath", suffering);
        }
        case CLASS_PALADIN:
        {
            for (char const* spell : { "avenging wrath", "consecration" })
            {
                if (botAI->CanCastSpell(spell, bot) && botAI->CastSpell(spell, bot))
                    return true;
            }

            for (char const* spell :
                 { "exorcism", "hammer of wrath", "holy shock", "judgement of light" })
            {
                if (botAI->CanCastSpell(spell, suffering) && botAI->CastSpell(spell, suffering))
                    return true;
            }

            return false;
        }
        case CLASS_PRIEST:
            return botAI->CanCastSpell("smite", suffering) && botAI->CastSpell("smite", suffering);
        case CLASS_SHAMAN:
        {
            for (char const* spell : { "earth shock", "chain lightning", "lightning bolt" })
            {
                if (botAI->CanCastSpell(spell, suffering) && botAI->CastSpell(spell, suffering))
                    return true;
            }

            return false;
        }
        default:
            return false;
    }
}

bool ReliquaryOfSoulsSpellstealRuneShieldAction::Execute(Event /*event*/)
{
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire)
        return false;

    if (!botAI->CanCastSpell(Id(BtSpells::SPELL_SPELLSTEAL), desire))
        return false;

    return botAI->CastSpell(Id(BtSpells::SPELL_SPELLSTEAL), desire);
}

bool ReliquaryOfSoulsSpellReflectDeadenAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(BtSpells::SPELL_SPELL_REFLECTION), bot) &&
        botAI->CastSpell(Id(BtSpells::SPELL_SPELL_REFLECTION), bot);
}

// Mother Shahraz

bool MotherShahrazTanksPositionBossUnderPillarAction::Execute(Event /*event*/)
{
    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz)
        return false;

    if (AI_VALUE(Unit*, "current target") != shahraz)
        return Attack(shahraz);

    Unit* victim = shahraz->GetVictim();
    if (!victim)
        return false;

    // Saber Lash is split between her victim and the two closest to it.
    if (victim != bot)
    {
        if (bot->GetExactDist2d(victim) <= SHAHRAZ_OFF_TANK_DISTANCE)
            return false;

        return MoveTo(
            BT_MAP_ID, victim->GetPositionX(), victim->GetPositionY(), bot->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    float const distToTankPosition = bot->GetExactDist2d(SHAHRAZ_TANK_POSITION);
    if (distToTankPosition <= SHAHRAZ_TANK_POSITION_TOLERANCE)
    {
        if (!bot->HasInArc(static_cast<float>(M_PI) / 2.0f, shahraz))
            bot->SetFacingTo(bot->GetAngle(shahraz));

        return false;
    }

    if (!bot->IsWithinMeleeRange(shahraz))
        return false;

    // Within this distance of the tank spot, the bot is past the statue.
    float const finalLegDistance =
        SHAHRAZ_TRANSITION_POSITION.GetExactDist2d(SHAHRAZ_TANK_POSITION) +
        SHAHRAZ_TANK_POSITION_TOLERANCE;
    Position const& position = distToTankPosition <= finalLegDistance ?
        SHAHRAZ_TANK_POSITION : SHAHRAZ_TRANSITION_POSITION;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool MotherShahrazMeleeDpsWaitAtSafePositionAction::Execute(Event /*event*/)
{
    Position const& position = SHAHRAZ_RANGED_POSITION;
    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);

}

// This doesn't matter for bots since they don't take fall damage, and it's actually easier
// to tank her closer to her starting position, but I want to simulate a player strategy
bool MotherShahrazPositionRangedUnderPillarAction::Execute(Event /*event*/)
{
    Position const& position = SHAHRAZ_RANGED_POSITION;
    if (bot->GetExactDist2d(position) <= 1.0f)
        return false;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool MotherShahrazBreakFatalAttractionAction::Execute(Event /*event*/)
{
    std::vector<Player*> const attractedPlayers = GetAttractedPlayers();
    if (attractedPlayers.size() < 2)
        return false;

    float centerX = 0.0f, centerY = 0.0f;
    for (Player* member : attractedPlayers)
    {
        centerX += member->GetPositionX();
        centerY += member->GetPositionY();
    }
    centerX /= attractedPlayers.size();
    centerY /= attractedPlayers.size();

    auto const botIt = std::find(attractedPlayers.begin(), attractedPlayers.end(), bot);
    if (botIt == attractedPlayers.end())
        return false;

    // They all land on one spot, so each takes its own heading by index, then keeps away
    // from the others as they part.
    float spreadAngle;
    constexpr float minCenterDistance = 1.0f;
    if (bot->GetExactDist2d(centerX, centerY) > minCenterDistance)
    {
        spreadAngle = bot->GetAngle(centerX, centerY) + static_cast<float>(M_PI);
    }
    else
    {
        spreadAngle = 2.0f * static_cast<float>(M_PI) *
            std::distance(attractedPlayers.begin(), botIt) / attractedPlayers.size();
    }

    // Straight on first, then alternately left and right out to 90 degrees, so a bot against a
    // wall slides along it.
    constexpr uint8 numCandidates = 9;
    constexpr float angleStep = static_cast<float>(M_PI) / 8.0f;
    for (uint8 i = 0; i < numCandidates; ++i)
    {
        int8 const side = (i % 2) ? 1 : -1;
        float const angle = spreadAngle + side * ((i + 1) / 2) * angleStep;
        float const x = bot->GetPositionX() +
            std::cos(angle) * SHAHRAZ_FATAL_ATTRACTION_STEP_DISTANCE;
        float const y = bot->GetPositionY() +
            std::sin(angle) * SHAHRAZ_FATAL_ATTRACTION_STEP_DISTANCE;

        float stepX;
        float stepY;
        float stepZ;
        if (CanTakeStepTowards(
                bot, x, y, SHAHRAZ_FATAL_ATTRACTION_STEP_DISTANCE, stepX, stepY, stepZ))
        {
            return MoveTo(
                BT_MAP_ID, stepX, stepY, stepZ, false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

std::vector<Player*> MotherShahrazBreakFatalAttractionAction::GetAttractedPlayers()
{
    std::vector<Player*> attractedPlayers;
    Group* group = bot->GetGroup();
    if (!group)
        return attractedPlayers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION)))
        {
            attractedPlayers.push_back(member);
        }
    }

    std::sort(attractedPlayers.begin(), attractedPlayers.end(),
        [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    return attractedPlayers;
}

// Illidari Council

bool IllidariCouncilMisdirectToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && member->IsAlive() &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 4)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* councilTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
        tankTarget = GetZerevorMageTank(botAI);
    }
    else if (hunterIndex == 1)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "lady malande");
        tankTarget = GetGroupAssistTank(bot, 0);
    }
    else if (hunterIndex == 2)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
        tankTarget = GetGroupMainTank(bot);
    }
    else if (hunterIndex == 3)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "veras darkshadow");
        tankTarget = GetGroupAssistTank(bot, 1);
    }

    if (!tankTarget || !tankTarget->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, councilTarget, tankTarget);
}

bool IllidariCouncilMainTankPositionGathiosAction::Execute(Event /*event*/)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return false;

    // Failsafe for if bot falls through the floor, which tends to happen upon the pull
    if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
    {
        bot->TeleportTo(BT_MAP_ID, gathios->GetPositionX(), gathios->GetPositionY(),
                        gathios->GetPositionZ(), bot->GetOrientation());
    }

    if (MarkTargetWithSquare(bot, gathios))
        return true;

    SetRtiTarget(botAI, "square");

    if (AI_VALUE(Unit*, "current target") != gathios)
        return Attack(gathios);

    ObjectGuid const guid = bot->GetGUID();
    uint8 index = gathiosTankStep.count(guid) ? gathiosTankStep[guid] : 0;

    if (gathios->GetVictim() != bot || !bot->IsWithinMeleeRange(gathios))
        return false;

    constexpr float arrivalDist = 2.0f;
    if (bot->GetExactDist2d(GATHIOS_TANK_POSITIONS[index]) <= arrivalDist &&
        HasDangerousCouncilAura(bot))
    {
        index = (index + 1) % GATHIOS_TANK_POSITIONS.size();
        gathiosTankStep[guid] = index;
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, GATHIOS_TANK_POSITIONS[index], arrivalDist, gathios, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool IllidariCouncilMainTankReflectJudgementOfCommandAction::Execute(Event /*event*/)
{
    if (botAI->CanCastSpell("spell reflection", bot))
        return botAI->CastSpell("spell reflection", bot);

    return false;
}

bool IllidariCouncilFirstAssistTankFocusMalandeAction::Execute(Event /*event*/)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    // Failsafe for if bot falls through the floor, which tends to happen upon the pull
    if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
    {
        bot->TeleportTo(BT_MAP_ID, malande->GetPositionX(), malande->GetPositionY(),
                        malande->GetPositionZ(), bot->GetOrientation());
    }

    if (MarkTargetWithStar(bot, malande))
        return true;

    SetRtiTarget(botAI, "star");

    if (AI_VALUE(Unit*, "current target") != malande)
        return Attack(malande);

    return false;
}

bool IllidariCouncilSecondAssistTankPositionDarkshadowAction::Execute(Event /*event*/)
{
    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (!darkshadow)
        return false;

    // Failsafe for if bot falls through the floor, which tends to happen upon the pull
    if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
    {
        bot->TeleportTo(BT_MAP_ID, darkshadow->GetPositionX(), darkshadow->GetPositionY(),
                        darkshadow->GetPositionZ(), bot->GetOrientation());
    }

    if (MarkTargetWithCircle(bot, darkshadow))
        return true;

    SetRtiTarget(botAI, "circle");

    if (AI_VALUE(Unit*, "current target") != darkshadow)
        return Attack(darkshadow);

    if (darkshadow->GetVictim() != bot)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, mainTank->GetPosition(), arrivalDist, darkshadow, moveX, moveY, backwards))
        return false;

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool IllidariCouncilMageTankPositionZerevorAction::Execute(Event /*event*/)
{
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor)
        return false;

    if (zerevor->HasAura(Id(BtSpells::SPELL_DAMPEN_MAGIC)) &&
        botAI->CanCastSpell("spellsteal", zerevor))
    {
        return botAI->CastSpell("spellsteal", zerevor);
    }

    if (MarkTargetWithTriangle(bot, zerevor))
        return true;

    SetRtiTarget(botAI, "triangle");

    if (AI_VALUE(Unit*, "current target") != zerevor)
        return Attack(zerevor);

    if (zerevor->GetVictim() != bot)
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, ZEREVOR_TANK_POSITION, arrivalDist, zerevor, moveX, moveY, backwards))
        return false;

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool IllidariCouncilPositionMageTankHealerAction::Execute(Event /*event*/)
{
    Player* mageTank = GetZerevorMageTank(botAI);
    if (!mageTank)
        return false;

    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor || zerevor->GetVictim() != mageTank)
        return false;

    ObjectGuid const guid = bot->GetGUID();
    uint8 index = zerevorHealStep.count(guid) ? zerevorHealStep[guid] : 0;

    constexpr float arrivalDist = 1.0f;
    MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (bot->GetExactDist2d(ZEREVOR_HEALER_POSITIONS[index]) <= arrivalDist &&
        HasDangerousCouncilAura(bot))
    {
        index = (index + 1) % ZEREVOR_HEALER_POSITIONS.size();
        zerevorHealStep[guid] = index;
        priority = MovementPriority::MOVEMENT_FORCED;
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, ZEREVOR_HEALER_POSITIONS[index], arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        priority, true, backwards);
}

bool IllidariCouncilDisperseRangedAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 4.0f;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        return FleePosition(nearestPlayer->GetPosition(), safeDistance);

    return false;
}

bool IllidariCouncilCommandPetsToAttackGathiosAction::Execute(Event /*event*/)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return false;

    Pet* pet = bot->GetPet();
    if (pet && pet->IsAlive() && pet->GetVictim() != gathios)
    {
        pet->ClearUnitState(UNIT_STATE_FOLLOW);
        pet->AttackStop();
        pet->SetTarget(gathios->GetGUID());

        if (pet->GetCharmInfo())
        {
            pet->GetCharmInfo()->SetIsCommandAttack(true);
            pet->GetCharmInfo()->SetIsAtStay(false);
            pet->GetCharmInfo()->SetIsFollowing(false);
            pet->GetCharmInfo()->SetIsCommandFollow(false);
            pet->GetCharmInfo()->SetIsReturning(false);

            pet->AI()->AttackStart(gathios);
            return true;
        }
    }

    return false;
}

bool IllidariCouncilAssignDpsTargetsAction::Execute(Event /*event*/)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    bool shouldAttackMalande = false;
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (zerevor && zerevor->GetExactDist2d(malande) < 15.0f)
    {
        shouldAttackMalande = false;
    }
    else if (bot->getClass() == CLASS_ROGUE ||
             (bot->getClass() == CLASS_WARRIOR && PlayerbotAI::IsDps(bot)))
    {
        shouldAttackMalande = !malande->HasAura(Id(BtSpells::SPELL_BLESSING_OF_PROTECTION));
    }
    else if (bot->getClass() == CLASS_SHAMAN && PlayerbotAI::IsDps(bot))
    {
        shouldAttackMalande = !malande->HasAura(Id(BtSpells::SPELL_BLESSING_OF_SPELL_WARDING));
    }

    if (shouldAttackMalande)
    {
        SetRtiTarget(botAI, "star");

        if (AI_VALUE(Unit*, "current target") != malande)
            return Attack(malande);
    }
    else if (Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
        darkshadow && !IsDarkshadowVanished(darkshadow))
    {
        SetRtiTarget(botAI, "circle");

        if (AI_VALUE(Unit*, "current target") != darkshadow)
            return Attack(darkshadow);
    }
    else if (Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
    {
        SetRtiTarget(botAI, "square");

        if (AI_VALUE(Unit*, "current target") != gathios)
            return Attack(gathios);
    }

    return false;
}

bool IllidariCouncilManageDpsTimerAction::Execute(Event /*event*/)
{
    if (Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
    {
        return councilDpsWaitTimer.try_emplace(
            gathios->GetMap()->GetInstanceId(), getMSTime()).second;
    }

    return false;
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageMisdirectToTanksAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int const phase = GetIllidanPhase(illidan);

    if (phase == 2 && TryMisdirectToFlameTanks(group))
        return true;

    return phase == 4 && TryMisdirectToWarlockTank(illidan);
}

bool IllidanStormrageMisdirectToTanksAction::TryMisdirectToFlameTanks(Group* group)
{
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && member->IsAlive() &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 2)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    auto const [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
    if (!eastFlame || !westFlame || eastFlame == westFlame)
        return false;

    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    Player* secondAssistTank = GetGroupAssistTank(bot, 1);
    if (!firstAssistTank || !secondAssistTank)
        return false;

    if (hunters.size() == 1)
    {
        return eastFlame->GetHealthPct() >= 99.0f &&
            MisdirectTargetToTank(botAI, eastFlame, secondAssistTank);
    }

    Player* tankTarget = nullptr;
    Unit* flame = nullptr;

    if (hunterIndex == 0)
    {
        tankTarget = secondAssistTank;
        flame = eastFlame;
    }
    else if (hunterIndex == 1)
    {
        tankTarget = firstAssistTank;
        flame = westFlame;
    }
    else
        return false;

    if (!tankTarget || !tankTarget->IsAlive() || flame->GetHealthPct() < 90.0f)
        return false;

    return MisdirectTargetToTank(botAI, flame, tankTarget);
}

bool IllidanStormrageMisdirectToTanksAction::TryMisdirectToWarlockTank(Unit* illidan)
{
    if (!illidan)
        return false;

    return MisdirectTargetToTank(botAI, illidan, GetIllidanWarlockTank(botAI));
}

bool IllidanStormrageMainTankRepositionBossAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (AI_VALUE(Unit*, "current target") != illidan)
        return Attack(illidan);

    if (GetIllidanPhase(illidan) == 5)
    {
        GameObject* trap = FindNearestTrap(botAI);
        if (trap && bot->GetExactDist2d(trap) < 40.0f && illidan->GetVictim() == bot)
            return MoveToShadowTrap(illidan, trap);
    }
    else
    {
        illidanShadowTrapGuid.erase(bot->GetGUID());
        illidanShadowTrapDestination.erase(bot->GetGUID());
    }

    if (illidan->GetVictim() != bot)
    {
        illidanShadowTrapGuid.erase(bot->GetGUID());
        illidanShadowTrapDestination.erase(bot->GetGUID());
        return false;
    }

    std::vector<Unit*> const flameCrashes = GetAllFlameCrashes(bot);
    if (flameCrashes.empty())
        return false;

    constexpr float hazardRadius = 12.0f;
    bool inDanger = false;
    for (Unit* flameCrash : flameCrashes)
    {
        if (bot->GetDistance2d(flameCrash) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    constexpr float maxRadius = 30.0f;
    Position const safestPos = FindSafestNearbyPosition(flameCrashes, maxRadius, hazardRadius);

    return MoveTo(BT_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, true);
}

bool IllidanStormrageMainTankRepositionBossAction::MoveToShadowTrap(Unit* illidan, GameObject* trap)
{
    if (!illidan || !trap)
        return false;

    ObjectGuid const botGuid = bot->GetGUID();
    ObjectGuid const trapGuid = trap->GetGUID();
    Position target;

    auto const cachedTrapIt = illidanShadowTrapGuid.find(botGuid);
    auto const cachedDestinationIt = illidanShadowTrapDestination.find(botGuid);
    if (cachedTrapIt != illidanShadowTrapGuid.end() &&
        cachedDestinationIt != illidanShadowTrapDestination.end() &&
        cachedTrapIt->second == trapGuid)
    {
        target = cachedDestinationIt->second;
    }
    else
    {
        float const trapX = trap->GetPositionX();
        float const trapY = trap->GetPositionY();

        float const distToTrap = trap->GetExactDist2d(bot);
        if (distToTrap <= 0.0f)
            return false;

        constexpr float distBeyondTrap = 6.0f;

        float const dx = trapX - bot->GetPositionX();
        float const dy = trapY - bot->GetPositionY();
        float const targetX = trapX + (dx / distToTrap) * distBeyondTrap;
        float const targetY = trapY + (dy / distToTrap) * distBeyondTrap;

        target = Position(targetX, targetY, trap->GetPositionZ());
        illidanShadowTrapGuid[botGuid] = trapGuid;
        illidanShadowTrapDestination[botGuid] = target;
    }

    if (bot->GetHealthPct() <= 50.0f)
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, target, arrivalDist, illidan, moveX, moveY, backwards))
        return false;

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

Position IllidanStormrageMainTankRepositionBossAction::FindSafestNearbyPosition(
    std::vector<Unit*> const& flameCrashes, float maxRadius, float hazardRadius)
{
    constexpr uint8 numAngles = 32;
    constexpr float angleStep = 2.0f * M_PI / numAngles;
    constexpr float minDistance = 2.0f;
    constexpr float distanceStep = 1.0f;
    uint32 const numDistances = static_cast<uint32>((maxRadius - minDistance) / distanceStep);

    float const backwardsAngle = Position::NormalizeOrientation(bot->GetOrientation() + M_PI);

    Position bestPos;
    float bestAngleDiff = M_PI * 2.0f;
    float bestDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (uint32 i = 0; i <= numDistances; ++i)
    {
        float const distance = minDistance + i * distanceStep;
        for (uint8 j = 0; j < numAngles; ++j)
        {
            float const angleOffset = j * angleStep;
            for (int sign = -1; sign <= 1; sign += 2)
            {
                float const testAngle =
                    Position::NormalizeOrientation(backwardsAngle + sign * angleOffset);
                float const x = bot->GetPositionX() + distance * std::cos(testAngle);
                float const y = bot->GetPositionY() + distance * std::sin(testAngle);

                Position const testPos(x, y, bot->GetPositionZ());

                bool isSafe = true;
                for (Unit* flameCrash : flameCrashes)
                {
                    if (flameCrash->GetDistance2d(x, y) < hazardRadius)
                    {
                        isSafe = false;
                        break;
                    }
                }
                if (!isSafe)
                    continue;

                bool const pathSafe = IsPathSafeFromFlameCrashes(
                    bot->GetPosition(), testPos, flameCrashes, hazardRadius);

                float angleDiff = std::abs(Position::NormalizeOrientation(
                                           testAngle - backwardsAngle));
                if (angleDiff > M_PI)
                    angleDiff = 2 * M_PI - angleDiff;

                if (pathSafe && (!foundSafe || angleDiff < bestAngleDiff ||
                    (angleDiff == bestAngleDiff && distance < bestDistance)))
                {
                    bestPos = testPos;
                    bestAngleDiff = angleDiff;
                    bestDistance = distance;
                    foundSafe = true;
                }
                else if (!foundSafe && angleDiff < bestAngleDiff)
                {
                    bestPos = testPos;
                    bestAngleDiff = angleDiff;
                    bestDistance = distance;
                }
            }
            if (foundSafe)
                break;
        }
        if (foundSafe)
            break;
    }

    return bestPos;
}

bool IllidanStormrageMainTankRepositionBossAction::IsPathSafeFromFlameCrashes(
    Position const& start, Position const& end, std::vector<Unit*> const& flameCrashes,
    float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    float const dx = end.GetPositionX() - start.GetPositionX();
    float const dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float const ratio = static_cast<float>(i) / numChecks;
        float const checkX = start.GetPositionX() + dx * ratio;
        float const checkY = start.GetPositionY() + dy * ratio;

        for (Unit* flameCrash : flameCrashes)
        {
            float const distToFlameCrash = flameCrash->GetDistance2d(checkX, checkY);
            if (distToFlameCrash < hazardRadius)
                return false;
        }
    }

    return true;
}

bool IllidanStormrageIsolateBotWithParasiteAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int const phase = GetIllidanPhase(illidan);

    if (phase == 1)
    {
        constexpr float safeDistance = 15.0f;
        if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        {
            float const currentDistance = bot->GetExactDist2d(nearestPlayer);
            if (currentDistance < safeDistance)
                return MoveAway(nearestPlayer, safeDistance - currentDistance);
        }
    }
    else
    {
        float const angle = illidan->GetOrientation() + M_PI;
        constexpr float distBehindIllidan = 35.0f;

        float const targetX = illidan->GetPositionX() + std::cos(angle) * distBehindIllidan;
        float const targetY = illidan->GetPositionY() + std::sin(angle) * distBehindIllidan;
        Position const target(targetX, targetY, bot->GetPositionZ());

        if (HasParasiticShadowfiend(bot))
            return InfectedBotMoveFromGroup(target);

        if (GetIllidanTrapperHunter(bot) == bot)
            return FreezeTrapShadowfiend(target);
    }

    return false;
}

bool IllidanStormrageIsolateBotWithParasiteAction::InfectedBotMoveFromGroup(Position const& target)
{
    if (bot->GetExactDist2d(target) < 1.0f)
        return false;

    return MoveTo(BT_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                  target.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IllidanStormrageIsolateBotWithParasiteAction::FreezeTrapShadowfiend(Position const& target)
{
    if (bot->HasSpellCooldown(Id(BtSpells::SPELL_FROST_TRAP)))
        return false;

    Player* infected = GetBotWithParasiticShadowfiend(botAI);
    if (!infected)
        return false;

    if (bot->GetExactDist2d(target) > 2.0f)
    {
        return MoveTo(BT_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                      target.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (bot->GetExactDist2d(infected) < 2.0f &&
             botAI->CanCastSpell(Id(BtSpells::SPELL_FROST_TRAP), bot))
    {
        return botAI->CastSpell(Id(BtSpells::SPELL_FROST_TRAP), bot);
    }

    return false;
}

bool IllidanStormrageSetEarthbindTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell("earthbind totem", bot) &&
           botAI->CastSpell("earthbind totem", bot);
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::Execute(Event /*event*/)
{
    auto const [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
    // The second assist tank's flame is killed first; this is so that if the tank
    // for the second flame dies after the first flame is down, the dead flame's
    // tank will become the first assist tank and take over the remaining flame
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        if (eastFlame && westFlame)
        {
            if (AI_VALUE(Unit*, "current target") != eastFlame)
                return Attack(eastFlame);

            if (eastFlame->GetVictim() != bot)
            {
                if (!bot->IsWithinMeleeRange(eastFlame))
                {
                    return MoveTo(BT_MAP_ID, eastFlame->GetPositionX(),
                                  eastFlame->GetPositionY(), eastFlame->GetPositionZ(),
                                  false, false, false, false,
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
                return false;
            }
        }
        else if (!eastFlame && !westFlame)
        {
            // (1) Before flames spawn, go to the waiting position
            // (2) If both flames are dead and the waiting position is too close to hazards,
            //     move to a grate position
            std::list<Creature*> demonFires;
            constexpr float searchRadius = 40.0f;
            bot->GetCreatureListWithEntryInGrid(
                demonFires, Id(BtNpcs::NPC_DEMON_FIRE), searchRadius);

            Position const& pos = demonFires.empty() ?
                ILLIDAN_E_GLAIVE_WAITING_POSITION : ILLIDAN_E_GRATE_POSITION;

            if (bot->GetExactDist2d(pos) > 0.5f)
            {
                return MoveTo(BT_MAP_ID, pos.GetPositionX(), pos.GetPositionY(),
                              pos.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        // After the first flame dies, its tank waits with other bots
        else if (!eastFlame && westFlame)
        {
            Position const& pos = ILLIDAN_E_GRATE_POSITION;
            if (bot->GetExactDist2d(pos) > 0.5f)
            {
                return MoveTo(BT_MAP_ID, pos.GetPositionX(), pos.GetPositionY(),
                              pos.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        if (westFlame)
        {
            if (AI_VALUE(Unit*, "current target") != westFlame)
                return Attack(westFlame);

            if (westFlame->GetVictim() != bot)
            {
                if (!bot->IsWithinMeleeRange(westFlame))
                {
                    return MoveTo(BT_MAP_ID, westFlame->GetPositionX(),
                                  westFlame->GetPositionY(), westFlame->GetPositionZ(),
                                  false, false, false, false,
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
                return false;
            }
        }
        else
        {
            // (1) Before flames spawn, go to the waiting position
            // (2) If both flames are dead and the waiting position is too close to hazards,
            //     move to a grate position
            std::list<Creature*> demonFires;
            constexpr float searchRadius = 40.0f;
            bot->GetCreatureListWithEntryInGrid(
                demonFires, Id(BtNpcs::NPC_DEMON_FIRE), searchRadius);

            Position const& pos = demonFires.empty() ?
                ILLIDAN_W_GLAIVE_WAITING_POSITION : ILLIDAN_W_GRATE_POSITION;

            if (bot->GetExactDist2d(pos) > 0.5f)
            {
                return MoveTo(BT_MAP_ID, pos.GetPositionX(), pos.GetPositionY(),
                              pos.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    if (!AI_VALUE2(Unit*, "find target", "illidan stormrage"))
        return false;

    EyeBlastDangerArea const dangerArea = GetEyeBlastDangerArea(bot);

    // Only consider the eye blast if its trigger NPC is within 30 yards of the tank
    constexpr float eyeBlastTriggerRadius = 30.0f;
    if (dangerArea.width > 0.0f &&
        bot->GetExactDist2d(dangerArea.start) <= eyeBlastTriggerRadius)
    {
        return RepositionToAvoidEyeBlast(dangerArea);
    }
    else
    {
        return RepositionToAvoidBlaze(eastFlame, westFlame);
    }

    return false;
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidEyeBlast(
    EyeBlastDangerArea const& dangerArea)
{
    if (!IsPositionInEyeBlastDangerArea(bot->GetPosition(), dangerArea))
        return false;

    float const dx = dangerArea.end.GetPositionX() - dangerArea.start.GetPositionX();
    float const dy = dangerArea.end.GetPositionY() - dangerArea.start.GetPositionY();
    float const length = dangerArea.start.GetExactDist2d(dangerArea.end);

    float const px = bot->GetPositionX();
    float const py = bot->GetPositionY();
    float const sx = dangerArea.start.GetPositionX();
    float const sy = dangerArea.start.GetPositionY();

    float const projection = std::clamp(
        ((px - sx) * dx + (py - sy) * dy) / (length * length), 0.0f, 1.0f);
    float const closestX = sx + projection * dx;
    float const closestY = sy + projection * dy;

    float const distToLine = bot->GetExactDist2d(closestX, closestY);
    float const moveDist = (dangerArea.width - distToLine) + 0.5f;
    if (moveDist <= 0.0f)
        return false;

    float const rawDirX = px - closestX;
    float const rawDirY = py - closestY;
    float const rawDirLength = std::sqrt(rawDirX * rawDirX + rawDirY * rawDirY);
    float const dirX = rawDirLength == 0.0f ? -(dy / length) : rawDirX / rawDirLength;
    float const dirY = rawDirLength == 0.0f ? dx / length : rawDirY / rawDirLength;

    float const safeX = px + dirX * moveDist;
    float const safeY = py + dirY * moveDist;
    float const safeZ = bot->GetPositionZ();
    Position const safePosition(safeX, safeY, safeZ);

    constexpr float minGrateDistance = 10.0f;
    bool const tooCloseToNorthGrate =
        safePosition.GetExactDist2d(ILLIDAN_N_GRATE_POSITION) < minGrateDistance;
    bool const tooCloseToEastGrate =
        safePosition.GetExactDist2d(ILLIDAN_E_GRATE_POSITION) < minGrateDistance;
    bool const tooCloseToWestGrate =
        safePosition.GetExactDist2d(ILLIDAN_W_GRATE_POSITION) < minGrateDistance;

    if (tooCloseToNorthGrate || tooCloseToEastGrate || tooCloseToWestGrate)
        return false;

    return MoveTo(BT_MAP_ID, safeX, safeY, safeZ, false, false, false,
                  false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidBlaze(
    Unit* eastFlame, Unit* westFlame)
{
    decltype(E_GLAIVE_TANK_POSITIONS)* waypoints = nullptr;
    Unit* flame = nullptr;

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        if (!eastFlame || eastFlame->GetVictim() != bot ||
            !bot->IsWithinMeleeRange(eastFlame))
        {
            return false;
        }
        waypoints = &E_GLAIVE_TANK_POSITIONS;
        flame = eastFlame;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        if (!westFlame || westFlame->GetVictim() != bot ||
            !bot->IsWithinMeleeRange(westFlame))
        {
            return false;
        }
        waypoints = &W_GLAIVE_TANK_POSITIONS;
        flame = westFlame;
    }

    if (!waypoints || !flame)
        return false;

    size_t& waypointIndex = flameTankWaypointIndex[bot->GetGUID()];

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible triggers")->Get();

    bool blazeNearby = false;
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == Id(BtNpcs::NPC_BLAZE) &&
            bot->GetDistance2d(unit) <= 8.0f)
        {
            blazeNearby = true;
            break;
        }
    }

    constexpr float arrivalDist = 0.2f;
    if (blazeNearby && bot->GetExactDist2d((*waypoints)[waypointIndex]) <= arrivalDist)
        waypointIndex = (waypointIndex + 1) % waypoints->size();

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, (*waypoints)[waypointIndex], arrivalDist, flame, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Pets grab aggro right away during Phase 2 and wipe the raid if not put on passive
// Just like players, pets cannot melee Illidan during Phase 4
bool IllidanStormrageControlPetAggressionAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    int const phase = GetIllidanPhase(illidan);

    if ((phase == 2 || phase == 4) &&
        pet->GetReactState() != REACT_PASSIVE)
    {
        pet->AttackStop();
        pet->SetReactState(REACT_PASSIVE);
    }
    else if (pet->GetReactState() == REACT_PASSIVE)
    {
        pet->SetReactState(REACT_DEFENSIVE);
    }

    return false;
}

bool IllidanStormragePositionAboveGrateAction::Execute(Event /*event*/)
{
    auto const& gratePositions = GRATE_POSITIONS;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> bots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && GET_PLAYERBOT_AI(member) &&
            !PlayerbotAI::IsAssistTankOfIndex(member, 0, true) &&
            !PlayerbotAI::IsAssistTankOfIndex(member, 1, true))
        {
            bots.push_back(member);
        }
    }

    if (bots.empty())
        return false;

    std::sort(bots.begin(), bots.end(),
        [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    auto const it = std::find(bots.begin(), bots.end(), bot);
    if (it == bots.end())
        return false;

    size_t const botIndex = std::distance(bots.begin(), it);
    uint8 const index = botIndex % gratePositions.size();

    Position const& position = gratePositions[index];
    if (bot->GetExactDist2d(position) > 0.2f)
    {
        return MoveTo(BT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IllidanStormrageRemoveDarkBarrageAction::Execute(Event /*event*/)
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    return spellId && botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

bool IllidanStormrageMoveAwayFromLandingPointAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    constexpr float safeDistance = 20.0f;
    float const currentDistance = bot->GetExactDist2d(illidan);
    if (currentDistance < safeDistance)
        return MoveAway(illidan, safeDistance - currentDistance);

    return false;
}

// NOTE: Illidan's bounding radius is 0.459f, and combatreach is 7.5f
bool IllidanStormrageDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int const phase = GetIllidanPhase(illidan);

    if (phase == 4)
    {
        return SpreadInCircleInDemonPhase(illidan, group);
    }
    else if (GetBotWithParasiticShadowfiend(botAI) == bot ||
             (GetIllidanTrapperHunter(bot) == bot &&
              GetBotWithParasiticShadowfiend(botAI)))
    {
        return false;
    }
    else
    {
        return FanOutBehindInHumanPhase(illidan, group);
    }
}

bool IllidanStormrageDisperseRangedAction::FanOutBehindInHumanPhase(
    Unit* illidan, Group* group)
{
    if (!illidan)
        return false;

    std::vector<Unit*> const flameCrashes = GetAllFlameCrashes(bot);

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    constexpr float arcSpan = M_PI;
    float const arcCenter = illidan->GetOrientation() + M_PI;
    float const arcStart = arcCenter - arcSpan / 2.0f;

    float const radius = PlayerbotAI::IsHeal(bot) ? 18.0f : 25.0f;
    std::vector<Player*> const& bots = PlayerbotAI::IsHeal(bot) ? healers : rangedDps;
    size_t const count = bots.size();
    auto const findIt = std::find(bots.begin(), bots.end(), bot);
    size_t const botIndex = (findIt != bots.end()) ?
        std::distance(bots.begin(), findIt) : 0;

    float const angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) /
         static_cast<float>(count - 1));

    float const targetX = illidan->GetPositionX() + radius * std::cos(angle);
    float const targetY = illidan->GetPositionY() + radius * std::sin(angle);

    constexpr float hazardRadius = 12.0f;
    bool safe = true;
    for (Unit* flameCrash : flameCrashes)
    {
        if (flameCrash->GetDistance2d(targetX, targetY) < hazardRadius)
        {
            safe = false;
            break;
        }
    }

    if (!safe)
        return false;

    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        return MoveTo(BT_MAP_ID, targetX, targetY, bot->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                      true, false);
    }

    return false;
}

bool IllidanStormrageDisperseRangedAction::SpreadInCircleInDemonPhase(
    Unit* illidan, Group* group)
{
    if (!illidan)
        return false;

    Player* warlockTank = GetIllidanWarlockTank(botAI);
    if (!warlockTank)
    {
        constexpr float safeDistFromBoss = 24.0f;
        if (bot->GetExactDist2d(illidan) < safeDistFromBoss)
        {
            constexpr uint32 minInterval = 0;
            if (FleePosition(illidan->GetPosition(), safeDistFromBoss, minInterval))
                return true;
        }

        constexpr float safeDistFromPlayer = 6.0f;
        if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);

        return false;
    }

    std::vector<Player*> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        rangedBots.push_back(member);
    }

    if (rangedBots.empty())
        return false;

    size_t const count = rangedBots.size();
    auto const findIt = std::find(rangedBots.begin(), rangedBots.end(), bot);
    size_t const botIndex = (findIt != rangedBots.end()) ?
        std::distance(rangedBots.begin(), findIt) : 0;

    float const dx = warlockTank->GetPositionX() - illidan->GetPositionX();
    float const dy = warlockTank->GetPositionY() - illidan->GetPositionY();
    float const warlockAngle = std::atan2(dy, dx);

    constexpr float forbiddenArc = (2.0f / 3.0f) * M_PI;
    constexpr float allowedArc = (4.0f / 3.0f) * M_PI;

    float const arcStart = Position::NormalizeOrientation(warlockAngle + forbiddenArc / 2.0f);
    constexpr float radius = 25.0f;

    float const angle = (count == 1) ?
        Position::NormalizeOrientation(arcStart + allowedArc / 2.0f) :
            Position::NormalizeOrientation(
                arcStart + allowedArc * static_cast<float>(botIndex) /
                static_cast<float>(count - 1));

    float const targetX = illidan->GetPositionX() + radius * std::cos(angle);
    float const targetY = illidan->GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        if (MoveTo(BT_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_COMBAT, true, false))
        {
            return true;
        }
        else
        {
            constexpr float safeDistFromTank = 25.0f;
            float const currentDistFromTank = bot->GetExactDist2d(warlockTank);
            if (currentDistFromTank < safeDistFromTank)
                return MoveAway(warlockTank, safeDistFromTank - currentDistFromTank);
        }
    }

    return false;
}

// Melee cannot attack Demon Form Illidan
bool IllidanStormrageMeleeGoSomewhereToNotDieAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    constexpr float demonSearchRadius = 25.0f;
    constexpr float shadowfiendSearchRadius = 15.0f;

    Unit* illidanVictim = illidan->GetVictim();
    // But they can attack Shadow Demons and Shadowfiends, if far enough from Illidan
    Unit* shadowDemon = bot->FindNearestCreature(
        Id(BtNpcs::NPC_SHADOW_DEMON), demonSearchRadius, true);

    if (shadowDemon && shadowDemon->GetDistance2d(illidan) > 15.0f &&
        (!illidanVictim || shadowDemon->GetDistance2d(illidanVictim) > 24.0f))
    {
        return false;
    }
    else
    {
        Unit* shadowfiend = bot->FindNearestCreature(
            Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND),
            shadowfiendSearchRadius, true);

        if (shadowfiend && shadowfiend->GetDistance2d(illidan) > 15.0f &&
            shadowfiend->GetHealthPct() < 30.0f &&
            (!illidanVictim || shadowfiend->GetDistance2d(illidanVictim) > 24.0f))
        {
            return false;
        }
    }

    // 30y is closer than ideal but is a compromise to allow melee to reach targets in time
    constexpr float safeDistFromBoss = 30.0f;
    float const currentDistFromBoss = bot->GetExactDist2d(illidan);
    if (currentDistFromBoss < safeDistFromBoss)
        MoveAway(illidan, safeDistFromBoss - currentDistFromBoss);

    if (Player* warlockTank = GetIllidanWarlockTank(botAI))
    {
        constexpr float safeDistFromTank = 25.0f;
        float const currentDistFromTank = bot->GetExactDist2d(warlockTank);
        if (currentDistFromTank < safeDistFromTank)
            MoveAway(warlockTank, safeDistFromTank - currentDistFromTank);
    }

    constexpr float safeDistFromPlayer = 6.0f;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        MoveAway(nearestPlayer, safeDistFromPlayer - bot->GetDistance2d(nearestPlayer));

    return true;
}

bool IllidanStormrageWarlockTankHandleDemonBossAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    constexpr float safeDistance = 24.0f;
    float const currentDistance = bot->GetExactDist2d(illidan);
    if (currentDistance < safeDistance &&
        MoveAway(illidan, safeDistance - currentDistance))
    {
        return true;
    }

    if (botAI->CanCastSpell("shadow ward", bot) &&
        botAI->CastSpell("shadow ward", bot))
    {
        return true;
    }

    if (botAI->CanCastSpell("searing pain", illidan))
        return botAI->CastSpell("searing pain", illidan);

    return false;
}

bool IllidanStormrageDpsPrioritizeAddsAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int const phase = GetIllidanPhase(illidan);

    std::vector<Unit*> targets;

    if (phase == 4)
    {
        constexpr float searchRadius = 35.0f;

        Unit* shadowDemon = bot->FindNearestCreature(
            Id(BtNpcs::NPC_SHADOW_DEMON), searchRadius, true);

        if (GetIllidanWarlockTank(botAI) == bot)
        {
            targets = { shadowDemon, illidan };
        }
        else
        {
            Unit* shadowfiend = bot->FindNearestCreature(
                Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND),
                searchRadius, true);

            if (PlayerbotAI::IsRanged(bot))
            {
                if (shadowDemon)
                    targets = { shadowDemon };
                else if (shadowfiend && bot->GetDistance2d(shadowfiend) > 10.0f)
                    targets = { shadowfiend };
                else
                    targets = { illidan };
            }
            else if (PlayerbotAI::IsMelee(bot))
            {
                targets = { shadowDemon, shadowfiend };
            }
        }
    }
    else if (PlayerbotAI::IsRanged(bot))
    {
        if (phase == 1 || phase == 3 || phase == 5)
        {
            constexpr float searchRadius = 35.0f;
            Unit* shadowfiend = bot->FindNearestCreature(
                Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND),
                searchRadius, true);

            if (shadowfiend && bot->GetDistance2d(shadowfiend) > 10.0f)
                targets = { shadowfiend };
            else
                targets = { illidan };
        }
        else if (phase == 2)
        {
            constexpr float searchRadius = 20.0f;
            Unit* shadowfiend = bot->FindNearestCreature(
                Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND),
                searchRadius, true);

            if (shadowfiend && bot->GetDistance2d(shadowfiend) > 5.0f)
            {
                targets = { shadowfiend };
            }
            else
            {
                auto const [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
                targets = { eastFlame, westFlame };
            }
        }
    }

    for (Unit* candidate : targets)
    {
        if (candidate && candidate->IsAlive())
        {
            if (AI_VALUE(Unit*, "current target") != candidate)
                return Attack(candidate);

            return false;
        }
    }

    return false;
}

bool IllidanStormrageUseShadowTrapAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    GameObject* trap = FindNearestTrap(botAI);
    if (!trap || illidan->GetExactDist2d(trap) >= 4.0f)
        return false;

    if (bot->GetExactDist2d(trap) < 3.0f)
    {
        trap->Use(bot);
        return true;
    }
    else
    {
        return MoveTo(BT_MAP_ID, trap->GetPositionX(), trap->GetPositionY(),
                      trap->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IllidanStormrageManageDpsTimerAndRtiAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    uint32 const now = getMSTime();
    uint32 const instanceId = illidan->GetMap()->GetInstanceId();

    bool updated = false;
    int const phase = GetIllidanPhase(illidan);
    int lastPhase = -1;
    if (auto const it = illidanLastPhase.find(instanceId); it != illidanLastPhase.end())
        lastPhase = it->second;
    bool const phaseChanged = lastPhase != phase;
    illidanLastPhase[instanceId] = phase;

    if (phaseChanged)
    {
        if (phase == 1 || phase == 3 || phase == 4 || phase == 5)
        {
            illidanBossDpsWaitTimer[instanceId] = now;
            updated = true;
        }
        else if (phase == 2)
        {
            if (illidanBossDpsWaitTimer.erase(instanceId) > 0)
                updated = true;
        }

        if (phase != 2 && illidanFlameDpsWaitTimer.erase(instanceId) > 0)
            updated = true;
    }

    if (phase == 2)
    {
        if (eastFlameGuid.find(instanceId) == eastFlameGuid.end() &&
            westFlameGuid.find(instanceId) == westFlameGuid.end())
        {
            std::list<Creature*> creatureList;
            constexpr float searchRadius = 50.0f;
            illidan->GetCreatureListWithEntryInGrid(
                creatureList, Id(BtNpcs::NPC_FLAME_OF_AZZINOTH), searchRadius);

            std::vector<Creature*> flames;
            for (Creature* creature : creatureList)
            {
                if (creature && creature->IsAlive())
                    flames.push_back(creature);
            }

            if (flames.size() == 2)
            {
                float const eastDist0 =
                    flames[0]->GetExactDist2d(ILLIDAN_E_GLAIVE_WAITING_POSITION);
                float const eastDist1 =
                    flames[1]->GetExactDist2d(ILLIDAN_E_GLAIVE_WAITING_POSITION);

                if (eastDist0 < eastDist1)
                {
                    eastFlameGuid[instanceId] = flames[0]->GetGUID();
                    westFlameGuid[instanceId] = flames[1]->GetGUID();
                }
                else
                {
                    eastFlameGuid[instanceId] = flames[1]->GetGUID();
                    westFlameGuid[instanceId] = flames[0]->GetGUID();
                }

                illidanFlameDpsWaitTimer[instanceId] = now;

                updated = true;
            }
        }
    }
    else
    {
        if (eastFlameGuid.erase(instanceId) > 0)
            updated = true;
        if (westFlameGuid.erase(instanceId) > 0)
            updated = true;
    }

    return updated;
}

bool IllidanStormrageDestroyHazardsAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int const phase = GetIllidanPhase(illidan);
    constexpr float searchRadius = 50.0f;
    std::list<Creature*> hazards;
    std::vector<uint32> entries;

    if (phase == 2 || phase == 4)
        entries = { Id(BtNpcs::NPC_FLAME_CRASH) };
    else if (phase == 0)
        entries = { Id(BtNpcs::NPC_DEMON_FIRE), Id(BtNpcs::NPC_BLAZE) };

    if (!entries.empty())
        bot->GetCreatureListWithEntryInGrid(hazards, entries, searchRadius);

    for (Creature* creature : hazards)
    {
        if (creature && creature->IsAlive())
        {
            creature->Kill(bot, creature);
            return true;
        }
    }

    return false;
}

// Reduce Shadow Demon to 25% health and kill residual Shadowfiends in Phase 2
bool IllidanStormrageHandleAddsCheatAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (GetIllidanPhase(illidan) == 2)
    {
        constexpr float searchRadius = 20.0f;
        if (Unit* shadowfiend = bot->FindNearestCreature(
                Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND),
                searchRadius, true))
        {
            shadowfiend->Kill(bot, shadowfiend);
            return true;
        }
    }
    else
    {
        constexpr float searchRadius = 75.0f;
        Unit* shadowDemon = bot->FindNearestCreature(
            Id(BtNpcs::NPC_SHADOW_DEMON), searchRadius, true);

        if (shadowDemon && shadowDemon->GetHealthPct() > 25.0f)
        {
            uint32 desiredDamage = 0;
            uint32 const quarterHealth = shadowDemon->GetMaxHealth() / 4;
            if (shadowDemon->GetHealth() > quarterHealth)
                desiredDamage = shadowDemon->GetHealth() - quarterHealth;

            Unit::DealDamage(bot, shadowDemon, desiredDamage, nullptr, DIRECT_DAMAGE,
                             SPELL_SCHOOL_MASK_NORMAL, nullptr, false, false, nullptr);
            return true;
        }
    }

    return false;
}
