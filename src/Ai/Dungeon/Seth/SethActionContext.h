/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SETHACTIONCONTEXT_H
#define PLAYERBOTS_SETHACTIONCONTEXT_H

#include "SethActions.h"
#include "NamedObjectContext.h"

class TbcDungeonSethekkHallsActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonSethekkHallsActionContext()
    {
        creators["time-lost controller mark charming totem with skull"] =
            &TbcDungeonSethekkHallsActionContext::time_lost_controller_mark_charming_totem_with_skull;

        creators["sethekk prophet drop tremor totem"] =
            &TbcDungeonSethekkHallsActionContext::sethekk_prophet_drop_tremor_totem;

        creators["darkweaver syth mark elementals with skull"] =
            &TbcDungeonSethekkHallsActionContext::darkweaver_syth_mark_elementals_with_skull;

        creators["anzu alternate marks on boss"] =
            &TbcDungeonSethekkHallsActionContext::anzu_alternate_marks_on_boss;

        creators["anzu cast heal over time spell on bird spirit"] =
            &TbcDungeonSethekkHallsActionContext::anzu_cast_heal_over_time_spell_on_bird_spirit;

        creators["talon king ikiss move to pillar position"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_move_to_pillar_position;

        creators["talon king ikiss los arcane explosion"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_los_arcane_explosion;
    }
private:
    static Action* time_lost_controller_mark_charming_totem_with_skull(PlayerbotAI* botAI) {
        return new TimeLostControllerMarkCharmingTotemWithSkullAction(botAI);
    }
    static Action* sethekk_prophet_drop_tremor_totem(PlayerbotAI* botAI) {
        return new SethekkProphetDropTremorTotemAction(botAI);
    }
    static Action* darkweaver_syth_mark_elementals_with_skull(PlayerbotAI* botAI) {
        return new DarkweaverSythMarkElementalsWithSkullAction(botAI);
    }
    static Action* anzu_alternate_marks_on_boss(PlayerbotAI* botAI) {
        return new AnzuAlternateMarksOnBossAction(botAI);
    }
    static Action* anzu_cast_heal_over_time_spell_on_bird_spirit(PlayerbotAI* botAI) {
        return new AnzuCastHealOverTimeSpellOnBirdSpiritAction(botAI);
    }
    static Action* talon_king_ikiss_move_to_pillar_position(PlayerbotAI* botAI) {
        return new TalonKingIkissMoveToPillarPositionAction(botAI);
    }
    static Action* talon_king_ikiss_los_arcane_explosion(PlayerbotAI* botAI) {
        return new TalonKingIkissLosArcaneExplosionAction(botAI);
    }
};

#endif
