#include "RaidSSCMultipliers.h"
#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "ChooseTargetActions.h"
#include "DestroyItemAction.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "LootAction.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"

using namespace SerpentShrineCavernHelpers;

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

// Trash

float ColossusRagerDoNotUseBloodlustOrHeroismMultiplier::GetValue(Action* action)
{
    Unit* rager = AI_VALUE2(Unit*, "find target", "colossus rager");
    if (!rager)
        return 1.0f;

    if (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Hydross the Unstable <Duke of Currents>

float HydrossTheUnstableDisableTankActionsMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross");
    if (!hydross || dynamic_cast<WipeAction*>(action))
        return 1.0f;

    if (botAI->IsMainTank(bot))
    {
        if (dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;

        if (hydross->HasAura(SPELL_CORRUPTION))
        {
            if (!dynamic_cast<HydrossTheUnstablePositionFrostTankAction*>(action))
                return 0.0f;
        }
    }

    if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;

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

    const uint32 mapId = hydross->GetMapId();
    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 5;
    const uint8 phaseChangeWaitSeconds = 6;

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        auto itDps = hydrossFrostDpsWaitTimer.find(mapId);
        auto itPhase = hydrossChangeToFrostPhaseTimer.find(mapId);

        bool justChanged = (itDps == hydrossFrostDpsWaitTimer.end() ||
                            (now - itDps->second) < dpsWaitSeconds);

        bool aboutToChange = (itPhase != hydrossChangeToFrostPhaseTimer.end() &&
                              (now - itPhase->second) > phaseChangeWaitSeconds);

        if (justChanged || aboutToChange)
        {
            if ((!botAI->IsMainTank(bot) && dynamic_cast<AttackAction*>(action)) ||
                (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION))
    {
        auto itDps = hydrossNatureDpsWaitTimer.find(mapId);
        auto itPhase = hydrossChangeToNaturePhaseTimer.find(mapId);

        bool justChanged = (itDps == hydrossNatureDpsWaitTimer.end() ||
                            (std::time(nullptr) - itDps->second) < dpsWaitSeconds);

        bool aboutToChange = (itPhase != hydrossChangeToNaturePhaseTimer.end() &&
                              (std::time(nullptr) - itPhase->second) > phaseChangeWaitSeconds);

        if (justChanged || aboutToChange)
        {
            if ((!botAI->IsAssistTankOfIndex(bot, 0) && dynamic_cast<AttackAction*>(action)) ||
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

    const uint32 mapId = lurker->GetMapId();
    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(mapId);
    if (it != lurkerSpoutTimer.end() && it->second > now)
    {
        if (IsChargeAction(action) || dynamic_cast<CastKillingSpreeAction*>(action) ||
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
        if (IsChargeAction(action) || (!botAI->IsTank(bot) && dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<LeotherasTheBlindRunAwayFromWhirlwindAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float LeotherasTheBlindDisableTankActionsMultiplier::GetValue(Action* action)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon || dynamic_cast<LeotherasTheBlindInnerDemonCheatAction*>(action) ||
        dynamic_cast<WipeAction*>(action))
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action) || dynamic_cast<CastShadowWardAction*>(action))
        return 0.0f;

    Unit* leotherasDemonPhase2 = GetPhase2LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (botAI->IsTank(bot) && bot != demonFormTank && leotherasDemonPhase2)
        return 0.0f;

    return 1.0f;
}

//
float LeotherasTheBlindMainTankMaintainDemonFormPositionMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (!leotherasDemon || (demonFormTank && demonFormTank->getClass() != CLASS_WARLOCK))
        return 1.0f;

    if ((botAI->IsMainTank(bot) || botAI->IsTank(bot) && leotherasDemon->GetVictim() == bot) &&
        (dynamic_cast<MovementAction*>(action) && !dynamic_cast<LeotherasTheBlindDemonFormTankAttackBossAction*>(action)))
        return 0.0f;

    return 1.0f;
}

// Applies only if there is no Warlock tank
float LeotherasTheBlindDemonFormDisableMeleeActionsMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (!leotherasPhase2Demon || leotherasPhase2Demon->HasAura(SPELL_LEOTHERAS_BANISHED) ||
        botAI->IsRanged(bot) || leotherasPhase2Demon->GetVictim() == bot ||
        botAI->IsMainTank(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER) ||
        (demonFormTank && demonFormTank->getClass() != CLASS_WARLOCK))
        return 1.0f;

    if ((dynamic_cast<MovementAction*>(action) && !dynamic_cast<LeotherasTheBlindDemonFormPositionMeleeAction*>(action)) ||
        dynamic_cast<CastKillingSpreeAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Applies only if there is no Warlock tank
float LeotherasTheBlindWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    const uint32 mapId = leotheras->GetMapId();
    const time_t now = std::time(nullptr);

    const uint8 dpsWaitSecondsPhase1 = 5;
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (leotherasHuman && !leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED) && !leotherasPhase3Demon)
    {
        auto it = leotherasHumanFormDpsWaitTimer.find(mapId);
        if (it == leotherasHumanFormDpsWaitTimer.end() || (now - it->second) < dpsWaitSecondsPhase1)
        {
            if ((!botAI->IsTank(bot) && dynamic_cast<AttackAction*>(action)) ||
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
        if (demonFormTank == bot)
            return 1.0f;

        auto it = leotherasFinalPhaseDpsWaitTimer.find(mapId);
        if (it == leotherasFinalPhaseDpsWaitTimer.end() || (now - it->second) < dpsWaitSecondsPhase3)
        {
            if ((!botAI->IsTank(bot) && dynamic_cast<AttackAction*>(action)) ||
                (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    return 1.0f;
}

float LeotherasTheBlindDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasPhase3Demon && (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action)))
        return 0.0f;

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

    const uint32 mapId = karathress->GetMapId();
    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 8;

    auto it = karathressDpsWaitTimer.find(mapId);
    if (it == karathressDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        if ((!botAI->IsTank(bot) && dynamic_cast<AttackAction*>(action)) ||
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

    if (botAI->IsHealAssistantOfIndex(bot, 0) &&
        (dynamic_cast<FleeAction*>(bot) || dynamic_cast<FollowAction*>(bot)))
        return 0.0f;

    return 1.0f;
}

float MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return 1.0f;

    Unit* murloc = AI_VALUE2(Unit*, "find target", "tidewalker lurker");
    if (!murloc && (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action)))
        return 0.0f;

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

float LadyVashjDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    if (!IsLadyVashjInPhase3(botAI) &&
        (dynamic_cast<CastHeroismAction*>(action) ||
         dynamic_cast<CastBloodlustAction*>(action)))
        return 0.0f;

    /* if (IsLadyVashjInPhase1(botAI) && dynamic_cast<CastMetamorphosisAction*>(action))
        return 0.0f; */

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
            IsChargeAction(action))
            return 0.0f;
    }

    return 1.0f;
}

float LadyVashjDoNotLootTheTaintedCoreMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || !botAI->HasCheat(BotCheatMask::raid))
        return 1.0f;

    if (dynamic_cast<LootAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LadyVashjCorePassersPrioritizePositioningMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    Group* group = bot->GetGroup();
    Player* master = botAI->GetMaster();
    if (!IsLadyVashjInPhase2(botAI) || !group || !master /* || dynamic_cast<WipeAction*>(action) ||
        dynamic_cast<DestroyItemAction*>(action) || dynamic_cast<StoreLootAction*>(action) ||
        dynamic_cast<LadyVashjCheatToTestAction*>(action) */)
        return 1.0f;

    Player* designatedLooter = GetDesignatedCoreLooter(group, master, botAI);
    Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
    Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(group, botAI);

    if (bot == designatedLooter || bot == firstCorePasser || bot == secondCorePasser ||
        bot == thirdCorePasser || bot == fourthCorePasser)
    {
        auto hasCore = [](Player* player) { return player && player->HasItemCount(ITEM_TAINTED_CORE, 1, false); };

        if (bot == designatedLooter && (hasCore(firstCorePasser) || hasCore(secondCorePasser) ||
            hasCore(thirdCorePasser) || hasCore(fourthCorePasser)))
            return 1.0f;
        else if (bot == firstCorePasser && (hasCore(secondCorePasser) || hasCore(thirdCorePasser) ||
            hasCore(fourthCorePasser)))
            return 1.0f;
        else if (bot == secondCorePasser && (hasCore(thirdCorePasser) || hasCore(fourthCorePasser)))
            return 1.0f;
        else if (bot == thirdCorePasser && hasCore(fourthCorePasser))
            return 1.0f;

        const uint32 mapId = vashj->GetMapId();
        // query helper once for recent paralyze (default grace = 3s)
        bool recentParalyze = AnyRecentParalyze(group, mapId);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            // if any recent paralyze (or the member currently has paralyze),
            // movement actions (except the pass-the-core action) should be deprioritized.
            if (recentParalyze || member->HasAura(SPELL_PARALYZE))
            {
                if (dynamic_cast<MovementAction*>(action) &&
                    !dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action))
                    return 0.0f;
            }
        }
    }

    return 1.0f;
}

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
        if (enchanted && enchanted->IsAlive() && bot->GetVictim() == enchanted &&
            dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
            return 0.0f;
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
            if (bot->GetVictim() == enchanted && dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
                return 0.0f;
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
