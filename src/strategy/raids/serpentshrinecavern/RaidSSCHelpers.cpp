#include "RaidSSCHelpers.h"
#include "RaidSSCActions.h"
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

    std::map<ObjectGuid, uint8> tidewalkerTankStep;
    std::map<ObjectGuid, uint8> tidewalkerRangedStep;

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
}
