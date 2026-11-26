/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAMULTIPLIERS_H
#define PLAYERBOTS_ZAMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "ZAHelpers.h"
#include <optional>
#include <string>

// Shared

class ZulAmanEncounterMultiplier : public Multiplier
{
public:
    ZulAmanEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    ZulAmanEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name, ZaHelpers::ZaBosses boss)
        : Multiplier(botAI, name), _boss(boss) {}

    float GetValue(Action* action) final
    {
        bool const inProgress = _boss ?
            EncounterHelpers::IsBossInProgress(
                bot, ZaHelpers::ZA_MAP_ID, ZaHelpers::Id(*_boss)) :
            EncounterHelpers::IsEncounterInProgress(bot, ZaHelpers::ZA_MAP_ID);
        return inProgress ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
    Unit* GetEncounterBoss()
    {
        return _boss ? ZaHelpers::GetZaBoss(bot, *_boss) : nullptr;
    }

private:
    std::optional<ZaHelpers::ZaBosses> const _boss;
};

class ZulAmanDelayDpsCooldownsMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZulAmanDisableTankActionsMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman disable tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZulAmanControlMisdirectionMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZulAmanDisableCombatFormationMoveMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman disable combat formation move") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZulAmanAvoidWhirlwindMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman avoid whirlwind") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Akil'zon <Eagle Avatar>

class AkilzonStayInEyeOfTheStormMultiplier : public ZulAmanEncounterMultiplier
{
public:
    AkilzonStayInEyeOfTheStormMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(
            botAI, "akil'zon stay in eye of the storm", ZaHelpers::ZaBosses::DATA_AKILZON) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Nalorakk <Bear Avatar>
// N/A

// Jan'alai <Dragonhawk Avatar>

class JanalaiStayAwayFromFireBombsMultiplier : public ZulAmanEncounterMultiplier
{
public:
    JanalaiStayAwayFromFireBombsMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(
            botAI, "jan'alai stay away from fire bombs", ZaHelpers::ZaBosses::DATA_JANALAI) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class JanalaiDoNotCrowdControlHatchersMultiplier : public ZulAmanEncounterMultiplier
{
public:
    JanalaiDoNotCrowdControlHatchersMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(
            botAI, "jan'alai do not crowd control hatchers", ZaHelpers::ZaBosses::DATA_JANALAI) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Halazzi <Lynx Avatar>

class HalazziDisableAutoDpsTargetingMultiplier : public ZulAmanEncounterMultiplier
{
public:
    HalazziDisableAutoDpsTargetingMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(
            botAI, "halazzi disable auto dps targeting", ZaHelpers::ZaBosses::DATA_HALAZZI) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Hex Lord Malacrass

class HexLordMalacrassUnstableAfflictionMultiplier : public ZulAmanEncounterMultiplier
{
public:
    HexLordMalacrassUnstableAfflictionMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(
            botAI, "hex lord malacrass unstable affliction",
            ZaHelpers::ZaBosses::DATA_HEX_LORD_MALACRASS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HexLordMalacrassSpellReflectionMultiplier : public ZulAmanEncounterMultiplier
{
public:
    HexLordMalacrassSpellReflectionMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(
            botAI, "hex lord malacrass spell reflection",
            ZaHelpers::ZaBosses::DATA_HEX_LORD_MALACRASS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HexLordMalacrassStayAwayFromFreezingTrapMultiplier : public ZulAmanEncounterMultiplier
{
public:
    HexLordMalacrassStayAwayFromFreezingTrapMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(
            botAI, "hex lord malacrass stay away from freezing trap",
            ZaHelpers::ZaBosses::DATA_HEX_LORD_MALACRASS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Zul'jin

class ZuljinNoAttackingDuringPhaseChangeMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZuljinNoAttackingDuringPhaseChangeMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(
            botAI, "zul'jin no attacking during phase change",
            ZaHelpers::ZaBosses::DATA_ZULJIN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZuljinEagleDisableAvoidAoeMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZuljinEagleDisableAvoidAoeMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(
            botAI, "zul'jin eagle disable avoid aoe", ZaHelpers::ZaBosses::DATA_ZULJIN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
