#include "RaidSSCHelpers.h"
#include "RaidSSCActions.h"
#include "AiFactory.h"
#include "Creature.h"
#include "Group.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace SerpentShrineCavernHelpers
{
    std::unordered_map<uint32, time_t> hydrossFrostDpsWaitTimer;
    std::unordered_map<uint32, time_t> hydrossNatureDpsWaitTimer;
    std::unordered_map<ObjectGuid, time_t> hydrossChangeToFrostPhaseTimer;
    std::unordered_map<ObjectGuid, time_t> hydrossChangeToNaturePhaseTimer;

    std::unordered_map<uint32, time_t> lurkerSpoutTimer;

    std::unordered_map<uint32, time_t> leotherasHumanFormDpsWaitTimer;
    std::unordered_map<uint32, time_t> leotherasDemonFormDpsWaitTimer;
    std::unordered_map<uint32, time_t> leotherasFinalPhaseDpsWaitTimer;

    std::unordered_map<uint32, time_t> karathressDpsWaitTimer;

    std::unordered_map<ObjectGuid, uint8> tidewalkerTankStep;
    std::unordered_map<ObjectGuid, uint8> tidewalkerRangedStep;

    std::unordered_map<ObjectGuid, Position> vashjRangedPositions;
    std::unordered_map<ObjectGuid, bool> vashjHasReachedRangedPosition;

    namespace SerpentShrineCavernLocations
    {
        const Location HydrossFrostTankPosition = { -236.669f, -358.352f, -0.828f };
        const Location HydrossNatureTankPosition = { -225.471f, -327.790f, -3.682f };
        // Hydross dps locations?

        /* const Location LurkerMainTankPosition = { 23.706f, -406.038f, -19.686f };
        const Location LurkerEMeleePosition = { 39.265f, -433.439f, -19.712f };
        const Location LurkerWMeleePosition = { 47.721f, -406.668f, -19.635f };
        const Location LurkerNWIsletPosition = { 77.937f, -384.500f, -19.722f };
        const Location LurkerNWIsletSwimPosition = { 73.743f, -388.018f, -21.297f };
        const Location LurkerNEIsletPosition = { 63.022f, -456.310f, -19.793f };
        const Location LurkerNEIsletSwimPosition = { 60.338f, -451.668f, -21.351f };
        const Location LurkerEIsletPosition = { 14.283f, -457.467f, -19.793f };
        const Location LurkerEIsletSwimPosition = { 17.191f, -452.586f, -21.336f };
        const Location LurkerSEHealerLandPosition = { 16.237f, -438.098f, -19.551f };
        const Location LurkerSEHealerSwimPosition = { 14.637f, -440.707f, -21.359f };
        const Location LurkerSWHealerLandPosition = { 37.255f, -387.031f, -19.417f };
        const Location LurkerSWHealerSwimPosition = { 37.505f, -382.907f, -21.297f };
        const Location LurkerNHealerLandPosition = { 66.268f, -418.774f, -19.592f };
        const Location LurkerNHealerSwimPosition = { 71.255f, -419.223f, -21.281f };*/
        const Location LurkerMainTankPosition = { 23.706f, -406.038f, -19.686f };
        const Location LurkerEMeleePosition = { 39.265f, -433.439f, -19.712f };
        const Location LurkerWMeleePosition = { 47.721f, -406.668f, -19.635f };
        const Location LurkerNWIsletPosition = { 77.937f, -384.500f, -19.722f };
        const Location LurkerNWIsletSwimPosition = { 73.743f, -388.018f, -21.397f };
        const Location LurkerNEIsletPosition = { 63.022f, -456.310f, -19.793f };
        const Location LurkerNEIsletSwimPosition = { 60.338f, -451.668f, -21.451f };
        const Location LurkerEIsletPosition = { 14.283f, -457.467f, -19.793f };
        const Location LurkerEIsletSwimPosition = { 17.191f, -452.586f, -21.436f };
        const Location LurkerSEHealerLandPosition = { 16.237f, -438.098f, -19.551f };
        const Location LurkerSEHealerSwimPosition = { 14.637f, -440.707f, -21.459f };
        const Location LurkerSWHealerLandPosition = { 37.255f, -387.031f, -19.417f };
        const Location LurkerSWHealerSwimPosition = { 37.505f, -382.907f, -21.397f };
        const Location LurkerNHealerLandPosition = { 66.268f, -418.774f, -19.592f };
        const Location LurkerNHealerSwimPosition = { 71.255f, -419.223f, -21.381f };

        const Location KarathressTankPosition = { 472.973f, -540.804f, -7.548f };
        const Location TidalvessTankPosition = { 511.282f, -501.162f, -13.158f };
        // const Location SharkkisTankPosition = { 507.396f, -555.750f, -7.146f }; // OG
        const Location SharkkisTankPosition = { 508.057f, -541.109f, -10.133f };
        const Location CaribdisTankPosition = { 464.462f, -475.820f, -13.158f };
        // const Location CaribdisTankPosition = { 450.472f, -483.398f, -13.158f }; // try this if need farther
        // const Location CaribdisTankPosition = { 464.474f, -467.951f, -13.158f }; // or this for near the corner
        const Location CaribdisHealerPosition = { 466.203f, -503.201f, -13.158f };
        // const Location KarathressRoomCenterPosition = { 502.570f, -510.805f, -13.158f }; // smack dab
        // const Location KarathressRoomCenterPosition = { 494.458f, -510.435f, -13.158f }; // middle of the "more centra" positions

        const Location TidewalkerPhase1TankPosition = { 410.925f, -741.916f, -7.146f };
        const Location TidewalkerPhaseTransitionWaypoint = { 407.035f, -759.479f, -7.168f };
        const Location TidewalkerPhase2TankPosition = { 446.571f, -767.155f, -7.144f };
        const Location TidewalkerPhase2RangedPosition = { 432.595f, -766.288f, -7.145f };
        // const Location TidewalkerGraveHealerPosition = { 388.558f, -723.956f, -11.941f };
        // Tidewalker offtank position(s) for murlocs?

        // Room Center to top of stairs is 56-57 yards ish
        const Location VashjRoomCenterPosition = { 29.634f, -923.541f, 42.985f };
        const Location VashjNWStairsPosition = { 65.087f, -878.344f, 41.097f };
        const Location VashjWStairsPosition = { 29.693f, -865.188f, 41.097f };
        const Location VashjSWStairsPosition = { 9.766f, -869.707f, 41.097f };
        const Location VashjSSWStairsPosition = { -25.352f, -910.754f, 41.097f };
        const Location VashjSEStairsPosition = { -9.504f, -964.514f, 41.097f };
        const Location VashjEStairsPosition = { 29.701f, -982.523f, 41.097f };
        const Location VashjENEStairsPosition = { 42.143f, -978.813f, 41.097f };
        const Location VashjNNEStairsPosition = { 83.647f, -941.901f, 41.097f };
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

    /*
    bool HasAnyMarkOfHydross(Player* bot)
    {
        return bot->HasAura(SPELL_MARK_OF_HYDROSS_10) ||
               bot->HasAura(SPELL_MARK_OF_HYDROSS_25) ||
               bot->HasAura(SPELL_MARK_OF_HYDROSS_50) ||
               bot->HasAura(SPELL_MARK_OF_HYDROSS_100) ||
               bot->HasAura(SPELL_MARK_OF_HYDROSS_250) ||
               bot->HasAura(SPELL_MARK_OF_HYDROSS_500);
    }

    bool HasAnyMarkOfCorruption(Player* bot)
    {
        return bot->HasAura(SPELL_MARK_OF_CORRUPTION_10) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_25) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_50) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_500);
    } */

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

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            if (member->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(member)->HasStrategy("tank", BotState::BOT_STATE_COMBAT))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            if (GET_PLAYERBOT_AI(member)->IsMainTank(member))
                return member;
        }

        return nullptr;
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

        return vashj && vashj->GetHealthPct() > 70.0f;
    }

    bool IsLadyVashjInPhase2(PlayerbotAI* botAI)
    {
        Unit* vashj = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();

        return vashj && vashj->GetHealthPct() <= 70.0f && vashj->GetHealthPct() > 50.0f;
    }

    bool IsLadyVashjInPhase3(PlayerbotAI* botAI)
    {
        Unit* vashj = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();

        return vashj && vashj->GetHealthPct() <= 50.0f;
    }

    bool IsMeleeRTIMarker(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && botAI->IsMelee(member) &&
                    botAI->IsDps(member) && GET_PLAYERBOT_AI(member))
                    return member == bot;
            }
        }

        return true;
    }

    bool IsRangedRTIMarker(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            std::vector<Player*> assignedRanged = GetPhase2AssignedRangedDpsBots(group, botAI);
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && botAI->IsRangedDps(member) &&
                    GET_PLAYERBOT_AI(member) && !botAI->IsRangedDpsAssistantOfIndex(member, 0) &&
                    !botAI->IsRangedDpsAssistantOfIndex(member, 1))
                    return member == bot;
            }
        }

        return true;
    }

    std::vector<Player*> GetPhase2AssignedRangedDpsBots(Group* group, PlayerbotAI* botAI)
    {
        std::vector<Player*> prioritized;
        std::vector<Player*> others;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || !botAI->IsRangedDps(member))
                continue;

            uint8 cls = member->getClass();
            int8 tab = AiFactory::GetPlayerSpecTab(member);

            if (cls == CLASS_HUNTER || cls == CLASS_MAGE || cls == CLASS_DRUID || (cls == CLASS_WARLOCK && tab != 0))
                prioritized.push_back(member);
            else
                others.push_back(member);
        }

        std::vector<Player*> assignedRanged;
        assignedRanged.insert(assignedRanged.end(), prioritized.begin(), prioritized.end());
        assignedRanged.insert(assignedRanged.end(), others.begin(), others.end());

        // Only return up to 8 bots
        if (assignedRanged.size() > 8)
            assignedRanged.resize(8);

        return assignedRanged;
    }

    Player* GetFirstTaintedCorePasser(Player* master, Group* group, PlayerbotAI* botAI)
    {
        if (!master || !group)
            return nullptr;

        Player* closest = nullptr;
        float minDist = std::numeric_limits<float>::max();

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || botAI->IsTank(member) || !GET_PLAYERBOT_AI(member))
                continue;

            float dist = master->GetExactDist(member);
            if (dist < minDist)
            {
                minDist = dist;
                closest = member;
            }
        }
        return closest;
    }

    Player* GetSecondTaintedCorePasser(Unit* closestTrigger, Player* firstPasser, Group* group, PlayerbotAI* botAI)
    {
        if (!closestTrigger || !group)
            return nullptr;

        Player* closest = nullptr;
        float minDist = std::numeric_limits<float>::max();

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == firstPasser ||
                botAI->IsTank(member) || !GET_PLAYERBOT_AI(member))
                continue;

            float dist = closestTrigger->GetExactDist(member);
            if (dist < minDist)
            {
                minDist = dist;
                closest = member;
            }
        }
        return closest;
    }

    Player* GetThirdTaintedCorePasser(Player* secondPasser, Group* group, PlayerbotAI* botAI)
    {
        if (!secondPasser || !group)
            return nullptr;

        Player* closest = nullptr;
        float minDist = std::numeric_limits<float>::max();

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == secondPasser ||
                botAI->IsTank(member) || !GET_PLAYERBOT_AI(member))
                continue;

            float dist = secondPasser->GetExactDist(member);
            if (dist < minDist)
            {
                minDist = dist;
                closest = member;
            }
        }
        return closest;
    }

    void ScheduleCoreReconcile(PlayerbotAI* botAI, Player* giver, Player* receiver, uint32 coreId, uint32 delayMs)
    {
        if (!botAI || !giver || !receiver)
            return;

        botAI->AddTimedEvent([giver, receiver, coreId]()
        {
            if (!giver || !receiver)
                return;

            bool receiverHas = GET_PLAYERBOT_AI(receiver)->HasItemInInventory(coreId);
            bool giverHas = GET_PLAYERBOT_AI(giver)->HasItemInInventory(coreId);

            // If receiver already has one, remove extra from giver to avoid duplicate
            if (receiverHas)
            {
                if (giverHas)
                {
                    giver->DestroyItemCount(coreId, 1, true);
                }
                return;
            }

            // If server consumed giver but didn't grant receiver -> create receiver item
            if (!receiverHas && !giverHas)
            {
                ItemPosCountVec dest;
                uint32 count = 1;
                int canStore = receiver->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, coreId, count);
                if (canStore == EQUIP_ERR_OK)
                {
                    receiver->StoreNewItem(dest, coreId, true, Item::GenerateItemRandomPropertyId(coreId));
                }
                return;
            }

            // Server didn't process -> perform manual transfer (create then remove)
            if (!receiverHas && giverHas)
            {
                ItemPosCountVec dest;
                uint32 count = 1;
                int canStore = receiver->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, coreId, count);
                if (canStore == EQUIP_ERR_OK)
                {
                    Item* created = receiver->StoreNewItem(dest, coreId, true, Item::GenerateItemRandomPropertyId(coreId));
                    if (created)
                    {
                        giver->DestroyItemCount(coreId, 1, true);
                        bool removedAfter = !GET_PLAYERBOT_AI(giver)->HasItemInInventory(coreId);
                        if (!removedAfter)
                        {
                            receiver->DestroyItemCount(coreId, 1, true);
                        }
                    }
                }
            }
        }, delayMs);
    }

    const std::vector<uint32> SHIELD_GENERATOR_DB_GUIDS = { 47482, 47483, 47484, 47485 }; // NW, NE, SE, SW
    // Entries are 185052 { 52.048f, -901.236f, 44.000f }, 185054 { 52.448f, -944.825f, 44.000f },
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
