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

bool SupremusNeedToManagePhaseTimerTrigger::IsActive()
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

bool TeronGorefiendBossEngagedTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot) && !botAI->IsRanged(bot))
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    return gorefiend != nullptr;
}

bool TeronGorefiendBossIsCastingShadowOfDeathTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER &&
        bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_ROGUE)
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend || !gorefiend->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = gorefiend->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != SPELL_SHADOW_OF_DEATH)
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

bool TeronGorefiendBotHasShadowOfDeathTrigger::IsActive()
{
    Aura* aura = bot->GetAura(SPELL_SHADOW_OF_DEATH);
    return aura && aura->GetDuration() < 12000; // less than 12 seconds remaining
}

bool TeronGorefiendBotTransformedIntoVengefulSpiritTrigger::IsActive()
{
    return bot->HasAura(SPELL_SPIRITUAL_VENGEANCE);
}

// Gurtogg Bloodboil

bool GurtoggBloodboilPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    auto it = gurtoggPhaseTimer.find(gurtogg->GetMap()->GetInstanceId());
    if (it == gurtoggPhaseTimer.end())
        return false;

    time_t elapsed = std::time(nullptr) - it->second;

    // Trigger if in first 10s, or in 120-130s, or in the first 10s of any 90s cycle after 120s
    if ((elapsed < 10) ||
        (elapsed >= 120 && elapsed < 130) ||
        (elapsed > 130 && ((elapsed - 120) % 90) < 10))
    {
        return true;
    }

    return false;
}

bool GurtoggBloodboilBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && !gurtogg->HasAura(SPELL_BOSS_FEL_RAGE);
}

bool GurtoggBloodboilBossCastsAoeSpellsTrigger::IsActive()
{
    if (!botAI->IsRanged(bot) || bot->HasAura(SPELL_PLAYER_FEL_RAGE))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    // Get rotation groups and active group index
    auto groups = GetGurtoggRangedRotationGroups(bot);
    int8 activeGroup = GetGurtoggActiveRotationGroup(gurtogg);

    // Exclude bots in the active rotation group
    if (activeGroup >= 0 && activeGroup < groups.size())
    {
        const auto& group = groups[activeGroup];
        if (std::find(group.begin(), group.end(), bot) != group.end())
            return false;
    }

    return true;
}

bool GurtoggBloodboilBossCastsBloodboilOnFiveFarthestPlayersTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    auto groups = GetGurtoggRangedRotationGroups(bot);
    int activeGroup = GetGurtoggActiveRotationGroup(gurtogg);

    if (activeGroup >= 0 && activeGroup < groups.size())
    {
        const auto& group = groups[activeGroup];
        if (std::find(group.begin(), group.end(), bot) != group.end())
            return true;
    }

    return false;
}

bool GurtoggBloodboilBotHasFelRageTrigger::IsActive()
{
    return bot->HasAura(SPELL_PLAYER_FEL_RAGE);
}

bool GurtoggBloodboilNeedToManagePhaseTimerTrigger::IsActive()
{
    if (!IsInstanceTimerManager(botAI, bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "gurtogg bloodboil") != nullptr;
}

// Reliquary of Souls

bool ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* reliquary = AI_VALUE2(Unit*, "find target", "reliquary of the lost");
    return reliquary != nullptr;
}

bool ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger::IsActive()
{
    if (!botAI->IsDps(bot) && !botAI->IsMelee(bot))
        return false;

    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    return suffering && suffering->GetVictim() != bot;
}

bool ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger::IsActive()
{
    if (!botAI->IsHeal(bot) || bot->getClass() == CLASS_PRIEST)
        return false;

    return AI_VALUE2(Unit*, "find target", "reliquary of the lost");
}

bool ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    return desire && desire->HasAura(SPELL_RUNE_SHIELD);
}

bool ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger::IsActive()
{
    if (!botAI->IsTank(bot) || bot->getClass() != CLASS_WARRIOR)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire || !desire->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = desire->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != SPELL_DEADEN)
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

bool ReliquaryOfSoulsNeedToManageDpsTimerTrigger::IsActive()
{
    if (!IsInstanceTimerManager(botAI, bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "reliquary of the lost");
}


// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
