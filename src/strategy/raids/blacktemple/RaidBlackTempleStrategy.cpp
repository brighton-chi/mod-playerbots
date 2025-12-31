#include "RaidBlackTempleStrategy.h"
#include "RaidBlackTempleMultipliers.h"

void RaidBlackTempleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // High Warlord Naj'entus
    triggers.push_back(new TriggerNode("high warlord naj'entus pulling boss",
        NextAction::array(0, new NextAction("high warlord naj'entus misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("high warlord naj'entus boss engaged by main tank",
        NextAction::array(0, new NextAction("high warlord naj'entus main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high warlord naj'entus casts needle spines",
        NextAction::array(0, new NextAction("high warlord naj'entus disperse ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high warlord naj'entus player is impaled",
        NextAction::array(0, new NextAction("high warlord naj'entus remove impaling spine", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high warlord naj'entus boss has tidal shield",
        NextAction::array(0, new NextAction("high warlord naj'entus throw impaling spine", ACTION_RAID + 2), nullptr)
    ));

    // Supremus
    triggers.push_back(new TriggerNode("supremus pulling boss or changing phase",
        NextAction::array(0, new NextAction("supremus misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("supremus boss engaged by ranged",
        NextAction::array(0, new NextAction("supremus disperse ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("supremus boss is fixated on bot",
        NextAction::array(0, new NextAction("supremus kite boss", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("supremus volcano is nearby",
        NextAction::array(0, new NextAction("supremus move away from volcanos", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("supremus need to manage phase timer",
        NextAction::array(0, new NextAction("supremus manage phase timer", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Shade of Akama
    // N/A

    // Teron Gorefiend
    triggers.push_back(new TriggerNode("teron gorefiend pulling boss",
        NextAction::array(0, new NextAction("teron gorefiend misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("teron gorefiend boss engaged",
        NextAction::array(0,
            new NextAction("teron gorefiend main tank position boss", ACTION_RAID + 1),
            new NextAction("teron gorefiend position ranged on balcony", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("teron gorefiend boss is casting shadow of death",
        NextAction::array(0, new NextAction("teron gorefiend avoid shadow of death", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode("teron gorefiend bot has shadow of death",
        NextAction::array(0, new NextAction("teron gorefiend move to corner to die", ACTION_EMERGENCY + 10), nullptr)
    ));
    /* triggers.push_back(new TriggerNode("teron gorefiend bot transformed into vengeful spirit",
        NextAction::array(0, new NextAction("teron gorefiend control and destroy shadowy constructs", ACTION_EMERGENCY + 10), nullptr)
    )); */

    // Gurtogg Bloodboil
    triggers.push_back(new TriggerNode("gurtogg bloodboil pulling boss",
        NextAction::array(0, new NextAction("gurtogg bloodboil misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("gurtogg bloodboil boss engaged by tanks",
        NextAction::array(0, new NextAction("gurtogg bloodboil tanks position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("gurtogg bloodboil boss casts aoe spells",
        NextAction::array(0, new NextAction("gurtogg bloodboil disperse ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("gurtogg bloodboil boss casts bloodboil on five farthest players",
        NextAction::array(0, new NextAction("gurtogg bloodboil ranged move to absorb bloodboil position", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("gurtogg bloodboil bot has fel rage",
        NextAction::array(0, new NextAction("gurtogg bloodboil fel raged bot move to tank position", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("gurtogg bloodboil need to manage phase timer",
        NextAction::array(0, new NextAction("gurtogg bloodboil manage phase timer", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Reliquary of Souls // NOTE: UNTESTED
    triggers.push_back(new TriggerNode("reliquary of souls aggro resets upon phase change",
        NextAction::array(0, new NextAction("reliquary of souls misdirect boss to main tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("reliquary of souls essence of suffering fixates on closest target",
        NextAction::array(0, new NextAction("reliquary of souls melee dps stay at max range from essence of suffering", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("reliquary of souls essence of suffering disables healing",
        NextAction::array(0, new NextAction("reliquary of souls healers dps essence of suffering", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("reliquary of souls essence of desire has rune shield",
        NextAction::array(0, new NextAction("reliquary of souls spellsteal rune shield", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("reliquary of souls essence of desire casting deaden",
        NextAction::array(0, new NextAction("reliquary of souls spell reflect deaden", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode("reliquary of souls need to manage dps timer",
        NextAction::array(0, new NextAction("reliquary of souls manage dps timer", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Mother Shahraz
    triggers.push_back(new TriggerNode("mother shahraz pulling boss",
        NextAction::array(0, new NextAction("mother shahraz misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("mother shahraz boss engaged by tanks",
        NextAction::array(0, new NextAction("mother shahraz tanks position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("mother shahraz sinister beam knocks back players",
        NextAction::array(0, new NextAction("mother shahraz position ranged under statue", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("mother shahraz bots are linked by fatal attraction",
        NextAction::array(0, new NextAction("mother shahraz run away to break fatal attraction", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
}

void RaidBlackTempleStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // High Warlord Naj'entus
    // N/A

    // Supremus
    multipliers.push_back(new SupremusFocusOnAvoidanceInPhase2Multiplier(botAI));

    // Shade of Akama
    // N/A

    // Teron Gorefiend
    multipliers.push_back(new TeronGorefiendDisableMovementMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendMarkedBotOnlyMoveToDieMultiplier(botAI));
    // multipliers.push_back(new TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendDisableAttackingConstructsMultiplier(botAI));

    // Gurtogg Bloodboil
    multipliers.push_back(new GurtoggBloodboilDisableMovementMultiplier(botAI));

    // Reliquary of Souls
    multipliers.push_back(new ReliquaryOfSoulsWaitForDpsMultiplier(botAI));
    multipliers.push_back(new ReliquaryOfSoulsDontWasteHealingMultiplier(botAI));
    multipliers.push_back(new ReliquaryOfSoulsDontInterruptDeadenIfReflectableMultiplier(botAI));

    // Mother Shahraz
    multipliers.push_back(new MotherShahrazDisableMovementMultiplier(botAI));
    multipliers.push_back(new MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier(botAI));

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
}
