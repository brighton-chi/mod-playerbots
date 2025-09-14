#ifndef _PLAYERBOT_RAIDGRUULSLAIRACTIONCONTEXT_H
#define _PLAYERBOT_RAIDGRUULSLAIRACTIONCONTEXT_H

#include "RaidGruulsLairActions.h"
#include "NamedObjectContext.h"

class RaidGruulsLairActionContext : public NamedObjectContext<Action>
{
public:
    RaidGruulsLairActionContext()
    {
        creators["high king maulgar dps priority"] = &RaidGruulsLairActionContext::high_king_maulgar_dps_priority;
        creators["high king maulgar main tank"] = &RaidGruulsLairActionContext::high_king_maulgar_main_tank;
        creators["high king maulgar first off tank"] = &RaidGruulsLairActionContext::high_king_maulgar_first_off_tank;
        creators["high king maulgar blindeye tank"] = &RaidGruulsLairActionContext::high_king_maulgar_blindeye_tank;
        creators["high king maulgar mage tank"] = &RaidGruulsLairActionContext::high_king_maulgar_mage_tank;
        creators["high king maulgar avoid blast wave"] = &RaidGruulsLairActionContext::high_king_maulgar_avoid_blast_wave;
        creators["high king maulgar control felstalker"] = &RaidGruulsLairActionContext::high_king_maulgar_control_felstalker;
        creators["high king maulgar avoid whirlwind"] = &RaidGruulsLairActionContext::high_king_maulgar_avoid_whirlwind;
        creators["high king maulgar hunter misdirection"] = &RaidGruulsLairActionContext::high_king_maulgar_hunter_misdirection;
    }

private:
    static Action* high_king_maulgar_dps_priority(PlayerbotAI* botAI) { return new HighKingMaulgarDPSPriorityAction(botAI); }
    static Action* high_king_maulgar_main_tank(PlayerbotAI* botAI) { return new HighKingMaulgarMainTankAction(botAI); }
    static Action* high_king_maulgar_first_off_tank(PlayerbotAI* botAI) { return new HighKingMaulgarFirstOffTankAction(botAI); }
    static Action* high_king_maulgar_blindeye_tank(PlayerbotAI* botAI) { return new HighKingMaulgarBlindeyeTankAction(botAI); }
    static Action* high_king_maulgar_mage_tank(PlayerbotAI* botAI) { return new HighKingMaulgarMageTankAction(botAI); }
    static Action* high_king_maulgar_avoid_blast_wave(PlayerbotAI* botAI) { return new HighKingMaulgarAvoidBlastWaveAction(botAI); }
    static Action* high_king_maulgar_control_felstalker(PlayerbotAI* botAI) { return new HighKingMaulgarControlFelstalkerAction(botAI); }
    static Action* high_king_maulgar_avoid_whirlwind(PlayerbotAI* botAI) { return new HighKingMaulgarAvoidWhirlwindAction(botAI); }
    static Action* high_king_maulgar_hunter_misdirection(PlayerbotAI* botAI) { return new HighKingMaulgarHunterMisdirectionAction(botAI); }
};

#endif
