#include "RaidKarazhanTriggers.h"
#include "RaidKarazhanHelpers.h"
#include "RaidKarazhanActions.h"
#include "Playerbots.h"

using namespace KarazhanHelpers;

bool AttumenTheHuntsmanMainTankTrigger::IsActive()
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    return (midnight || attumen || attumenMounted) && botAI->IsMainTank(bot);
}

bool AttumenTheHuntsmanSplitBossesTrigger::IsActive()
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);

    return midnight && attumen && botAI->IsAssistTankOfIndex(bot, 0);
}

bool AttumenTheHuntsmanStackBehindTrigger::IsActive()
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    return attumenMounted && !botAI->IsMainTank(bot) && attumenMounted->GetVictim() != bot;
}

bool AttumenTheHuntsmanManageTimerTrigger::IsActive()
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    return (midnight || attumenMounted) && IsMapIDTimerManager(bot);
}

bool MoroesMarkTargetTrigger::IsActive()
{
    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");
    Unit* target = dorothea ? dorothea : (catriona ? catriona : (keira ? keira : (rafe ? rafe : (robin ? robin : crispin))));

    return target && target->IsAlive();
}

bool MaidenOfVirtueMainTankMovementTrigger::IsActive()
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    return maiden && botAI->IsMainTank(bot) && maiden->GetVictim() == bot;
}

bool MaidenOfVirtuePositionRangedTrigger::IsActive()
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    return maiden && botAI->IsRanged(bot);
}

bool BigBadWolfPositionBossTrigger::IsActive()
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    return wolf && botAI->IsMainTank(bot) && wolf->GetVictim() == bot &&
           !bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD);
}

bool BigBadWolfRunAwayTrigger::IsActive()
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    return wolf && bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD);
}

bool RomuloAndJulianneMarkTargetTrigger::IsActive()
{
    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");

    return julianne && romulo && (julianne->IsAlive() || romulo->IsAlive());
}

bool WizardOfOzMarkTargetTrigger::IsActive()
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

bool WizardOfOzScorchStrawmanTrigger::IsActive()
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");

    return strawman && strawman->IsAlive() && bot->getClass() == CLASS_MAGE;
}

bool TheCuratorMarkAstralFlareTrigger::IsActive()
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    Unit* target = AI_VALUE2(Unit*, "find target", "astral flare");

    return curator && target && target->IsAlive();
}

// Inclusion of Assist Tank is because they need to be given the directive to attack
// Tanks with TankAssistAction disabled will not initiate combat unless ordered to do so
bool TheCuratorPositionBossTrigger::IsActive()
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");

    return curator && (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0));
}

bool TheCuratorSpreadRangedTrigger::IsActive()
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");

    return curator && botAI->IsRanged(bot);
}

bool TerestianIllhoofMarkTargetTrigger::IsActive()
{
    Unit* illhoof = AI_VALUE2(Unit*, "find target", "terestian illhoof");
    Unit* target = GetFirstAliveUnitByEntry(botAI, NPC_DEMON_CHAINS);

    if (!target || !target->IsAlive()) 
    {
        target = GetFirstAliveUnitByEntry(botAI, NPC_KILREK);
        if (!target || !target->IsAlive()) 
            target = illhoof;
    }

    return illhoof && target && target->IsAlive();
}

bool ShadeOfAranArcaneExplosionRunAwayTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");

    return aran && aran->HasUnitState(UNIT_STATE_CASTING) &&
           aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION);
}

bool ShadeOfAranFlameWreathStopMovementTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");

    return aran && IsFlameWreathActive(botAI, bot);
}

// Exclusion of Banish is so the player may Banish elementals if they wish
bool ShadeOfAranMarkConjuredElementalTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    Unit* target = GetFirstAliveUnitByEntry(botAI, NPC_CONJURED_ELEMENTAL);

    return aran && target && target->IsAlive() && !target->HasAura(SPELL_WARLOCK_BANISH);
}

bool ShadeOfAranSpreadRangedTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");

    return aran && botAI->IsRanged(bot) && !IsFlameWreathActive(botAI, bot) &&
           !(aran->HasUnitState(UNIT_STATE_CASTING) && 
           aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION));
}

bool NetherspiteBlockRedBeamTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);

    return netherspite && redPortal && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED);
}

bool NetherspiteBlockBlueBeamTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);

    return netherspite && bluePortal && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED);
}

bool NetherspiteBlockGreenBeamTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);

    return netherspite && greenPortal && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED);
}

bool NetherspiteAvoidBeamAndVoidZoneTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);

    return netherspite && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED) && bot != redBlocker && 
           bot != blueBlocker && bot != greenBlocker;
}

bool NetherspiteBanishPhaseAvoidVoidZoneTrigger::IsActive()
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

bool NetherspiteManageTimersTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");

    return netherspite;
}

bool PrinceMalchezaarEnfeebledAvoidHazardTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");

    return malchezaar && bot->HasAura(SPELL_ENFEEBLE);
}

bool PrinceMalchezaarNonTankAvoidInfernalTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");

    return malchezaar && !botAI->IsMainTank(bot);
}

/* bool PrinceMalchezaarNonTankAvoidHazardTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");

    return malchezaar && !(botAI->IsMainTank(bot) && malchezaar->GetVictim() != bot);
} */

bool PrinceMalchezaarMainTankMovementTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");

    return malchezaar && botAI->IsMainTank(bot) && malchezaar->GetVictim() == bot;
}

bool NightbaneGroundPhasePositionBossTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");

    return nightbane && !nightbane->IsFlying() && botAI->IsMainTank(bot);
}

bool NightbaneGroundPhaseRotateRangedPositionsTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");

    return nightbane && !nightbane->IsFlying() && botAI->IsRanged(bot);
}

bool NightbaneCastFearWardOnMainTankTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    Player* mainTank = nullptr;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    return nightbane && bot->getClass() == CLASS_PRIEST && 
           mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) &&
           botAI->CanCastSpell("fear ward", mainTank);
}

bool NightbaneControlPetAggressionTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    Pet* pet = bot->GetPet();

    return nightbane && pet && pet->IsAlive();
}

bool NightbaneFlightPhaseMovementTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || !nightbane->IsFlying())
        return false;

    uint32 mapId = nightbane->GetMapId();
    time_t now = time(nullptr);

    return nightbaneFlightPhaseStartTimer.find(mapId) != nightbaneFlightPhaseStartTimer.end() &&
           (now - nightbaneFlightPhaseStartTimer[mapId] < 35);
}

bool NightbaneManageTimersAndTrackersTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");

    return nightbane;
}
