#ifndef _PLAYERBOT_RAIDZULAMANTRIGGERS_H
#define _PLAYERBOT_RAIDZULAMANTRIGGERS_H

#include "Trigger.h"

class AkilzonPullingBossTrigger : public Trigger
{
public:
    AkilzonPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "akilzon pulling boss") {}
    bool IsActive() override;
};

class AkilzonBossCastsStaticDisruptionTrigger : public Trigger
{
public:
    AkilzonBossCastsStaticDisruptionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "akilzon boss casts static disruption") {}
    bool IsActive() override;
};

class AkilzonElectricalStormHasFormedTrigger : public Trigger
{
public:
    AkilzonElectricalStormHasFormedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "akilzon electrical storm has formed") {}
    bool IsActive() override;
};

class NalorakkPullingBossTrigger : public Trigger
{
public:
    NalorakkPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nalorakk pulling boss") {}
    bool IsActive() override;
};

class NalorakkBossSwitchesFormsTrigger : public Trigger
{
public:
    NalorakkBossSwitchesFormsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nalorakk boss switches forms") {}
    bool IsActive() override;
};

class NalorakkBossCastsSurgeTrigger : public Trigger
{
public:
    NalorakkBossCastsSurgeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nalorakk boss casts surge") {}
    bool IsActive() override;
};

class JanalaiPullingBossTrigger : public Trigger
{
public:
    JanalaiPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "janalai pulling boss") {}
    bool IsActive() override;
};

class JanalaiBossEngagedByMainTankTrigger : public Trigger
{
public:
    JanalaiBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "janalai boss engaged by main tank") {}
    bool IsActive() override;
};

class JanalaiBossCastsFlameBreathTrigger : public Trigger
{
public:
    JanalaiBossCastsFlameBreathTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "janalai boss casts flame breath") {}
    bool IsActive() override;
};

class JanalaiBossSummoningFireBombsTrigger : public Trigger
{
public:
    JanalaiBossSummoningFireBombsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "janalai boss summoning fire bombs") {}
    bool IsActive() override;
};

class HalazziPullingBossTrigger : public Trigger
{
public:
    HalazziPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "halazzi pulling boss") {}
    bool IsActive() override;
};

class HalazziBossEngagedByMainTankTrigger : public Trigger
{
public:
    HalazziBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "halazzi boss engaged by main tank") {}
    bool IsActive() override;
};

class HalazziBossSummonsSpiritLynxTrigger : public Trigger
{
public:
    HalazziBossSummonsSpiritLynxTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "halazzi boss summons spirit lynx") {}
    bool IsActive() override;
};

class HalazziDeterminingDpsTargetTrigger : public Trigger
{
public:
    HalazziDeterminingDpsTargetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "halazzi determining dps target") {}
    bool IsActive() override;
};

class HexLordMalacrassPullingBossTrigger : public Trigger
{
public:
    HexLordMalacrassPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hex lord malacrass pulling boss") {}
    bool IsActive() override;
};

class HexLordMalacrassDeterminingKillOrderTrigger : public Trigger
{
public:
    HexLordMalacrassDeterminingKillOrderTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hex lord malacrass determining kill order") {}
    bool IsActive() override;
};

class HexLordMalacrassBossHasSiphonedBuffTrigger : public Trigger
{
public:
    HexLordMalacrassBossHasSiphonedBuffTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hex lord malacrass boss has siphoned buff") {}
    bool IsActive() override;
};

class HexLordMalacrassPartyMemberIsMindControlledTrigger : public Trigger
{
public:
    HexLordMalacrassPartyMemberIsMindControlledTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hex lord malacrass party member is mind controlled") {}
    bool IsActive() override;
};

class ZuljinMainTankNeedsAggroUponPullOrPhaseChangeTrigger : public Trigger
{
public:
    ZuljinMainTankNeedsAggroUponPullOrPhaseChangeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "zuljin main tank needs aggro upon pull or phase change") {}
    bool IsActive() override;
};

class ZuljinBossIsChannelingWhirlwindInTrollFormTrigger : public Trigger
{
public:
    ZuljinBossIsChannelingWhirlwindInTrollFormTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "zuljin boss is channeling whirlwind in troll form") {}
    bool IsActive() override;
};

class ZuljinBossCastsAoeAbilitiesTrigger : public Trigger
{
public:
    ZuljinBossCastsAoeAbilitiesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "zuljin boss casts aoe abilities") {}
    bool IsActive() override;
};

class ZuljinBossIsChargingPlayersInLynxFormTrigger : public Trigger
{
public:
    ZuljinBossIsChargingPlayersInLynxFormTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "zuljin boss is charging players in lynx form") {}
    bool IsActive() override;
};

#endif
