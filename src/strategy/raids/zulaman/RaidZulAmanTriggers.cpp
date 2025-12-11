#include "RaidZulAmanTriggers.h"
#include "RaidZulAmanHelpers.h"
#include "RaidZulAmanActions.h"
#include "Playerbots.h"

using namespace ZulAmanHelpers;

// Akil'zon <Eagle Avatar>

bool AkilzonPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    return akilzon && akilzon->GetHealthPct() > 95.0f;
}

bool AkilzonBossCastsStaticDisruptionTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (!akilzon)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_ELECTRICAL_STORM))
            return false;
    }

    return true;
}

bool AkilzonElectricalStormHasFormedTrigger::IsActive()
{
    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (!akilzon)
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->HasAura(SPELL_ELECTRICAL_STORM))
                return true;
        }
    }

    return false;
}

// Nalorakk <Bear Avatar>

bool NalorakkPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    return nalorakk && nalorakk->GetHealthPct() > 95.0f;
}

bool NalorakkBossSwitchesFormsTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    return nalorakk != nullptr;
}

bool NalorakkBossCastsSurgeTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    return nalorakk != nullptr;
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    return janalai && janalai->GetHealthPct() > 95.0f;
}

bool JanalaiBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    if (AnyNearbyNpcWithEntry(botAI, NPC_FIRE_BOMB))
        return false;

    return true;
}

bool JanalaiBossCastsFlameBreathTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    // Trying to find a way to get the fixed dispersal off when hatchlings spawn
    // TBD whether some other type of dispersal will still be needed during this period
    Unit* hatchling = AI_VALUE2(Unit*, "find target", "amani dragonhawk hatchling");
    if (hatchling && hatchling->GetExactDist2d(janalai) < 30.0f)
        return false;

    if (AnyNearbyNpcWithEntry(botAI, NPC_FIRE_BOMB))
        return false;

    return true;
}

bool JanalaiBossSummoningFireBombsTrigger::IsActive()
{
    if (AnyNearbyNpcWithEntry(botAI, NPC_FIRE_BOMB))
        return true;

    return false;
}

// Halazzi <Lynx Avatar>

bool HalazziPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    return halazzi && halazzi->GetHealthPct() > 95.0f;
}

bool HalazziBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    return halazzi != nullptr;
}

bool HalazziBossSummonsSpiritLynxTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    return halazzi != nullptr;
}

bool HalazziDeterminingDpsTargetTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    return halazzi != nullptr;
}

// Hex Lord Malacrass

bool HexLordMalacrassPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    return malacrass && malacrass->GetHealthPct() > 95.0f;
}

bool HexLordMalacrassDeterminingKillOrderTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    return malacrass != nullptr;
}

bool HexLordMalacrassBossHasSiphonedBuffTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER &&
        bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PRIEST &&
        bot->getClass() != CLASS_SHAMAN &&
        bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    return malacrass &&
           (malacrass->HasAura(SPELL_LIFEBLOOM) || malacrass->HasAura(SPELL_AVENGING_WRATH));
}

bool HexLordMalacrassPartyMemberIsMindControlledTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST &&
        bot->getClass() != CLASS_SHAMAN &&
        bot->getClass() != CLASS_WARLOCK)
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            if (member->HasAura(SPELL_MIND_CONTROL))
                return true;
        }
    }

    return false;
}

// Zul'jin

bool ZuljinMainTankNeedsAggroUponPullOrPhaseChangeTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    float hp = zuljin->GetHealthPct();

    return (hp <= 100.0f && hp > 95.0f) ||
           (hp <= 80.0f && hp > 75.0f && zuljin->HasAura(SPELL_SHAPE_OF_THE_BEAR)) ||
           (hp <= 40.0f && hp > 35.0f && zuljin->HasAura(SPELL_SHAPE_OF_THE_LYNX)) ||
           (hp <= 20.0f && hp > 15.0f && zuljin->HasAura(SPELL_SHAPE_OF_THE_DRAGONHAWK));
}

bool ZuljinBossIsChannelingWhirlwindInTrollFormTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin && zuljin->HasAura(SPELL_WHIRLWIND);
}

bool ZuljinBossCastsAoeAbilitiesTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin &&
           (zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE) ||
            zuljin->HasAura(SPELL_SHAPE_OF_THE_DRAGONHAWK));
}

bool ZuljinBossIsChargingPlayersInLynxFormTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin && zuljin->HasAura(SPELL_SHAPE_OF_THE_LYNX);
}
