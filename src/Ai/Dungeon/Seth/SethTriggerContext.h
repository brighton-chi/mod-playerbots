/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SETHTRIGGERCONTEXT_H
#define PLAYERBOTS_SETHTRIGGERCONTEXT_H

#include "SethTriggers.h"
#include "NamedObjectContext.h"

class TbcDungeonSethekkHallsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TbcDungeonSethekkHallsTriggerContext()
    {
        creators["time-lost controller drops charming totem"] =
            &TbcDungeonSethekkHallsTriggerContext::time_lost_controller_drops_charming_totem;

        creators["sethekk prophet casts fear"] =
            &TbcDungeonSethekkHallsTriggerContext::sethekk_prophet_casts_fear;

        creators["darkweaver syth boss summons elementals"] =
            &TbcDungeonSethekkHallsTriggerContext::darkweaver_syth_boss_summons_elementals;

        creators["anzu encounter has two phases"] =
            &TbcDungeonSethekkHallsTriggerContext::anzu_encounter_has_two_phases;

        creators["anzu bird spirits provide buffs"] =
            &TbcDungeonSethekkHallsTriggerContext::anzu_bird_spirits_provide_buffs;

        creators["talon king ikiss ranged prepare for arcane explosion"] =
            &TbcDungeonSethekkHallsTriggerContext::talon_king_ikiss_ranged_prepare_for_arcane_explosion;

        creators["talon king ikiss boss engaged by tank"] =
            &TbcDungeonSethekkHallsTriggerContext::talon_king_ikiss_boss_engaged_by_tank;

        creators["talon king ikiss boss casting arcane explosion"] =
            &TbcDungeonSethekkHallsTriggerContext::talon_king_ikiss_boss_casting_arcane_explosion;
    }
private:
    static Trigger* time_lost_controller_drops_charming_totem(PlayerbotAI* botAI) {
        return new TimeLostControllerDropsCharmingTotemTrigger(botAI);
    }
    static Trigger* sethekk_prophet_casts_fear(PlayerbotAI* botAI) {
        return new SethekkProphetCastsFearTrigger(botAI);
    }
    static Trigger* darkweaver_syth_boss_summons_elementals(PlayerbotAI* botAI) {
        return new DarkweaverSythBossSummonsElementalsTrigger(botAI);
    }
    static Trigger* anzu_encounter_has_two_phases(PlayerbotAI* botAI) {
        return new AnzuEncounterHasTwoPhasesTrigger(botAI);
    }
    static Trigger* anzu_bird_spirits_provide_buffs(PlayerbotAI* botAI) {
        return new AnzuBirdSpiritsProvideBuffsTrigger(botAI);
    }
    static Trigger* talon_king_ikiss_ranged_prepare_for_arcane_explosion(PlayerbotAI* botAI) {
        return new TalonKingIkissRangedPrepareForArcaneExplosionTrigger(botAI);
    }
    static Trigger* talon_king_ikiss_boss_engaged_by_tank(PlayerbotAI* botAI) {
        return new TalonKingIkissBossEngagedByTankTrigger(botAI);
    }
    static Trigger* talon_king_ikiss_boss_casting_arcane_explosion(PlayerbotAI* botAI) {
        return new TalonKingIkissBossCastingArcaneExplosionTrigger(botAI);
    }
};

#endif
