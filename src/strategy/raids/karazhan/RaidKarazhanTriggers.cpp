#include "RaidKarazhanTriggers.h"
#include "RaidKarazhanHelpers.h"
#include "RaidKarazhanActions.h"
#include "Playerbots.h"

using namespace KarazhanHelpers;

bool SpectralRetainerNeedTargetPriorityTrigger::IsActive()
{
    Unit* retainer = AI_VALUE2(Unit*, "find target", "spectral retainer");

    return retainer && IsMapIDTimerManager(botAI, bot);
}

bool ManaWarpIsAboutToExplodeTrigger::IsActive()
{
    Unit* manaWarp = AI_VALUE2(Unit*, "find target", "mana warp");

    return manaWarp && manaWarp->GetHealthPct() < 15 && (bot->getClass() == CLASS_ROGUE ||
           bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_PALADIN);
}

bool AttumenTheHuntsmanNeedTargetPriorityTrigger::IsActive()
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    return midnight || attumen || attumenMounted;
}

bool AttumenTheHuntsmanAttumenSpawnedTrigger::IsActive()
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);

    return midnight && attumen && botAI->IsAssistTankOfIndex(bot, 0);
}

bool AttumenTheHuntsmanAttumenMountedTrigger::IsActive()
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    return attumenMounted && !botAI->IsMainTank(bot) && !botAI->IsHeal(bot) &&
           attumenMounted->GetVictim() != bot;
}

bool AttumenTheHuntsmanManageTimerTrigger::IsActive()
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    return (midnight || attumenMounted) && IsMapIDTimerManager(botAI, bot);
}

bool MoroesNeedTargetPriorityTrigger::IsActive()
{
    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");
    Unit* target = dorothea ? dorothea : (catriona ? catriona : (keira ? keira : (rafe ? rafe : (robin ? robin : crispin))));

    return target && target->IsAlive() && IsMapIDTimerManager(botAI, bot);
}

bool MaidenOfVirtueBossEngagedByMainTankTrigger::IsActive()
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    return maiden && botAI->IsMainTank(bot) && maiden->GetVictim() == bot;
}

bool MaidenOfVirtueBossEngagedByRangedTrigger::IsActive()
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    return maiden && botAI->IsRanged(bot);
}

bool BigBadWolfBossEngagedByMainTankTrigger::IsActive()
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    return wolf && botAI->IsMainTank(bot) && wolf->GetVictim() == bot &&
           !bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD);
}

bool BigBadWolfBossIsChasingLittleRedRidingHoodTrigger::IsActive()
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    return wolf && bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD);
}

bool RomuloAndJulianneBothBossesRevivedTrigger::IsActive()
{
    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");

    return julianne && romulo && (julianne->IsAlive() || romulo->IsAlive()) && IsMapIDTimerManager(botAI, bot);
}

bool WizardOfOzNeedTargetPriorityTrigger::IsActive()
{
    Unit* dorothee = AI_VALUE2(Unit*, "find target", "dorothee");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");
    Unit* target = dorothee ? dorothee : (tito ? tito : (roar ? roar : (strawman ? strawman : (tinhead ? tinhead : crone))));

    return target && target->IsAlive();
}

bool WizardOfOzStrawmanIsVulnerableToFireTrigger::IsActive()
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");

    return strawman && strawman->IsAlive() && bot->getClass() == CLASS_MAGE;
}

bool TheCuratorAstralFlareSpawnedTrigger::IsActive()
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    Unit* target = AI_VALUE2(Unit*, "find target", "astral flare");

    return curator && target && IsMapIDTimerManager(botAI, bot);
}

bool TheCuratorBossEngagedByTanksTrigger::IsActive()
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");

    return curator && (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0));
}

bool TheCuratorBossEngagedByRangedTrigger::IsActive()
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");

    return curator && botAI->IsRanged(bot);
}

