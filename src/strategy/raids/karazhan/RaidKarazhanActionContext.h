#ifndef _PLAYERBOT_RAIDKARAZHANACTIONS_CONTEXT_H
#define _PLAYERBOT_RAIDKARAZHANACTIONS_CONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidKarazhanActions.h"

class RaidKarazhanActionContext : public NamedObjectContext<Action>
{
public:
    RaidKarazhanActionContext()
    {
        creators["karazhan moroes mark target"] = &RaidKarazhanActionContext::karazhan_moroes_mark_target;

        creators["karazhan maiden of virtue position boss"] = &RaidKarazhanActionContext::karazhan_maiden_of_virtue_position_boss;
        creators["karazhan maiden of virtue position ranged"] = &RaidKarazhanActionContext::karazhan_maiden_of_virtue_position_ranged;

        creators["karazhan big bad wolf run away"] = &RaidKarazhanActionContext::karazhan_big_bad_wolf_run_away;
        creators["karazhan romulo julianne mark target"] = &RaidKarazhanActionContext::karazhan_romulo_julianne_mark_target;
        creators["karazhan wizard of oz mark target"] = &RaidKarazhanActionContext::karazhan_wizard_of_oz_mark_target;

        creators["karazhan the curator mark target"] = &RaidKarazhanActionContext::karazhan_the_curator_mark_target;
        creators["karazhan the curator position boss"] = &RaidKarazhanActionContext::karazhan_the_curator_position_boss;
        creators["karazhan the curator spread ranged"] = &RaidKarazhanActionContext::karazhan_the_curator_spread_ranged;

        creators["karazhan terestian illhoof mark target"] = &RaidKarazhanActionContext::karazhan_terestian_illhoof_mark_target;

        creators["karazhan shade of aran arcane explosion"] = &RaidKarazhanActionContext::karazhan_shade_of_aran_arcane_explosion;
        creators["karazhan shade of aran flame wreath"] = &RaidKarazhanActionContext::karazhan_shade_of_aran_flame_wreath;
        creators["karazhan shade of aran mark target"] = &RaidKarazhanActionContext::karazhan_shade_of_aran_mark_target;
        creators["karazhan shade of aran spread ranged"] = &RaidKarazhanActionContext::karazhan_shade_of_aran_spread_ranged;

        /*creators["karazhan netherspite soak beams"] = &RaidKarazhanActionContext::karazhan_netherspite_soak_beams;
        creators["karazhan netherspite avoid void zone"] = &RaidKarazhanActionContext::karazhan_netherspite_avoid_void_zone;
        creators["karazhan netherspite banished"] = &RaidKarazhanActionContext::karazhan_netherspite_banished;*/
    }

private:
    static Action* karazhan_moroes_mark_target(PlayerbotAI* ai) { return new KarazhanMoroesMarkTargetAction(ai); }

    static Action* karazhan_maiden_of_virtue_position_boss(PlayerbotAI* ai) { return new KarazhanMaidenOfVirtuePositionBossAction(ai); }
    static Action* karazhan_maiden_of_virtue_position_ranged(PlayerbotAI* ai) { return new KarazhanMaidenOfVirtuePositionRangedAction(ai); }

    static Action* karazhan_big_bad_wolf_run_away(PlayerbotAI* ai) { return new KarazhanBigBadWolfRunAwayAction(ai); }
    static Action* karazhan_romulo_julianne_mark_target(PlayerbotAI* ai) { return new KarazhanRomuloJulianneMarkTargetAction(ai); }
    static Action* karazhan_wizard_of_oz_mark_target(PlayerbotAI* ai) { return new KarazhanWizardOfOzMarkTargetAction(ai); }

    static Action* karazhan_the_curator_mark_target(PlayerbotAI* ai) { return new KarazhanTheCuratorMarkTargetAction(ai); }
    static Action* karazhan_the_curator_position_boss(PlayerbotAI* ai) { return new KarazhanTheCuratorPositionBossAction(ai); }
    static Action* karazhan_the_curator_spread_ranged(PlayerbotAI* ai) { return new KarazhanTheCuratorSpreadRangedAction(ai); }

    static Action* karazhan_terestian_illhoof_mark_target(PlayerbotAI* ai) { return new KarazhanTerestianIllhoofMarkTargetAction(ai); }

    static Action* karazhan_shade_of_aran_arcane_explosion(PlayerbotAI* ai) { return new KarazhanShadeOfAranArcaneExplosionAction(ai); }
    static Action* karazhan_shade_of_aran_flame_wreath(PlayerbotAI* ai) { return new KarazhanShadeOfAranFlameWreathAction(ai); }
    static Action* karazhan_shade_of_aran_mark_target(PlayerbotAI* ai) { return new KarazhanShadeOfAranMarkTargetAction(ai); }
    static Action* karazhan_shade_of_aran_spread_ranged(PlayerbotAI* ai) { return new KarazhanShadeOfAranSpreadRangedAction(ai); }

    /*static Action* karazhan_netherspite_soak_beams(PlayerbotAI* ai) { return new KarazhanNetherspiteSoakBeamsAction(ai); }
    static Action* karazhan_netherspite_avoid_void_zone(PlayerbotAI* ai) { return new KarazhanNetherspiteAvoidVoidZoneAction(ai); }
    static Action* karazhan_netherspite_banished(PlayerbotAI* ai) { return new KarazhanNetherspiteBanishedAction(ai); }*/
};

#endif
