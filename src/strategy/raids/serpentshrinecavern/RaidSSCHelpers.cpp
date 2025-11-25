#include "RaidSSCHelpers.h"
#include "AiFactory.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace SerpentShrineCavernHelpers
{
    std::unordered_map<uint32, time_t> hydrossFrostDpsWaitTimer;
    std::unordered_map<uint32, time_t> hydrossNatureDpsWaitTimer;
    std::unordered_map<uint32, time_t> hydrossChangeToFrostPhaseTimer;
    std::unordered_map<uint32, time_t> hydrossChangeToNaturePhaseTimer;

    std::unordered_map<uint32, time_t> lurkerSpoutTimer;
    std::unordered_map<ObjectGuid, Position> lurkerRangedPositions;

    std::unordered_map<uint32, time_t> leotherasHumanFormDpsWaitTimer;
    std::unordered_map<uint32, time_t> leotherasDemonFormDpsWaitTimer;
    std::unordered_map<uint32, time_t> leotherasFinalPhaseDpsWaitTimer;

    std::unordered_map<uint32, time_t> karathressDpsWaitTimer;

    std::unordered_map<ObjectGuid, uint8> tidewalkerTankStep;
    std::unordered_map<ObjectGuid, uint8> tidewalkerRangedStep;

    std::unordered_map<ObjectGuid, Position> vashjRangedPositions;
    std::unordered_map<ObjectGuid, bool> vashjHasReachedRangedPosition;
    std::unordered_map<ObjectGuid, Position> intendedLineup;
    std::unordered_map<ObjectGuid, time_t> lastImbueAttempt;
    std::unordered_map<uint32, time_t> lastParalyzeTime;

    namespace SerpentShrineCavernPositions
    {
        const Position HydrossFrostTankPosition = { -236.669f, -358.352f, -0.828f };
        const Position HydrossNatureTankPosition = { -225.471f, -327.790f, -3.682f };

        const Position LurkerMainTankPosition = { 23.706f, -406.038f, -19.686f };

        const Position KarathressTankPosition = { 474.403f, -531.118f, -7.548f };
        const Position TidalvessTankPosition = { 511.282f, -501.162f, -13.158f };
        const Position SharkkisTankPosition = { 508.057f, -541.109f, -10.133f };
        const Position CaribdisTankPosition = { 464.462f, -475.820f, -13.158f };
        const Position CaribdisHealerPosition = { 466.203f, -503.201f, -13.158f };
        const Position CaribdisRangedDpsPosition = { 463.197f, -501.190f, -13.158f };

        const Position TidewalkerPhase1TankPosition = { 410.925f, -741.916f, -7.146f };
        const Position TidewalkerPhaseTransitionWaypoint = { 407.035f, -759.479f, -7.168f };
        const Position TidewalkerPhase2TankPosition = { 446.571f, -767.155f, -7.144f };
        const Position TidewalkerPhase2RangedPosition = { 432.595f, -766.288f, -7.145f };

        const Position VashjPlatformCenterPosition = { 29.634f, -923.541f, 42.985f };
    }

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
    {
        if (!target)
            return;

        if (Group* group = bot->GetGroup())
        {
            ObjectGuid currentGuid = group->GetTargetIcon(iconId);
            if (currentGuid != target->GetGUID())
                group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
        }
    }

    void MarkTargetWithSkull(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::skullIndex);
    }

    void MarkTargetWithSquare(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
    }

    void MarkTargetWithStar(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
    }

    void MarkTargetWithCircle(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::circleIndex);
    }

    void MarkTargetWithDiamond(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::diamondIndex);
    }

    void MarkTargetWithTriangle(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::triangleIndex);
    }

    void MarkTargetWithCross(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::crossIndex);
    }

    void MarkTargetWithMoon(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::moonIndex);
    }

    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target)
    {
        if (!target)
            return;

        std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();

        if (currentRti != rtiName || currentTarget != target)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
        }
    }

    // Dps bot selected for marking and managing timers and trackers
    bool IsMapIDTimerManager(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && botAI->IsDps(member) && GET_PLAYERBOT_AI(member))
                    return member == bot;
            }
        }

        return false;
    }

    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry)
    {
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
                return unit;
        }

        return nullptr;
    }

    bool HasMarkOfHydrossAt100Percent(Player* bot)
    {
        return bot->HasAura(SPELL_MARK_OF_HYDROSS_100) ||
               bot->HasAura(SPELL_MARK_OF_HYDROSS_250) ||
               bot->HasAura(SPELL_MARK_OF_HYDROSS_500);
    }

    bool HasNoMarkOfHydross(Player* bot)
    {
        return !bot->HasAura(SPELL_MARK_OF_HYDROSS_10) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_25) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_50) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_100) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_250) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_500);
    }

    bool HasMarkOfCorruptionAt100Percent(Player* bot)
    {
        return bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_500);
    }

    bool HasNoMarkOfCorruption(Player* bot)
    {
        return
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_10) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_25) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_50) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_500);
    }

    bool IsLurkerCastingSpout(Unit* lurker)
    {
        if (!lurker || !lurker->HasUnitState(UNIT_STATE_CASTING))
            return false;

        Spell* currentSpell = lurker->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!currentSpell)
            return false;

        uint32 spellId = currentSpell->m_spellInfo->Id;
        bool isSpout = spellId == SPELL_SPOUT_VISUAL;

        return isSpout;
    }

    Unit* GetLeotherasHuman(PlayerbotAI* botAI)
    {
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (auto const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_LEOTHERAS_THE_BLIND &&
                unit->IsInCombat() && !unit->HasAura(SPELL_METAMORPHOSIS))
                return unit;
        }
        return nullptr;
    }

    Unit* GetPhase2LeotherasDemon(PlayerbotAI* botAI)
    {
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (auto const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_LEOTHERAS_THE_BLIND &&
                unit->HasAura(SPELL_METAMORPHOSIS))
                return unit;
        }
        return nullptr;
    }

    Unit* GetPhase3LeotherasDemon(PlayerbotAI* botAI)
    {
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (auto const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SHADOW_OF_LEOTHERAS)
                return unit;
        }
        return nullptr;
    }

    Unit* GetActiveLeotherasDemon(PlayerbotAI* botAI)
    {
        Unit* phase2 = GetPhase2LeotherasDemon(botAI);
        Unit* phase3 = GetPhase3LeotherasDemon(botAI);
        return phase2 ? phase2 : phase3;
    }

    Player* GetLeotherasDemonFormTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        Player* mainTankCandidate = nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            if (member->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(member)->HasStrategy("tank", BotState::BOT_STATE_COMBAT))
                return member;

            if (!mainTankCandidate && GET_PLAYERBOT_AI(member)->IsMainTank(member))
                mainTankCandidate = member;
        }

        return mainTankCandidate;
    }

    bool IsMainTankInSameSubgroup(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group || !group->isRaidGroup())
            return false;

        uint8 botSubGroup = group->GetMemberGroup(bot->GetGUID());
        if (botSubGroup >= MAX_RAID_SUBGROUPS)
            return false;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;

            if (group->GetMemberGroup(member->GetGUID()) != botSubGroup)
                continue;

            if (PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member))
            {
                if (memberAI->IsMainTank(member))
                    return true;
            }
        }

        return false;
    }

    bool IsLadyVashjInPhase1(PlayerbotAI* botAI)
    {
        Unit* vashj = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();
        if (!vashj)
            return false;

        Creature* vashjCreature = vashj->ToCreature();
        return vashjCreature && vashjCreature->GetHealthPct() > 70.0f && vashjCreature->GetReactState() != REACT_PASSIVE;
    }

    bool IsLadyVashjInPhase2(PlayerbotAI* botAI)
    {
        Unit* vashj = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();
        if (!vashj)
            return false;

        Creature* vashjCreature = vashj->ToCreature();
        return vashjCreature && vashjCreature->GetReactState() == REACT_PASSIVE;
    }

    bool IsLadyVashjInPhase3(PlayerbotAI* botAI)
    {
        Unit* vashj = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();
        if (!vashj)
            return false;

        Creature* vashjCreature = vashj->ToCreature();
        return vashjCreature && vashjCreature->GetHealthPct() <= 50.0f && vashjCreature->GetReactState() != REACT_PASSIVE;
    }

    bool IsValidPhase2CombatNpc(Unit* unit, PlayerbotAI* botAI)
    {
        if (!unit || !unit->IsAlive())
            return false;

        uint32 entry = unit->GetEntry();

        if (IsLadyVashjInPhase2(botAI))
        {
            return entry == NPC_TAINTED_ELEMENTAL || entry == NPC_ENCHANTED_ELEMENTAL ||
                   entry == NPC_COILFANG_ELITE || entry == NPC_COILFANG_STRIDER;
        }
        else if (IsLadyVashjInPhase3(botAI))
        {
            return entry == NPC_TAINTED_ELEMENTAL || entry == NPC_ENCHANTED_ELEMENTAL ||
                   entry == NPC_COILFANG_ELITE || entry == NPC_COILFANG_STRIDER ||
                   entry == NPC_TOXIC_SPOREBAT || entry == NPC_LADY_VASHJ;
        }

        return false;
    }

    bool AnyRecentParalyze(Group* group, uint32 mapId, uint32 graceSeconds)
    {
        const time_t now = std::time(nullptr);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member)
                continue;

            if (member->IsAlive() && member->HasAura(SPELL_PARALYZE))
            {
                lastParalyzeTime[mapId] = now;
                return true;
            }
        }

        auto it = lastParalyzeTime.find(mapId);
        if (it != lastParalyzeTime.end())
        {
            if ((now - it->second) <= static_cast<time_t>(graceSeconds))
                return true;
        }

        return false;
    }

    Player* GetDesignatedCoreLooter(Group* group, Player* master, PlayerbotAI* botAI)
    {
        if (!botAI->HasCheat(BotCheatMask::raid))
            return master;

        Player* fallback = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == master)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsMelee(member) && memberAI->IsDps(member))
                return member;

            if (!fallback && memberAI->IsRangedDps(member))
                fallback = member;
        }

        return fallback ? fallback : master;
    }

    Player* GetFirstTaintedCorePasser(Group* group, PlayerbotAI* botAI)
    {
        Player* designatedLooter = GetDesignatedCoreLooter(group, botAI->GetMaster(), botAI);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == designatedLooter)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsHealAssistantOfIndex(member, 0))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                botAI->IsTank(member) || member == designatedLooter)
                continue;
            return member;
        }

        return nullptr;
    }

    Player* GetSecondTaintedCorePasser(Group* group, PlayerbotAI* botAI)
    {
        Player* designatedLooter = GetDesignatedCoreLooter(group, botAI->GetMaster(), botAI);
        Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == designatedLooter ||
                member == firstCorePasser)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsHealAssistantOfIndex(member, 1))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || botAI->IsTank(member) ||
                member == designatedLooter || member == firstCorePasser)
                continue;
            return member;
        }

        return nullptr;
    }

    Player* GetThirdTaintedCorePasser(Group* group, PlayerbotAI* botAI)
    {
        Player* designatedLooter = GetDesignatedCoreLooter(group, botAI->GetMaster(), botAI);
        Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
        Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == designatedLooter ||
                member == firstCorePasser || member == secondCorePasser)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsHealAssistantOfIndex(member, 2))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || botAI->IsTank(member) ||
                member == designatedLooter || member == firstCorePasser || member == secondCorePasser)
                continue;
            return member;
        }

        return nullptr;
    }

    Player* GetFourthTaintedCorePasser(Group* group, PlayerbotAI* botAI)
    {
        Player* designatedLooter = GetDesignatedCoreLooter(group, botAI->GetMaster(), botAI);
        Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
        Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
        Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == designatedLooter ||
                member == firstCorePasser || member == secondCorePasser || member == thirdCorePasser)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsRangedDpsAssistantOfIndex(member, 0))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || botAI->IsTank(member) ||
                member == designatedLooter || member == firstCorePasser || member == secondCorePasser ||
                member == thirdCorePasser)
                continue;
            return member;
        }

        return nullptr;
    }

    const std::vector<uint32> SHIELD_GENERATOR_DB_GUIDS = { 47482, 47483, 47484, 47485 }; // NW, NE, SE, SW

    // Get the positions of all active Shield Generators by their database GUIDs
    std::vector<GeneratorInfo> GetAllGeneratorInfosByDbGuids(Map* map, const std::vector<uint32>& generatorDbGuids)
    {
        std::vector<GeneratorInfo> generators;
        if (!map)
            return generators;

        for (uint32 dbGuid : generatorDbGuids)
        {
            auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(dbGuid);
            if (bounds.first == bounds.second)
                continue;

            GameObject* go = bounds.first->second;
            if (!go)
                continue;

            if (go->GetGoState() != GO_STATE_READY)
                continue;

            GeneratorInfo info;
            info.guid = go->GetGUID();
            info.x = go->GetPositionX();
            info.y = go->GetPositionY();
            info.z = go->GetPositionZ();
            generators.push_back(info);
        }

        return generators;
    }

    // Returns the nearest active Shield Generator to the bot
    // Active generators are powered by NPC_WORLD_INVISIBLE_TRIGGER creatures, which depawn after use
    Unit* GetNearestActiveShieldGeneratorTriggerByEntry(Player* bot, Unit* reference)
    {
        if (!bot || !reference)
            return nullptr;

        Map* map = bot->GetMap();
        if (!map)
            return nullptr;

        std::list<Creature*> triggers;
        float searchRange = 150.0f;
        reference->GetCreatureListWithEntryInGrid(triggers, NPC_WORLD_INVISIBLE_TRIGGER, searchRange);

        Creature* nearest = nullptr;
        float minDist = std::numeric_limits<float>::max();

        for (Creature* creature : triggers)
        {
            if (!creature->IsAlive())
                continue;

            float dist = reference->GetDistance(creature);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = creature;
            }
        }

        return nearest;
    }

    const GeneratorInfo* GetNearestGeneratorToBot(Player* bot, const std::vector<GeneratorInfo>& generators)
    {
        if (!bot || generators.empty())
            return nullptr;

        const GeneratorInfo* nearest = nullptr;
        float minDist = std::numeric_limits<float>::max();

        for (auto const& gen : generators)
        {
            float dist = bot->GetExactDist(gen.x, gen.y, gen.z);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = &gen;
            }
        }

        return nearest;
    }
}
