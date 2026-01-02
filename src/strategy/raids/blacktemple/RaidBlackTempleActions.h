#ifndef _PLAYERBOT_RAIDBLACKTEMPLEACTIONS_H
#define _PLAYERBOT_RAIDBLACKTEMPLEACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// High Warlord Naj'entus

class HighWarlordNajentusMisdirectBossToMainTankAction : public AttackAction
{
public:
    HighWarlordNajentusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "high warlord naj'entus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusMainTankPositionBossAction : public AttackAction
{
public:
    HighWarlordNajentusMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "high warlord naj'entus main tank position boss") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusDisperseRangedAction : public MovementAction
{
public:
    HighWarlordNajentusDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "naj'entus disperse ranged") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusRemoveImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusRemoveImpalingSpineAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "high warlord naj'entus remove impaling spine") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusThrowImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusThrowImpalingSpineAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "high warlord naj'entus throw impaling spine") {}
    bool Execute(Event event) override;
};

// Supremus

class SupremusMisdirectBossToMainTankAction : public AttackAction
{
public:
    SupremusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "supremus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class SupremusDisperseRangedAction : public MovementAction
{
public:
    SupremusDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus disperse ranged") {}
    bool Execute(Event event) override;
};

class SupremusKiteBossAction : public MovementAction
{
public:
    SupremusKiteBossAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus kite boss") {}
    bool Execute(Event event) override;
};

class SupremusMoveAwayFromVolcanosAction : public MovementAction
{
public:
    SupremusMoveAwayFromVolcanosAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus move away from volcanos") {}
    bool Execute(Event event) override;

private:
    Position FindSafestNearbyPosition(
        const std::vector<Unit*>& volcanos, float maxRadius, float hazardRadius);
    bool IsPathSafeFromVolcanos(const Position& start,
        const Position& end, const std::vector<Unit*>& volcanos, float hazardRadius);
    std::vector<Unit*> GetAllSupremusVolcanos(PlayerbotAI* botAI, Player* bot);
};

class SupremusManagePhaseTimerAction : public Action
{
public:
    SupremusManagePhaseTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "supremus manage phase timer") {}
    bool Execute(Event event) override;
};

// Shade of Akama
// N/A

// Teron Gorefiend
class TeronGorefiendMisdirectBossToMainTankAction : public AttackAction
{
public:
    TeronGorefiendMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "teron gorefiend misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class TeronGorefiendMainTankPositionBossAction : public AttackAction
{
public:
    TeronGorefiendMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "teron gorefiend main tank position boss") {}
    bool Execute(Event event) override;
};

class TeronGorefiendPositionRangedOnBalconyAction : public MovementAction
{
public:
    TeronGorefiendPositionRangedOnBalconyAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "teron gorefiend position ranged on balcony") {}
    bool Execute(Event event) override;
};

class TeronGorefiendAvoidShadowOfDeathAction : public Action
{
public:
    TeronGorefiendAvoidShadowOfDeathAction(
        PlayerbotAI* botAI) : Action(botAI, "teron gorefiend avoid shadow of death") {}
    bool Execute(Event event) override;
};

class TeronGorefiendMoveToCornerToDieAction : public MovementAction
{
public:
    TeronGorefiendMoveToCornerToDieAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "teron gorefiend move to corner to die") {}
    bool Execute(Event event) override;
};

class TeronGorefiendControlAndDestroyShadowyConstructsAction : public MovementAction
{
public:
    TeronGorefiendControlAndDestroyShadowyConstructsAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "teron gorefiend control and destroy shadowy constructs") {}
    bool Execute(Event event) override;
};

// Gurtogg Bloodboil

class GurtoggBloodboilMisdirectBossToMainTankAction : public AttackAction
{
public:
    GurtoggBloodboilMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "gurtogg bloodboil misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilTanksPositionBossAction : public AttackAction
{
public:
    GurtoggBloodboilTanksPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "gurtogg bloodboil tanks position boss") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilDisperseRangedAction : public MovementAction
{
public:
    GurtoggBloodboilDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "gurtogg bloodboil disperse ranged") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilRangedMoveToAbsorbBloodboilPositionAction : public MovementAction
{
public:
    GurtoggBloodboilRangedMoveToAbsorbBloodboilPositionAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "gurtogg bloodboil ranged move to absorb bloodboil position") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilFelRagedBotMoveToTankPositionAction : public MovementAction
{
public:
    GurtoggBloodboilFelRagedBotMoveToTankPositionAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "gurtogg bloodboil fel raged bot move to tank position") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilManagePhaseTimerAction : public Action
{
public:
    GurtoggBloodboilManagePhaseTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "gurtogg bloodboil manage phase timer") {}
    bool Execute(Event event) override;
};

// Reliquary of Souls

class ReliquaryOfSoulsMisdirectBossToMainTankAction : public AttackAction
{
public:
    ReliquaryOfSoulsMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "reliquary of souls misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsMeleeDpsStayAtMaxRangeFromEssenceOfSufferingAction : public MovementAction
{
public:
    ReliquaryOfSoulsMeleeDpsStayAtMaxRangeFromEssenceOfSufferingAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "reliquary of souls melee dps stay at max range from essence of suffering") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsHealersDpsEssenceOfSufferingAction : public Action
{
public:
    ReliquaryOfSoulsHealersDpsEssenceOfSufferingAction(
        PlayerbotAI* botAI) : Action(botAI, "reliquary of souls healers dps essence of suffering") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsSpellstealRuneShieldAction : public Action
{
public:
    ReliquaryOfSoulsSpellstealRuneShieldAction(
        PlayerbotAI* botAI) : Action(botAI, "reliquary of souls spellsteal rune shield") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsSpellReflectDeadenAction : public Action
{
public:
    ReliquaryOfSoulsSpellReflectDeadenAction(
        PlayerbotAI* botAI) : Action(botAI, "reliquary of souls spell reflect deaden") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsManageDpsTimerAction : public Action
{
public:
    ReliquaryOfSoulsManageDpsTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "reliquary of souls manage dps timer") {}
    bool Execute(Event event) override;
};

// Mother Shahraz

class MotherShahrazMisdirectBossToMainTankAction : public AttackAction
{
public:
    MotherShahrazMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "mother shahraz misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class MotherShahrazTanksPositionBossAction : public AttackAction
{
public:
    MotherShahrazTanksPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "mother shahraz tanks position boss") {}
    bool Execute(Event event) override;
};

class MotherShahrazPositionRangedUnderStatueAction : public MovementAction
{
public:
    MotherShahrazPositionRangedUnderStatueAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "mother shahraz position ranged under statue") {}
    bool Execute(Event event) override;
};

class MotherShahrazRunAwayToBreakFatalAttractionAction : public MovementAction
{
public:
    MotherShahrazRunAwayToBreakFatalAttractionAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "mother shahraz run away to break fatal attraction") {}
    bool Execute(Event event) override;
};

// Illidari Council

class IllidariCouncilMisdirectBossesToTanksAction : public AttackAction
{
public:
    IllidariCouncilMisdirectBossesToTanksAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidari council misdirect bosses to tanks") {}
    bool Execute(Event event) override;
};

// Illidan Stormrage <The Betrayer>

#endif
