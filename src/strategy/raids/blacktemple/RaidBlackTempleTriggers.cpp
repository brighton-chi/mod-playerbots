#include "RaidBlackTempleTriggers.h"
#include "RaidBlackTempleHelpers.h"
#include "RaidBlackTempleActions.h"
#include "AiFactory.h"
#include "Playerbots.h"

using namespace BlackTempleHelpers;

// High Warlord Naj'entus

bool HighWarlordNajentusPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    return najentus && najentus->GetHealthPct() > 95.0f;
}

bool HighWarlordNajentusBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    return najentus != nullptr;
}

bool HighWarlordNajentusCastsNeedleSpinesTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    return najentus != nullptr;
}

bool HighWarlordNajentusPlayerIsImpaledTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* impaledPlayer = nullptr;
    // Find any player with Impaling Spine (other than bot itself, duh)
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (member->HasAura(SPELL_IMPALING_SPINE))
        {
            impaledPlayer = member;
            break;
        }
    }

    // Only fire for the closest non-tank bot to the impaled player
    if (impaledPlayer)
    {
        // Check if this bot is the closest non-tank bot to the impaled player
        float myDist = bot->GetDistance(impaledPlayer);
        bool isClosest = true;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == impaledPlayer ||
                !GET_PLAYERBOT_AI(member) || botAI->IsTank(member))
                continue;

            float otherDist = member->GetDistance(impaledPlayer);
            if (otherDist < myDist)
            {
                isClosest = false;
                break;
            }
        }

        return isClosest;
    }

    return false;
}

bool HighWarlordNajentusBossHasTidalShieldTrigger::IsActive()
{
    if (!botAI->HasItemInInventory(ITEM_NAJENTUS_SPINE))
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    return najentus && najentus->HasAura(SPELL_TIDAL_SHIELD);
}

// Supremus

bool SupremusPullingBossOrChangingPhaseTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    auto it = supremusPhaseTimer.find(supremus->GetMap()->GetInstanceId());
    if (it == supremusPhaseTimer.end())
        return false; // Timer not started yet

    time_t now = time(nullptr);
    time_t elapsed = now - it->second;

    // Fire during first 10 seconds, or during 60-70, 120-130, etc.
    if ((elapsed < 10) || ((elapsed % 60) < 10 && elapsed >= 60))
        return true;

    return false;
}

bool SupremusBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && !supremus->HasAura(SPELL_SNARE_SELF);
}

bool SupremusBossIsFixatedOnBotTrigger::IsActive()
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && supremus->HasAura(SPELL_SNARE_SELF)
           && supremus->GetVictim() == bot;
}

bool SupremusVolcanoIsNearbyTrigger::IsActive()
{
    return HasSupremusVolcanoNearby(botAI, bot);
}

bool SupremusChangesPhaseEvery60SecondsTrigger::IsActive()
{
    if (!IsInstanceTimerManager(botAI, bot))
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus != nullptr;
}

// Shade of Akama
// N/A

// Teron Gorefiend

bool TeronGorefiendPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    return gorefiend && gorefiend->GetHealthPct() > 95.0f;
}

bool TeronGorefiendBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    return gorefiend != nullptr;
}

bool TeronGorefiendBotHasShadowOfDeathTrigger::IsActive()
{
    Aura* aura = bot->GetAura(SPELL_SHADOW_OF_DEATH);
    return aura && aura->GetDuration() < 20000; // less than 20 seconds remaining
}

bool TeronGorefiendBotTransformedIntoVengefulSpiritTrigger::IsActive()
{
    return bot->HasAura(SPELL_SPIRITUAL_VENGEANCE);
}

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
