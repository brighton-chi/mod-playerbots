#ifndef _PLAYERBOT_RAIDGRUULSLAIRACTIONCONTEXT_H
#define _PLAYERBOT_RAIDGRUULSLAIRACTIONCONTEXT_H

#include "RaidGruulsLairActions.h"
#include "NamedObjectContext.h"

class RaidGruulsLairActionContext : public NamedObjectContext<Action>
{
public:
    RaidGruulsLairActionContext()
    {
        creators["high king maulgar maulgar tank"] = &RaidGruulsLairActionContext::high_king_maulgar_maulgar_tank;
        creators["high king maulgar olm tank"] = &RaidGruulsLairActionContext::high_king_maulgar_olm_tank;
        creators["high king maulgar blindeye tank"] = &RaidGruulsLairActionContext::high_king_maulgar_blindeye_tank;
        creators["high king maulgar mage tank"] = &RaidGruulsLairActionContext::high_king_maulgar_mage_tank;
        creators["high king maulgar boomkin tank"] = &RaidGruulsLairActionContext::high_king_maulgar_boomkin_tank;
        creators["high king maulgar melee dps"] = &RaidGruulsLairActionContext::high_king_maulgar_melee_dps;
        creators["high king maulgar ranged dps"] = &RaidGruulsLairActionContext::high_king_maulgar_ranged_dps;
        //creators["high king maulgar avoid blast wave"] = &RaidGruulsLairActionContext::high_king_maulgar_avoid_blast_wave;
        //creators["high king maulgar control felstalker"] = &RaidGruulsLairActionContext::high_king_maulgar_control_felstalker;
        //creators["high king maulgar avoid whirlwind"] = &RaidGruulsLairActionContext::high_king_maulgar_avoid_whirlwind;
        creators["high king maulgar hunter misdirection"] = &RaidGruulsLairActionContext::high_king_maulgar_hunter_misdirection;
    }

private:
    static Action* high_king_maulgar_maulgar_tank(PlayerbotAI* botAI) { return new HighKingMaulgarMaulgarTankAction(botAI); }
    static Action* high_king_maulgar_olm_tank(PlayerbotAI* botAI) { return new HighKingMaulgarOlmTankAction(botAI); }
    static Action* high_king_maulgar_blindeye_tank(PlayerbotAI* botAI) { return new HighKingMaulgarBlindeyeTankAction(botAI); }
    static Action* high_king_maulgar_mage_tank(PlayerbotAI* botAI) { return new HighKingMaulgarMageTankAction(botAI); }
    static Action* high_king_maulgar_boomkin_tank(PlayerbotAI* botAI) { return new HighKingMaulgarBoomkinTankAction(botAI); }
    static Action* high_king_maulgar_melee_dps(PlayerbotAI* botAI) { return new HighKingMaulgarMeleeDPSAction(botAI); }
    static Action* high_king_maulgar_ranged_dps(PlayerbotAI* botAI) { return new HighKingMaulgarRangedDPSAction(botAI); }
    //static Action* high_king_maulgar_avoid_blast_wave(PlayerbotAI* botAI) { return new HighKingMaulgarAvoidBlastWaveAction(botAI); }
    //static Action* high_king_maulgar_control_felstalker(PlayerbotAI* botAI) { return new HighKingMaulgarControlFelstalkerAction(botAI); }
    //static Action* high_king_maulgar_avoid_whirlwind(PlayerbotAI* botAI) { return new HighKingMaulgarAvoidWhirlwindAction(botAI); }
    static Action* high_king_maulgar_hunter_misdirection(PlayerbotAI* botAI) { return new HighKingMaulgarHunterMisdirectionAction(botAI); }
};

#endif
