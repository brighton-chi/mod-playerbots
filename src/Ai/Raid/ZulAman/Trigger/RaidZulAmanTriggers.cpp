#include "RaidZulAmanTriggers.h"
#include "RaidZulAmanHelpers.h"
#include "RaidZulAmanActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace ZulAmanHelpers;

// Trash

bool AmanishiMedicineManSummonedWardTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "amani'shi medicine man");
}

// Akil'zon <Eagle Avatar>

bool AkilzonPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    return akilzon && akilzon->GetHealthPct() > 95.0f;
}

bool AkilzonBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool AkilzonBossCastsStaticDisruptionTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool AkilzonElectricalStormHasFormedTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    Player* stormTarget = GetElectricalStormTarget(bot);
    return stormTarget && stormTarget != bot;
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
    return (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true)) &&
           AI_VALUE2(Unit*, "find target", "nalorakk");
}

bool NalorakkBossCastsSurgeTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "nalorakk");
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    return janalai && janalai->GetHealthPct() > 95.0f;
}

bool JanalaiBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return false;

    return !HasFireBombNearby(botAI, bot);
}

bool JanalaiBossCastsFlameBreathTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return false;

    if (AI_VALUE2(Unit*, "find target", "amani dragonhawk hatchling"))
        return false;

    return !HasFireBombNearby(botAI, bot);
}

bool JanalaiBossSummoningFireBombsTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return false;

    return HasFireBombNearby(botAI, bot);
}

bool JanalaiAmanishiHatchersSpawnedTrigger::IsActive()
{
    if (!botAI->IsRangedDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return false;

    return bot->FindNearestCreature(NPC_AMANISHI_HATCHER, 40.0f);
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
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "halazzi");
}

bool HalazziBossSummonsSpiritLynxTrigger::IsActive()
{
    return botAI->IsAssistTankOfIndex(bot, 0, true) &&
           AI_VALUE2(Unit*, "find target", "halazzi");
}

bool HalazziDeterminingDpsTargetTrigger::IsActive()
{
    return botAI->IsDps(bot) &&
           AI_VALUE2(Unit*, "find target", "halazzi");
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
    return botAI->IsDps(bot) &&
           AI_VALUE2(Unit*, "find target", "hex lord malacrass");
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

    if (!AI_VALUE2(Unit*, "find target", "hex lord malacrass"))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (member->HasAura(SPELL_MIND_CONTROL))
            return true;
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

bool ZuljinBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin && !zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE) &&
           !zuljin->HasAura(SPELL_SHAPE_OF_THE_DRAGONHAWK);
}

bool ZuljinBossIsChannelingWhirlwindInTrollFormTrigger::IsActive()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin || !zuljin->HasAura(SPELL_WHIRLWIND))
        return false;

    return (botAI->IsTank(bot) && zuljin->GetVictim() == bot);
}

bool ZuljinBossIsSummoningCyclonesInEagleFormTrigger::IsActive()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin || !zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE))
        return false;

    constexpr float searchRadius = 40.0f;
    return bot->FindNearestCreature(NPC_FEATHER_VORTEX, searchRadius, true);
}

bool ZuljinBossCastsAoeAbilitiesInDragonhawkFormTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin && zuljin->HasAura(SPELL_SHAPE_OF_THE_DRAGONHAWK);
}
