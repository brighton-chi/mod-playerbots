#include "RaidSSCHelpers.h"
#include "RaidSSCActions.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace SerpentShrineCavernHelpers
{
    // std::unordered_map<uint32, time_t> hydrossFrostDPSWaitTimer;
    // std::unordered_map<uint32, time_t> hydrossNatureDPSWaitTimer;
    std::unordered_map<ObjectGuid, time_t> hydrossChangeToFrostPhaseTimer;
    std::unordered_map<ObjectGuid, time_t> hydrossChangeToNaturePhaseTimer;
    std::unordered_map<uint32, time_t> leotherasHumanFormDPSWaitTimer;
    std::unordered_map<uint32, time_t> leotherasDemonFormDPSWaitTimer;
    std::unordered_map<uint32, time_t> leotherasFinalPhaseDPSWaitTimer;
    std::unordered_map<ObjectGuid, bool> hasReachedDemonFormTankPosition;

    namespace SerpentShrineCavernLocations
    {
        const Location HydrossFrostTankPosition = { -244.242f, -378.373f, -0.827f };
        const Location HydrossNatureTankPosition = { -232.043f, -345.070f, -0.883f };
        // Hydross dps locations?

        // Lurker liquid level -19.881f
        const Location LurkerCenterOfPoolPosition = { 40.843f, -416.379f, -21.613f };

        const Location KarathressTankPosition = { 469.083f, -543.687f, -7.547f };
        const Location TidalvessTankPosition = { 522.923f, -497.355f, -13.158f };
        const Location SharkkisTankPosition = { 521.580f, -546.681f, -7.377f };
        const Location CaribdisTankPosition = { 456.715f, -490.817f, -13.158f };

        const Location LeotherasHumanFormTankPosition = { 347.667f, -424.348f, 28.585f };
        const Location LeotherasDemonFormTankPosition = { 375.898f, -438.234f, 29.523f };

        const Location TidewalkerPhase1TankPosition = { 410.925f, -741.916f, -7.146f };
        const Location TidewalkerPhase2TankPosition = { 446.571f, -767.155f, -7.144f };
        const Location TidewalkerPhase2DPSAndHealerPosition = { 427.494f, -751.716f, -7.146f };
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
            {
                group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
            }
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

    std::string GetRtiName(uint8 index)
    {
        switch (index)
        {
            case RtiTargetValue::squareIndex:   return "square";
            case RtiTargetValue::starIndex:     return "star";
            case RtiTargetValue::circleIndex:   return "circle";
            case RtiTargetValue::diamondIndex:  return "diamond";
            case RtiTargetValue::triangleIndex: return "triangle";
            case RtiTargetValue::crossIndex:    return "cross";
            case RtiTargetValue::moonIndex:     return "moon";
            case RtiTargetValue::skullIndex:    return "skull";
            default:                            return "none";
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
        for (const auto& npcGuid : npcs)
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

    Player* GetCaribdisTankHealer(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            if (member->getClass() == CLASS_PALADIN && botAI->IsHeal(member))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            if (botAI->IsHeal(member))
                return member;
        }

        return nullptr;
    }

}
