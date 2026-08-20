/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULTRIGGERCONTEXT_H
#define PLAYERBOTS_GRUULTRIGGERCONTEXT_H

#include "GruulTriggers.h"
#include "NamedObjectContext.h"

class RaidGruulsLairTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidGruulsLairTriggerContext() : NamedObjectContext<Trigger>()
    {
        // General
        creators["gruul's lair bot is not in combat"] =
            &RaidGruulsLairTriggerContext::gruuls_lair_bot_is_not_in_combat;

        // High King Maulgar
        creators["high king maulgar bosses engaged by melee tanks"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_bosses_engaged_by_melee_tanks;

        creators["high king maulgar krosh engaged by mage tank"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_krosh_engaged_by_mage_tank;

        creators["high king maulgar kiggler engaged by moonkin tank"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_kiggler_engaged_by_moonkin_tank;

        creators["high king maulgar determining kill order"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_determining_kill_order;

        creators["high king maulgar boss channeling whirlwind"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_boss_channeling_whirlwind;

        creators["high king maulgar krosh casts blast wave"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_krosh_casts_blast_wave;

        creators["high king maulgar wild fel stalker spawned"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_wild_fel_stalker_spawned;

        creators["high king maulgar pulling ogre council"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_pulling_ogre_council;

        // Gruul the Dragonkiller
        creators["gruul the dragonkiller boss engaged by tanks"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_boss_engaged_by_tanks;

        creators["gruul the dragonkiller ranged should spread"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_ranged_should_spread;

        creators["gruul the dragonkiller incoming shatter"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_incoming_shatter;
    }

private:
    // General
    static Trigger* gruuls_lair_bot_is_not_in_combat(PlayerbotAI* botAI) {
        return new GruulsLairBotIsNotInCombatTrigger(botAI);
    }

    // High King Maulgar
    static Trigger* high_king_maulgar_bosses_engaged_by_melee_tanks(PlayerbotAI* botAI) {
        return new HighKingMaulgarBossesEngagedByMeleeTanksTrigger(botAI);
    }
    static Trigger* high_king_maulgar_krosh_engaged_by_mage_tank(PlayerbotAI* botAI) {
        return new HighKingMaulgarKroshEngagedByMageTankTrigger(botAI);
    }
    static Trigger* high_king_maulgar_kiggler_engaged_by_moonkin_tank(PlayerbotAI* botAI) {
        return new HighKingMaulgarKigglerEngagedByMoonkinTankTrigger(botAI);
    }
    static Trigger* high_king_maulgar_determining_kill_order(PlayerbotAI* botAI) {
        return new HighKingMaulgarDeterminingKillOrderTrigger(botAI);
    }
    static Trigger* high_king_maulgar_boss_channeling_whirlwind(PlayerbotAI* botAI) {
        return new HighKingMaulgarBossChannelingWhirlwindTrigger(botAI);
    }
    static Trigger* high_king_maulgar_krosh_casts_blast_wave(PlayerbotAI* botAI) {
        return new HighKingMaulgarKroshCastsBlastWaveTrigger(botAI);
    }
    static Trigger* high_king_maulgar_wild_fel_stalker_spawned(PlayerbotAI* botAI) {
        return new HighKingMaulgarWildFelStalkerSpawnedTrigger(botAI);
    }
    static Trigger* high_king_maulgar_pulling_ogre_council(PlayerbotAI* botAI) {
        return new HighKingMaulgarPullingOgreCouncilTrigger(botAI);
    }

    // Gruul the Dragonkiller
    static Trigger* gruul_the_dragonkiller_boss_engaged_by_tanks(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* gruul_the_dragonkiller_ranged_should_spread(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* gruul_the_dragonkiller_incoming_shatter(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerIncomingShatterTrigger(botAI);
    }
};

#endif