bool TerestianIllhoofNeedTargetPriorityTrigger::IsActive()
{
    Unit* illhoof = AI_VALUE2(Unit*, "find target", "terestian illhoof");
    Unit* target = AI_VALUE2(Unit*, "find target", "demon chains");

    if (!target || !target->IsAlive())
    {
        Unit* target = AI_VALUE2(Unit*, "find target", "kil'rek");
        if (!target || !target->IsAlive())
            target = illhoof;
    }

    return illhoof && target && IsMapIDTimerManager(botAI, bot);
}

bool ShadeOfAranArcaneExplosionIsCastingTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");

    return aran && aran->HasUnitState(UNIT_STATE_CASTING) &&
           aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION) &&
           !IsFlameWreathActive(botAI, bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");

    return aran && IsFlameWreathActive(botAI, bot);
}

// Exclusion of Banish is so the player may Banish elementals if they wish
bool ShadeOfAranConjuredElementalsSummonedTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    Unit* elemental = AI_VALUE2(Unit*, "find target", "conjured elemental");

    return IsMapIDTimerManager(botAI, bot) && elemental && elemental->IsAlive() &&
           !elemental->HasAura(SPELL_WARLOCK_BANISH);
}

bool ShadeOfAranBossUsesCounterspellAndBlizzardTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");

    return aran && botAI->IsRanged(bot) && !IsFlameWreathActive(botAI, bot) &&
           !(aran->HasUnitState(UNIT_STATE_CASTING) &&
             aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION));
}

bool NetherspiteRedBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);

    return netherspite && redPortal && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED);
}

bool NetherspiteBlueBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);

    return netherspite && bluePortal && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED);
}

bool NetherspiteGreenBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);

    return netherspite && greenPortal && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED);
}

bool NetherspiteBotIsNotBeamBlockerTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);

    return netherspite && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED) && bot != redBlocker &&
           bot != blueBlocker && bot != greenBlocker;
}

bool NetherspiteBossIsBanishedTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return false;

    std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);
    for (Unit* vz : voidZones)
    {
        if (bot->GetExactDist2d(vz) < 4.0f)
            return true;
    }

    return false;
}

bool NetherspiteNeedToManageTimersTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");

    return netherspite;
}

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");

    return malchezaar && bot->HasAura(SPELL_ENFEEBLE);
}

bool PrinceMalchezaarInfernalsAreSpawnedTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");

    return malchezaar && !botAI->IsMainTank(bot);
}

bool PrinceMalchezaarBossEngagedByMainTankTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");

    return malchezaar && botAI->IsMainTank(bot) && malchezaar->GetVictim() == bot;
}

// Z-axis of 95 yards is used to determine if Nightbane is flying
bool NightbaneBossEngagedByMainTankTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");

    return nightbane && nightbane->GetPositionZ() <= 95.0f && botAI->IsMainTank(bot);
}

bool NightbaneRangedPrepareForCharredEarthTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");

    return nightbane && nightbane->GetPositionZ() <= 95.0f && botAI->IsRanged(bot);
}

bool NightbaneMainTankIsSusceptibleToFearTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    Group* group = bot->GetGroup();
    if (!nightbane || !group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    return bot->getClass() == CLASS_PRIEST && mainTank &&
           !mainTank->HasAura(SPELL_FEAR_WARD) && botAI->CanCastSpell("fear ward", mainTank);
}

bool NightbanePetsIgnoreColllisionToChaseFlyingBossTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    Pet* pet = bot->GetPet();

    return nightbane && pet && pet->IsAlive();
}

bool NightbaneBossIsFlyingTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() <= 95.0f)
        return false;

    uint32 mapId = nightbane->GetMapId();
    time_t now = time(nullptr);

    return nightbaneFlightPhaseStartTimer.find(mapId) != nightbaneFlightPhaseStartTimer.end() &&
           (now - nightbaneFlightPhaseStartTimer[mapId] < 35);
}

bool NightbaneNeedToManageTimersAndTrackersTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");

    return nightbane;
}
