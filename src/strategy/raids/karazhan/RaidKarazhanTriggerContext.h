#ifndef _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "RaidKarazhanTriggers.h"

class RaidKarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidKarazhanTriggerContext()
    {
        creators["karazhan moroes"] = &RaidKarazhanTriggerContext::karazhan_moroes;
        creators["karazhan maiden of virtue"] = &RaidKarazhanTriggerContext::karazhan_maiden_of_virtue;
        creators["karazhan big bad wolf"] = &RaidKarazhanTriggerContext::Karazhan_big_bad_wolf;
        creators["karazhan romulo julianne"] = &RaidKarazhanTriggerContext::Karazhan_romulo_julianne;
        creators["karazhan wizard of oz"] = &RaidKarazhanTriggerContext::Karazhan_wizard_of_oz;
        creators["karazhan the curator"] = &RaidKarazhanTriggerContext::karazhan_the_curator;
        creators["karazhan terestian illhoof"] = &RaidKarazhanTriggerContext::karazhan_terestian_illhoof;
        creators["karazhan shade of aran"] = &RaidKarazhanTriggerContext::karazhan_shade_of_aran;
        //creators["karazhan netherspite"] = &RaidKarazhanTriggerContext::karazhan_netherspite;
    }

private:
    static Trigger* karazhan_moroes(PlayerbotAI* ai) { return new KarazhanMoroesTrigger(ai); }
    static Trigger* karazhan_maiden_of_virtue(PlayerbotAI* ai) { return new KarazhanMaidenOfVirtueTrigger(ai); }
    static Trigger* Karazhan_big_bad_wolf(PlayerbotAI* ai) { return new KarazhanBigBadWolfTrigger(ai); }
    static Trigger* Karazhan_romulo_julianne(PlayerbotAI* ai) { return new KarazhanRomuloJulianneTrigger(ai); }
    static Trigger* Karazhan_wizard_of_oz(PlayerbotAI* ai) { return new KarazhanWizardOfOzTrigger(ai); }
    static Trigger* karazhan_the_curator(PlayerbotAI* ai) { return new KarazhanTheCuratorTrigger(ai); }
    static Trigger* karazhan_terestian_illhoof(PlayerbotAI* ai) { return new KarazhanTerestianIllhoofTrigger(ai); }
    static Trigger* karazhan_shade_of_aran(PlayerbotAI* ai) { return new KarazhanShadeOfAranTrigger(ai); }
    //static Trigger* karazhan_netherspite(PlayerbotAI* ai) { return new KarazhanNetherspiteTrigger(ai); }
};

#endif
