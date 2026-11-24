/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"
#include "ZAActions.h"
#include "ZAHelpers.h"
#include <utility>

using namespace ZaHelpers;
using namespace EncounterHelpers;

namespace
{

bool IsApproachMovement(Action* action)
{
    return dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action);
        // CombatFormationMoveAction is the parent of SetBehindTargetAction, which circles around
        // targets when they are near, even if the bot is otherwise held in position.
}

bool IsHazardousMovement(Action* action)
{
    return (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action)) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action);
}

// The boss in progress, with null for the unit if the bot can't find it.
std::pair<Unit*, ZaBosses> GetZulAmanBoss(Player* bot)
{
    std::optional<uint32> const bossId = GetBossInProgress(bot, ZA_MAP_ID);
    if (!bossId)
        return { nullptr, ZaBosses::DATA_AKILZON };

    ZaBosses const boss = static_cast<ZaBosses>(*bossId);
    return { GetZaBoss(bot, boss), boss };
}

}

// Shared

float ZulAmanDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    // If the boss cannot be found but the encounter is in progress, it is probably because the bot
    // was rezzed and zone combat hasn't reapplied, so block cooldowns.
    auto const [boss, bossKey] = GetZulAmanBoss(bot);
    if (!boss || boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 0.0f;

    // Further restrictions on Bloodlust for Zul'jin and Jan'alai.
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastBloodlustAction*>(action) &&
        !dynamic_cast<CastHeroismAction*>(action))
    {
        return 1.0f;
    }

    // Zul'jin: hold until Phase 3 (or later).
    if (bossKey == ZaBosses::DATA_ZULJIN)
    {
        return (boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ||
            boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_LYNX)) ||
            boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK))) ? 1.0f : 0.0f;
    }

    // Jan'alai: hold until time to burn Hatchlings (see comments to the constants in ZAHelpers.h).
    if (bossKey == ZaBosses::DATA_JANALAI)
    {
        if (boss->GetHealthPct() <= JANALAI_HATCH_ALL_HEALTH_PCT)
            return 1.0f;

        return CountJanalaiHatchlingsByEntry(bot) >= JANALAI_BLOODLUST_HATCHLING_COUNT ?
            1.0f : 0.0f;
    }

    return 1.0f;
}

// Malacrass siphoning a Warrior soul and Zul'jin in troll form have very similar Whirlwinds.
float ZulAmanAvoidWhirlwindMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsApproachMovement(action) && !dynamic_cast<CastKillingSpreeAction*>(action))
        return 1.0f;

    Unit* boss = nullptr;
    uint32 whirlwind = 0;
    if (Unit* zuljin = GetZaBoss(bot, ZaBosses::DATA_ZULJIN))
    {
        boss = zuljin;
        whirlwind = Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND);
    }
    else if (Unit* malacrass = GetZaBoss(bot, ZaBosses::DATA_HEX_LORD_MALACRASS))
    {
        boss = malacrass;
        whirlwind = Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND);
    }

    if (!boss)
        return 1.0f;

    if (boss->GetVictim() == bot)
        return 1.0f;

    if (!boss->HasAura(whirlwind))
        return 1.0f;

    return bot->GetExactDist2d(boss) <= ZA_WHIRLWIND_HOLD_DISTANCE ? 0.0f : 1.0f;
}

float ZulAmanDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isTankFace = dynamic_cast<TankFaceAction*>(action);
    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action);
    bool const isTaunt = IsTauntAction(bot, action);

    if (!isTankFace && !isTankAssist && !isTaunt)
        return 1.0f;

    auto const [boss, bossKey] = GetZulAmanBoss(bot);
    if (!boss)
        return 1.0f;

    // Nalorakk: A tank swap is used by form, so only the tank assigned to the current form may
    // taunt. Same as NalorakkTanksPositionBossAction: main tank gets bear, assist tank gets troll.
    if (bossKey == ZaBosses::DATA_NALORAKK)
    {
        if (!isTaunt)
            return 0.0f;

        bool const isFormTank = IsNalorakkInBearForm(boss)
            ? PlayerbotAI::IsMainTank(bot) : PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);

        return isFormTank ? 1.0f : 0.0f;
    }

    // Halazzi: The assist tank picks up the lynx so disable the main tank from taunting it.
    if (bossKey == ZaBosses::DATA_HALAZZI)
    {
        if (!isTaunt)
            return 0.0f;

        if (!PlayerbotAI::IsMainTank(bot))
            return 1.0f;

        Unit* target = action->GetTarget();
        return target && target->GetEntry() == Id(ZaNpcs::NPC_SPIRIT_OF_THE_LYNX) ? 0.0f : 1.0f;
    }

    if (isTaunt)
        return 1.0f;

    // Zul'jin: Allow tank face in Phase 5 for the tank to turn him away from the raid.
    if (bossKey == ZaBosses::DATA_ZULJIN)
    {
        return isTankFace &&
            !boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK)) ? 0.0f : 1.0f;
    }

    // Jan'alai: Allow tank assist for the assist tank to pick up the Hatchlings. Tank face is
    // already disabled as part of ZulAmanDisableCombatFormationMoveMultiplier so the addition here
    // is just belt-and-suspenders for no cost.
    if (bossKey == ZaBosses::DATA_JANALAI)
        return isTankFace || PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;

    // Akil'zon disables only tank face action, and that is already handled by
    // ZulAmanDisableCombatFormationMoveMultiplier.
    return 1.0f;
}

