#include "RaidSSCMultipliers.h"
#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "ChooseTargetActions.h"
#include "DestroyItemAction.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "LootAction.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WipeAction.h"

using namespace SerpentShrineCavernHelpers;

// Trash

float UnderbogColossusEscapeToxicPoolMultiplier::GetValue(Action* action)
{
    Aura* aura = bot->GetAura(SPELL_TOXIC_POOL);
    if (!aura)
        return 1.0f;

    DynamicObject* dynObj = aura->GetDynobjOwner();
    if (!dynObj)
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<UnderbogColossusEscapeToxicPoolAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Hydross the Unstable <Duke of Currents>

float HydrossTheUnstableDisableTankActionsMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross");
    if (!hydross || dynamic_cast<WipeAction*>(action))
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (botAI->IsMainTank(bot))
    {
        if (hydross->HasAura(SPELL_CORRUPTION))
        {
            if (!dynamic_cast<HydrossTheUnstablePositionFrostTankAction*>(action))
                return 0.0f;
        }
    }

    if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (!hydross->HasAura(SPELL_CORRUPTION))
        {
            if (!dynamic_cast<HydrossTheUnstablePositionNatureTankAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

float HydrossTheUnstableWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0))
        return 1.0f;

    Unit* waterElemental = AI_VALUE2(Unit*, "find target", "pure spawn of hydross");
    Unit* natureElemental = AI_VALUE2(Unit*, "find target", "tainted spawn of hydross");
    if (botAI->IsAssistTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0) &&
        (waterElemental || natureElemental))
        return 1.0f;

    if (dynamic_cast<HydrossTheUnstableMisdirectBossToTankAction*>(action))
        return 1.0f;

    const uint32 mapId = hydross->GetMapId();
    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 5;
    const uint8 phaseChangeWaitSeconds = 6;

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        if (botAI->IsAssistTankOfIndex(bot, 0))
        {
            if (dynamic_cast<AttackAction*>(action) &&
                !dynamic_cast<HydrossTheUnstablePositionNatureTankAction*>(action))
                return 0.0f;
        }
        else if (botAI->IsTank(bot))
            return 1.0f;

        auto itDps = hydrossFrostDpsWaitTimer.find(mapId);
        auto itPhase = hydrossChangeToFrostPhaseTimer.find(mapId);

        bool justChanged = (itDps == hydrossFrostDpsWaitTimer.end() ||
                            (now - itDps->second) < dpsWaitSeconds);

        bool aboutToChange = (itPhase != hydrossChangeToFrostPhaseTimer.end() &&
                              (now - itPhase->second) > phaseChangeWaitSeconds);

        if (justChanged || aboutToChange)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION))
    {
        if (botAI->IsMainTank(bot))
        {
            if (dynamic_cast<AttackAction*>(action) &&
                !dynamic_cast<HydrossTheUnstablePositionNatureTankAction*>(action))
                return 0.0f;
        }
        else if (botAI->IsTank(bot))
            return 1.0f;

        auto itDps = hydrossNatureDpsWaitTimer.find(mapId);
        auto itPhase = hydrossChangeToNaturePhaseTimer.find(mapId);

        bool justChanged = (itDps == hydrossNatureDpsWaitTimer.end() ||
                            (now - itDps->second) < dpsWaitSeconds);

        bool aboutToChange = (itPhase != hydrossChangeToNaturePhaseTimer.end() &&
                              (now - itPhase->second) > phaseChangeWaitSeconds);

        if (justChanged || aboutToChange)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    return 1.0f;
}

float HydrossTheUnstableControlMisdirectionMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

// The Lurker Below

float TheLurkerBelowStayAwayFromSpoutMultiplier::GetValue(Action* action)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return 1.0f;

    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(lurker->GetMapId());
    if (it != lurkerSpoutTimer.end() && it->second > now)
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action) || dynamic_cast<CastKillingSpreeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Leotheras the Blind

float LeotherasTheBlindAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    if (!leotherasHuman)
        return 1.0f;

    if (!leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED) &&
        (leotherasHuman->HasAura(SPELL_WHIRLWIND) || leotherasHuman->HasAura(SPELL_WHIRLWIND_CHANNEL)))
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;

        if (!botAI->IsTank(bot))
        {
            if (dynamic_cast<MovementAction*>(action) &&
                !dynamic_cast<LeotherasTheBlindRunAwayFromWhirlwindAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

// Applies only if there is a Warlock tank
float LeotherasTheBlindDisableTankActionsMultiplier::GetValue(Action* action)
{
    // (1) Multipliers that apply during Phase 2 or 3
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon ||
        dynamic_cast<LeotherasTheBlindInnerDemonCheatAction*>(action) ||
        dynamic_cast<WipeAction*>(action))
        return 1.0f;

    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (!demonFormTank || demonFormTank->getClass() != CLASS_WARLOCK)
        return 1.0f;

    if (dynamic_cast<CastShadowWardAction*>(action))
        return 0.0f;

    // (2) Phase 2 only: Tanks other than the Demon Form tank should do absolutely nothing
    Unit* leotherasDemonPhase2 = GetPhase2LeotherasDemon(botAI);
    if (botAI->IsTank(bot) && bot != demonFormTank && leotherasDemonPhase2)
        return 0.0f;

    return 1.0f;
}

// Applies only if there is no Warlock tank
float LeotherasTheBlindMeleeTankMaintainDemonFormPositionMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return 1.0f;

    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (demonFormTank && demonFormTank->getClass() != CLASS_WARLOCK)
        return 1.0f;

    if (botAI->IsTank(bot) && leotherasDemon->GetVictim() == bot)
    {
    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindDemonFormTankAttackBossAction*>(action))
        return 0.0f;
    }

    return 1.0f;
}

