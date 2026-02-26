#include "RaidZulAmanHelpers.h"
#include "Group.h"
#include "Playerbots.h"

namespace ZulAmanHelpers
{
    // Akil'zon <Eagle Avatar>
    const Position AKILZON_TANK_POSITION = { 378.369f, 1407.718f, 74.797f };

    Player* GetElectricalStormTarget(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->HasAura(SPELL_ELECTRICAL_STORM))
                return member;
        }

        return nullptr;
    }

    // Nalorakk <Bear Avatar>
    const Position NALORAKK_TANK_POSITION = { -80.208f, 1324.530f, 40.942f };

    // Jan'alai <Dragonhawk Avatar>
    const Position JANALAI_TANK_POSITION = { -33.873f, 1149.571f, 19.146f };

    bool HasFireBombNearby(PlayerbotAI* botAI, Player* bot)
    {
        constexpr float searchRadius = 30.0f;
        std::list<Creature*> creatureList;
        bot->GetCreatureListWithEntryInGrid(creatureList, NPC_FIRE_BOMB, searchRadius);

        for (Creature* creature : creatureList)
        {
            if (creature && creature->IsAlive())
                return true;
        }

        return false;
    }

    std::pair<Unit*, Unit*> GetAmanishiHatcherPair(PlayerbotAI* botAI)
    {
        Unit* lowest = nullptr;
        Unit* highest = nullptr;

        for (auto const& guid :
             botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == NPC_AMANISHI_HATCHER)
            {
                if (!lowest || unit->GetGUID().GetCounter() < lowest->GetGUID().GetCounter())
                    lowest = unit;

                if (!highest || unit->GetGUID().GetCounter() > highest->GetGUID().GetCounter())
                    highest = unit;
            }
        }

        return {lowest, highest};
    }

    // Halazzi <Lynx Avatar>
    const Position HALAZZI_TANK_POSITION = { 370.733f, 1131.202f, 6.516f };

    // Zul'jin
    const Position ZULJIN_TANK_POSITION = { 120.210f, 705.564f, 45.111f };
}