// Nalorakk's troll form and Halazzi's lynx add need to be handled by the assist tank.
float ZulAmanControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return IsBossInProgress(bot, ZA_MAP_ID, Id(ZaBosses::DATA_NALORAKK)) ||
        IsBossInProgress(bot, ZA_MAP_ID, Id(ZaBosses::DATA_HALAZZI)) ? 0.0f : 1.0f;
}

// CombatFormationMoveAction is the action for the "disperse" command.
float ZulAmanDisableCombatFormationMoveMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    auto const [boss, bossKey] = GetZulAmanBoss(bot);
    if (!boss)
        return 1.0f;

    if (bossKey == ZaBosses::DATA_ZULJIN)
        return boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ? 0.0f : 1.0f;

    return bossKey == ZaBosses::DATA_JANALAI || bossKey == ZaBosses::DATA_AKILZON ? 0.0f : 1.0f;
}

// Akil'zon <Eagle Avatar>

float AkilzonStayInEyeOfTheStormMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsHazardousMovement(action))
        return 1.0f;

    if (dynamic_cast<AkilzonMoveToEyeOfTheStormAction*>(action))
        return 1.0f;

    std::optional<uint32> const& stormTimer = ZaState(bot->GetInstanceId()).akilzonStormTimer;
    if (!stormTimer)
        return 1.0f;

    return IsInStormWindow(*stormTimer) ? 0.0f : 1.0f;
}

// Nalorakk <Bear Avatar>

// N/A

// Jan'alai <Dragonhawk Avatar>

float JanalaiStayAwayFromFireBombsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsHazardousMovement(action))
        return 1.0f;

    if (dynamic_cast<JanalaiAvoidFireBombsAction*>(action))
        return 1.0f;

    Unit* janalai = GetEncounterBoss();
    if (!janalai)
        return 1.0f;

    return IsJanalaiBombing(janalai) ? 0.0f : 1.0f;
}

float JanalaiDoNotCrowdControlHatchersMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastCrowdControlSpellAction*>(action))
        return 1.0f;

    Unit* target = action->GetTarget();
    return target && target->GetEntry() == Id(ZaNpcs::NPC_AMANISHI_HATCHER) ? 0.0f : 1.0f;
}

// Halazzi <Lynx Avatar>

float HalazziDisableAutoDpsTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    return dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ? 0.0f : 1.0f;
}

// Hex Lord Malacrass

// Weirdly, Unstable Affliction is considered a magic effect, not a curse.
float HexLordMalacrassUnstableAfflictionMultiplier::GetValueInEncounter(Action* action)
{
    switch (bot->getClass())
    {
        case CLASS_PRIEST:
        {
            if (!dynamic_cast<CastDispelMagicOnPartyAction*>(action) &&
                !dynamic_cast<CastDispelMagicAction*>(action))
            {
                return 1.0f;
            }
            break;
        }
        case CLASS_PALADIN:
        {
            // Cleanse also dispels magic when it is cast for a poison or disease.
            if (!dynamic_cast<CastCleanseMagicOnPartyAction*>(action) &&
                !dynamic_cast<CastCleanseMagicAction*>(action) &&
                !dynamic_cast<CastCleansePoisonOnPartyAction*>(action) &&
                !dynamic_cast<CastCleansePoisonAction*>(action) &&
                !dynamic_cast<CastCleanseDiseaseOnPartyAction*>(action) &&
                !dynamic_cast<CastCleanseDiseaseAction*>(action))
            {
                return 1.0f;
            }
            break;
        }
        default:
            return 1.0f;
    }

    Unit* target = action->GetTarget();
    return target && target->HasAura(Id(ZaSpells::SPELL_UNSTABLE_AFFLICTION)) ? 0.0f : 1.0f;
}

float HexLordMalacrassSpellReflectionMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsCaster(bot))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    Unit* malacrass = GetEncounterBoss();
    if (!malacrass)
        return 1.0f;

    return malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_SPELL_REFLECTION)) ? 0.0f : 1.0f;
}

float HexLordMalacrassStayAwayFromFreezingTrapMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsApproachMovement(action))
        return 1.0f;

    return GetNearbyFreezingTrap(botAI) ? 0.0f : 1.0f;
}

// Zul'jin

float ZuljinNoAttackingDuringPhaseChangeMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    // Above 80% is Phase 1.
    Unit* zuljin = GetEncounterBoss();
    if (!zuljin || zuljin->GetHealthPct() > 80.0f)
        return 1.0f;

    // After 80%, Zul'jin drops into troll form only during transformation sequences.
    bool const isZuljinTransformed =
        zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_BEAR)) ||
        zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ||
        zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_LYNX)) ||
        zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));

    return isZuljinTransformed ? 1.0f : 0.0f;
}

float ZuljinEagleDisableAvoidAoeMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    Unit* zuljin = GetEncounterBoss();
    if (!zuljin)
        return 1.0f;

    return zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ? 0.0f : 1.0f;
}
