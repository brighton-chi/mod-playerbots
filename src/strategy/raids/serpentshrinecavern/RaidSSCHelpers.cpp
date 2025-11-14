#include "RaidSSCHelpers.h"
#include "RaidSSCActions.h"
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

    std::unordered_map<uint32, time_t> leotherasHumanFormDpsWaitTimer;
    std::unordered_map<uint32, time_t> leotherasDemonFormDpsWaitTimer;
    std::unordered_map<uint32, time_t> leotherasFinalPhaseDpsWaitTimer;

    std::unordered_map<uint32, time_t> karathressDpsWaitTimer;

    std::unordered_map<ObjectGuid, uint8> tidewalkerTankStep;
    std::unordered_map<ObjectGuid, uint8> tidewalkerRangedStep;

    std::unordered_map<ObjectGuid, Position> vashjRangedPositions;
    std::unordered_map<ObjectGuid, bool> vashjHasReachedRangedPosition;

    namespace SerpentShrineCavernPositions
    {
        const Position HydrossFrostTankPosition(-236.669f, -358.352f, -0.828f);
        const Position HydrossNatureTankPosition(-225.471f, -327.790f, -3.682f);
        // Hydross dps positions?

        const Position LurkerMainTankPosition(23.706f, -406.038f, -19.686f);
        const Position LurkerEMeleePosition(36.815f, -432.585f, -19.339f);
        const Position LurkerWMeleePosition(47.721f, -406.668f, -19.635f);
        // const Position LurkerNWIsletPosition(77.937f, -384.500f, -19.722f);
        // const Position LurkerNWIsletSwimPosition(73.743f, -388.018f, -21.397f);
        const Position LurkerNWRangedDpsPosition(58.372f, -393.667f, -19.526f);
        // const Position LurkerNEIsletPosition(63.022f, -456.310f, -19.793f);
        // const Position LurkerNEIsletSwimPosition(60.338f, -451.668f, -21.451f);
        const Position LurkerNERangedDpsPosition(60.666f, -440.899f, -19.575f);
        // const Position LurkerEIsletPosition(14.283f, -457.467f, -19.793f);
        // const Position LurkerEIsletSwimPosition(17.191f, -452.586f, -21.436f);
        const Position LurkerERangedDpsPosition(38.457f, -417.324f, -18.917f);
        const Position LurkerSEHealerLandPosition(16.237f, -438.098f, -19.551f);
        // const Position LurkerSEHealerSwimPosition(14.637f, -440.707f, -21.459f);
        const Position LurkerSWHealerLandPosition(37.255f, -387.031f, -19.417f);
        // const Position LurkerSWHealerSwimPosition(37.505f, -382.907f, -21.397f);
        const Position LurkerNHealerLandPosition(66.268f, -418.774f, -19.592f);
        // const Position LurkerNHealerSwimPosition(71.255f, -419.223f, -21.381f);

        // const Position KarathressTankPosition(472.973f, -540.804f, -7.548f);
        const Position KarathressTankPosition(474.403f, -531.118f, -7.548f); // above works but maybe LoS issues, trying this one
        const Position TidalvessTankPosition(511.282f, -501.162f, -13.158f);
        const Position SharkkisTankPosition(508.057f, -541.109f, -10.133f);
        const Position CaribdisTankPosition(464.462f, -475.820f, -13.158f);
        const Position CaribdisHealerPosition(466.203f, -503.201f, -13.158f);
        const Position CaribdisRangedDpsPosition(463.197f, -501.190f, -13.158f);

        const Position TidewalkerPhase1TankPosition(410.925f, -741.916f, -7.146f);
        const Position TidewalkerPhaseTransitionWaypoint(407.035f, -759.479f, -7.168f);
        const Position TidewalkerPhase2TankPosition(446.571f, -767.155f, -7.144f);
        const Position TidewalkerPhase2RangedPosition(432.595f, -766.288f, -7.145f);

        const Position VashjPlatformCenterPosition(29.634f, -923.541f, 42.985f);
        const Position VashjNWStairsPosition(65.087f, -878.344f, 41.097f);
        const Position VashjWStairsPosition(29.693f, -865.188f, 41.097f);
        const Position VashjSWStairsPosition(9.766f, -869.707f, 41.097f);
        const Position VashjSSWStairsPosition(-25.352f, -910.754f, 41.097f);
        const Position VashjSEStairsPosition(-9.504f, -964.514f, 41.097f);
        const Position VashjEStairsPosition(29.701f, -982.523f, 41.097f);
        const Position VashjENEStairsPosition(42.143f, -978.813f, 41.097f);
        const Position VashjNNEStairsPosition(83.647f, -941.901f, 41.097f);
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

    bool IsMapIDTimerManager(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && !botAI->IsTank(member) && GET_PLAYERBOT_AI(member))
                    return member == bot;
            }
        }

        return true;
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

    bool HasMarkOfCorruptionAt100Percent(Player* bot)
    {
        return bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_500);
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
        LOG_DEBUG("playerbots", "IsLurkerCastingSpout: spellId={}, isSpout={}", spellId, isSpout);
        return isSpout;
    }

    std::array<std::vector<Player*>, 3> GetRangedDpsBotGroups(Group* group)
    {
        std::array<std::vector<Player*>, 3> groups;
        if (!group)
            return groups;

        int idx = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(member);
            if (!botAI || !botAI->IsRanged(member) || !botAI->IsDps(member))
                continue;

            groups[idx % 3].push_back(member);
            idx++;
        }

        return groups;
    }

    std::array<std::vector<Player*>, 3> GetHealerBotGroups(Group* group)
    {
        std::array<std::vector<Player*>, 3> groups;
        if (!group)
            return groups;

        int idx = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(member);
            if (!botAI || !botAI->IsHeal(member))
                continue;

            groups[idx % 3].push_back(member);
            idx++;
        }

        return groups;
    }

    std::array<std::vector<Player*>, 2> GetMeleeBotGroups(Group* group)
    {
        std::array<std::vector<Player*>, 2> groups;
        if (!group)
            return groups;

        int idx = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(member);
            if (!botAI || !botAI->IsMelee(member) || botAI->IsMainTank(member))
                continue;

            groups[idx % 2].push_back(member);
            idx++;
        }

        return groups;
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

    Player* GetDesignatedCoreLooter(Group* group, Player* master, PlayerbotAI* botAI)
    {
        // If cheats are off, use the real master.
        if (!botAI->HasCheat(BotCheatMask::raid))
            return master;

        // Prefer a melee DPS bot; fall back to a ranged DPS bot; if none found return nullptr.
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
            if (!memberAI) // skip humans
                continue;

            if (memberAI->IsRangedDpsAssistantOfIndex(member, 0))
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

            if (memberAI->IsHealAssistantOfIndex(member, 0))
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

            if (memberAI->IsHealAssistantOfIndex(member, 1))
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

    const std::vector<uint32> SHIELD_GENERATOR_DB_GUIDS = { 47482, 47483, 47484, 47485 }; // NW, NE, SE, SW
    // Entries: 185052 { 52.048f, -901.236f, 44.000f }, 185054 { 52.448f, -944.825f, 44.000f },
    // 185051 { 7.81f, -945.244f, 44.000f }, 185053 { 7.417f, -901.109f, 44.000f }, respectively

    // Get the positions of all Shield Generators by their database GUIDs
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

            GeneratorInfo info;
            info.guid = go->GetGUID();
            info.x = go->GetPositionX();
            info.y = go->GetPositionY();
            info.z = go->GetPositionZ();
            generators.push_back(info);
        }

        return generators;
    }

    // Returns the nearest active shield generator trigger (NPC_WORLD_INVISIBLE_TRIGGER) to the bot
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
