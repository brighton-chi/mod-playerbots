/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTStrategy.h"
#include "BTHelpers.h"
#include "BTMultipliers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"

using namespace BtHelpers;
using namespace EncounterHelpers;

void RaidBlackTempleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("black temple no encounter in progress",
        { NextAction("black temple reset encounter states", ACTION_EMERGENCY + 10) }));

    // Trash
    triggers.push_back(new TriggerNode("sister of pleasure should be marked",
        { NextAction("mark sister of pleasure", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("shadowmoon reaver wand builds charges",
        { NextAction("shadowmoon reaver stop wand", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("shadowmoon reaver should control caster pet",
        { NextAction("shadowmoon reaver control caster pet", ACTION_EMERGENCY + 1) }));

    // High Warlord Naj'entus
    triggers.push_back(new TriggerNode("high warlord naj'entus hunter should misdirect",
        { NextAction("high warlord naj'entus misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus should be tanked",
        { NextAction("high warlord naj'entus tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus ranged should spread",
        { NextAction("high warlord naj'entus disperse ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus impaled player needs remover",
        { NextAction("high warlord naj'entus assign spine remover", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus impaling spine on group member",
        { NextAction("high warlord naj'entus remove impaling spine", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus needs spine thrower",
        { NextAction("high warlord naj'entus assign spine thrower", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus has tidal shield",
        { NextAction("high warlord naj'entus throw impaling spine", ACTION_RAID + 2) }));

    // Supremus
    triggers.push_back(new TriggerNode("supremus hunter should misdirect",
        { NextAction("supremus misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("supremus ranged should spread",
        { NextAction("supremus disperse ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("supremus fixated on bot",
        { NextAction("supremus kite boss", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("supremus near fire",
        { NextAction("supremus move away from fire", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("supremus reach blocked by fire",
        { NextAction("supremus reach around fire", ACTION_RAID - 1) }));

    // Shade of Akama
    triggers.push_back(new TriggerNode("shade of akama should prioritize channelers",
        { NextAction("shade of akama melee dps prioritize channelers", ACTION_RAID + 1) }));

    // Teron Gorefiend
    triggers.push_back(new TriggerNode("teron gorefiend hunter should misdirect",
        { NextAction("teron gorefiend misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("teron gorefiend should be tanked",
        { NextAction("teron gorefiend tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("teron gorefiend ranged should position on balcony",
        { NextAction("teron gorefiend position ranged on balcony", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("teron gorefiend casts shadow of death",
        { NextAction("teron gorefiend avoid shadow of death", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("teron gorefiend should position for vengeful spirit",
        { NextAction("teron gorefiend move to corner to die", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("teron gorefiend transformed into vengeful spirit",
        { NextAction(
            "teron gorefiend control and destroy shadowy constructs", ACTION_EMERGENCY + 10) }));

    // Gurtogg Bloodboil
    triggers.push_back(new TriggerNode("gurtogg bloodboil hunter should misdirect",
        { NextAction("gurtogg bloodboil misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil should be tanked",
        { NextAction("gurtogg bloodboil tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil should position for bloodboil",
        { NextAction("gurtogg bloodboil rotate ranged groups", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil melee should stay behind",
        { NextAction("gurtogg bloodboil melee move behind", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil should avoid fel rage target",
        { NextAction("gurtogg bloodboil avoid fel rage target", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil fel rage on bot",
        { NextAction("gurtogg bloodboil lead boss to tank position", ACTION_RAID + 1) }));

    // Reliquary of Souls
    triggers.push_back(new TriggerNode("reliquary of souls hunter should misdirect",
        { NextAction("reliquary of souls misdirect to main tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("reliquary of souls should position for suffering",
        { NextAction("reliquary of souls adjust distance from suffering", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("reliquary of souls healers should attack suffering",
        { NextAction("reliquary of souls healers dps suffering", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("reliquary of souls essence of desire has rune shield",
        { NextAction("reliquary of souls spellsteal rune shield", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("reliquary of souls essence of desire casts deaden",
        { NextAction("reliquary of souls spell reflect deaden", ACTION_EMERGENCY + 6) }));

    // Mother Shahraz
    triggers.push_back(new TriggerNode("mother shahraz hunter should misdirect",
        { NextAction("mother shahraz misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("mother shahraz should be tanked",
        { NextAction("mother shahraz tanks position boss under pillar", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("mother shahraz tanks are positioning boss",
        { NextAction("mother shahraz melee dps wait at safe position", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("mother shahraz ranged should position under pillar",
        { NextAction("mother shahraz position ranged under pillar", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("mother shahraz fatal attraction",
        { NextAction(
            "mother shahraz break fatal attraction", ACTION_EMERGENCY + 10) }));

    // Illidari Council
    triggers.push_back(new TriggerNode("illidari council hunter should misdirect",
        { NextAction("illidari council misdirect to tanks", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("illidari council gathios should be tanked",
        { NextAction("illidari council main tank position gathios", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council gathios casts judgement of command",
        { NextAction(
            "illidari council main tank reflect judgement of command", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidari council malande should be tanked",
        { NextAction("illidari council first assist tank focus malande", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council darkshadow should be tanked",
        { NextAction(
            "illidari council second assist tank position darkshadow", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council zerevor should be tanked by mage",
        { NextAction("illidari council mage tank position zerevor", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("illidari council mage tank needs dedicated healer",
        { NextAction("illidari council position mage tank healer", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council ranged should spread",
        { NextAction("illidari council disperse ranged", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidari council pets screw up the pull",
        { NextAction("illidari council command pets to attack gathios", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidari council should assign dps priority",
        { NextAction("illidari council assign dps targets", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council should manage dps timer",
        { NextAction("illidari council manage dps timer", ACTION_EMERGENCY + 10) }));

    // Illidan Stormrage <The Betrayer>
    triggers.push_back(new TriggerNode("illidan stormrage hunter should misdirect",
        { NextAction("illidan stormrage misdirect to tanks", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidan stormrage casts flame crash",
        { NextAction("illidan stormrage main tank reposition boss", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage parasitic shadowfiend on group member",
        { NextAction("illidan stormrage isolate bot with parasite", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidan stormrage parasitic shadowfiends run wild",
        { NextAction("illidan stormrage set earthbind totem", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage flames of azzinoth should be tanked",
        { NextAction(
            "illidan stormrage assist tanks handle flames of azzinoth", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage pets die to fire",
        { NextAction("illidan stormrage control pet aggression", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("illidan stormrage grate is safe from flames",
        { NextAction("illidan stormrage position above grate", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage dark barrage on immunity class",
        { NextAction("illidan stormrage remove dark barrage", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("illidan stormrage prepares to land",
        { NextAction("illidan stormrage move away from landing point", ACTION_EMERGENCY + 3) }));

    triggers.push_back(new TriggerNode("illidan stormrage ranged should spread",
        { NextAction("illidan stormrage disperse ranged", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage this expansion hates melee",
        { NextAction("illidan stormrage melee go somewhere to not die", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage warlock should tank demon form",
        { NextAction("illidan stormrage warlock tank handle demon boss", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("illidan stormrage should assign dps priority",
        { NextAction("illidan stormrage dps prioritize adds", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage maiev placed shadow trap",
        { NextAction("illidan stormrage use shadow trap", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage should manage dps timer and rti",
        { NextAction("illidan stormrage manage dps timer and rti", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("illidan stormrage should clear hazards between phases",
        { NextAction("illidan stormrage destroy hazards", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("illidan stormrage cheat",
        { NextAction("illidan stormrage handle adds cheat", ACTION_EMERGENCY + 10) }));
}

void RaidBlackTempleStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Shared
    multipliers.push_back(new BlackTempleDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new BlackTempleDisableKillingSpreeMultiplier(botAI));

    // Trash
    multipliers.push_back(new ShadowmoonReaverHoldChargeBuildingSpellsMultiplier(botAI));

    // High Warlord Naj'entus
    multipliers.push_back(new HighWarlordNajentusDisableCombatFormationMoveMultiplier(botAI));

    // Supremus
    multipliers.push_back(new SupremusFocusOnAvoidanceInKitePhaseMultiplier(botAI));
    multipliers.push_back(new SupremusDoNotReachThroughFireMultiplier(botAI));
    multipliers.push_back(new SupremusDisableAvoidAoeInKitePhaseMultiplier(botAI));

    // Shade of Akama
    multipliers.push_back(new ShadeOfAkamaDontDropOutOfSightTargetMultiplier(botAI));

    // Teron Gorefiend
    multipliers.push_back(new TeronGorefiendControlMovementMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendMarkedBotOnlyMoveToDieMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendSpiritsAttackShadowyConstructsMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendDisableAttackingConstructsMultiplier(botAI));

    // Gurtogg Bloodboil
    multipliers.push_back(new GurtoggBloodboilControlMovementMultiplier(botAI));
    multipliers.push_back(new GurtoggBloodboilNoThreatRedirectsDuringFelRageMultiplier(botAI));
    multipliers.push_back(new GurtoggBloodboilFelRageTargetHoldsAggroMultiplier(botAI));
    multipliers.push_back(new GurtoggBloodboilHoldThreatMultiplier(botAI));

    // Reliquary of Souls
    multipliers.push_back(new ReliquaryOfSoulsDontWasteHealingMultiplier(botAI));
    multipliers.push_back(new ReliquaryOfSoulsControlMeleeMovementMultiplier(botAI));
    multipliers.push_back(new ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier(botAI));

    // Mother Shahraz
    multipliers.push_back(new MotherShahrazControlMovementMultiplier(botAI));
    multipliers.push_back(new MotherShahrazFatalAttractionRunAwayMultiplier(botAI));

    // Illidari Council
    multipliers.push_back(new IllidariCouncilDisableMeleeTankActionsMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilDisableMageTankActionsMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilControlNonTankMovementMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilDisableArcaneShotOnZerevorMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilWaitForDpsMultiplier(botAI));

    // Illidan Stormrage <The Betrayer>
    multipliers.push_back(new IllidanStormrageControlTankActionsMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageDisableDefaultTargetingMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageControlNonTankMovementMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageUseEarthbindTotemMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageWaitForDpsMultiplier(botAI));
}

namespace
{

void AppendShadowmoonReaverExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    Player* bot = botAI->GetBot();
    if (!PlayerbotAI::IsCaster(bot) && !PlayerbotAI::IsHeal(bot))
        return;

    auto const& reavers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shadowmoon reavers")->RefGet();
    for (ObjectGuid const& guid : reavers)
    {
        if (IsShadowmoonReaverUnsafeForMagic(botAI->GetUnit(guid)))
            exclusions.insert(guid);
    }
}

void AppendShadeOfAkamaTankExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    if (!PlayerbotAI::IsTank(botAI->GetBot()))
        return;

    auto const& adds =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shade of akama adds")->RefGet();
    exclusions.insert(adds.begin(), adds.end());
}

void AppendShadowyConstructExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    if (botAI->GetBot()->HasAura(Id(BtSpells::SPELL_SPIRITUAL_VENGEANCE)))
        return;

    auto const& constructs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shadowy constructs")->RefGet();
    exclusions.insert(constructs.begin(), constructs.end());
}

} // end anonymous namespace

void RaidBlackTempleStrategy::AppendTargetExclusions(
    GuidSet& exclusions, TargetValueExclusionType type)
{
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != BT_MAP_ID)
        return;

    if (type != TargetValueExclusionType::TankTarget)
        AppendShadowmoonReaverExclusions(botAI, exclusions);

    if (!IsEncounterInProgress(bot, BT_MAP_ID))
        return;

    if (type == TargetValueExclusionType::TankTarget)
        AppendShadeOfAkamaTankExclusions(botAI, exclusions);

    AppendShadowyConstructExclusions(botAI, exclusions);
}