// Applies only if there is no Warlock tank
float LeotherasTheBlindDemonFormDisableMeleeActionsMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (demonFormTank && demonFormTank->getClass() == CLASS_WARLOCK)
        return 1.0f;

    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    if (!leotherasPhase2Demon || leotherasPhase2Demon->GetVictim() == bot ||
        bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return 1.0f;

    if (botAI->IsMelee(bot) && botAI->IsDps(bot))
    {
        if (dynamic_cast<CastKillingSpreeAction*>(action) || (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<LeotherasTheBlindDemonFormPositionMeleeAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float LeotherasTheBlindWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    if (dynamic_cast<LeotherasTheBlindMisdirectBossToDemonFormTankAction*>(action))
        return 1.0f;

    const uint32 mapId = leotheras->GetMapId();
    const time_t now = std::time(nullptr);

    const uint8 dpsWaitSecondsPhase1 = 5;
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (leotherasHuman && !leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED) && !leotherasPhase3Demon)
    {
        if (botAI->IsTank(bot))
            return 1.0f;

        auto it = leotherasHumanFormDpsWaitTimer.find(mapId);
        if (it == leotherasHumanFormDpsWaitTimer.end() || (now - it->second) < dpsWaitSecondsPhase1)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    const uint8 dpsWaitSecondsPhase2 = 10;
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (leotherasPhase2Demon)
    {
        if (demonFormTank == bot)
            return 1.0f;

        auto it = leotherasDemonFormDpsWaitTimer.find(mapId);
        if (it == leotherasDemonFormDpsWaitTimer.end() || (now - it->second) < dpsWaitSecondsPhase2)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    const uint8 dpsWaitSecondsPhase3 = 12;
    if (leotherasPhase3Demon)
    {
        if (demonFormTank == bot || botAI->IsTank(bot))
            return 1.0f;

        auto it = leotherasFinalPhaseDpsWaitTimer.find(mapId);
        if (it == leotherasFinalPhaseDpsWaitTimer.end() || (now - it->second) < dpsWaitSecondsPhase3)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    return 1.0f;
}

// Wait until the final phase to use Bloodlust/Heroism
float LeotherasTheBlindDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasPhase3Demon)
    {
        if (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Fathom-Lord Karathress

float FathomLordKarathressDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressDisableAoeMultiplier::GetValue(Action* action)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if (dynamic_cast<DpsAoeAction*>(action))
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressControlMisdirectionMultiplier::GetValue(Action* action)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if (botAI->IsTank(bot))
        return 1.0f;

    if (dynamic_cast<FathomLordKarathressMisdirectBossesToTanksAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 8;

    auto it = karathressDpsWaitTimer.find(karathress->GetMapId());
    if (it == karathressDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        if (dynamic_cast<AttackAction*>(action) ||
            (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier::GetValue(Action* action)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis || !caribdis->IsAlive())
        return 1.0f;

    if (botAI->IsHealAssistantOfIndex(bot, 0))
    {
        if (dynamic_cast<FleeAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;
    }

    return 1.0f;
}

// Use Bloodlust/Heroism after the first Murloc spawn
float MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return 1.0f;

    Unit* murloc = AI_VALUE2(Unit*, "find target", "tidewalker lurker");
    if (!murloc)
    {
        if (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float MorogrimTidewalkerDisablePhase2FleeActionMultiplier::GetValue(Action* action)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return 1.0f;

    if (tidewalker->GetHealthPct() < 25.0f)
    {
        if (dynamic_cast<FleeAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Wait until phase 3 to use Bloodlust/Heroism
float LadyVashjDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    if (!IsLadyVashjInPhase3(botAI))
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float LadyVashjStaticChargeStayAwayFromGroupMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || IsLadyVashjInPhase2(botAI))
        return 1.0f;

    if (!botAI->IsMainTank(bot) && bot->HasAura(SPELL_STATIC_CHARGE))
    {
        if ((dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<LadyVashjStaticChargeMoveAwayFromGroupAction*>(action)) ||
            dynamic_cast<CastKillingSpreeAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// If raid cheat (which enables bot looting of the core) is not enabled
// Bots should not loot the core
float LadyVashjDoNotLootTheTaintedCoreMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || !botAI->HasCheat(BotCheatMask::raid))
        return 1.0f;

    if (dynamic_cast<LootAction*>(action))
        return 0.0f;

    return 1.0f;
}

// All of phase 2 and 3 require a custom movement and targeting system
// So the standard target selection system must be disabled
float LadyVashjDisableAutomaticTargetingAndMovementModifier::GetValue(Action *action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (IsLadyVashjInPhase2(botAI))
    {
        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action))
            return 0.0f;

        if (!botAI->IsHeal(bot) && dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;

        Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
        if (enchanted && enchanted->IsAlive() && bot->GetVictim() == enchanted)
        {
            if (dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
                return 0.0f;
        }
    }

    if (IsLadyVashjInPhase3(botAI))
    {
        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action))
            return 0.0f;

        Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
        Unit* elite = AI_VALUE2(Unit*, "find target", "coilfang elite");
        Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");

        if (enchanted && enchanted->IsAlive())
        {
            if (bot->GetVictim() == enchanted)
            {
                if (dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
                return 0.0f;
            }
        }

        if ((!enchanted || !enchanted->IsAlive()) && (!strider || !strider->IsAlive()) &&
            (!elite || !elite->IsAlive()))
        {
            if (dynamic_cast<SetBehindTargetAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}
