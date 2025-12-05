#include "RaidTempestKeepStrategy.h"
#include "RaidTempestKeepMultipliers.h"

void RaidTempestKeepStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash
    triggers.push_back(new TriggerNode("crimson hand centurion casts arcane volley",
        NextAction::array(0, new NextAction("crimson hand centurion cast polymorph", ACTION_RAID + 1), nullptr)
    ));

    // Al'ar
    triggers.push_back(new TriggerNode("alar pulling boss",
        NextAction::array(0, new NextAction("alar misdirect boss to main tank", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar engaged by tanks in phase 1",
        NextAction::array(0, new NextAction("alar boss tanks move between platforms", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar boss engaged by melee dps",
        NextAction::array(0, new NextAction("alar melee dps prioritize boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar boss engaged by ranged dps",
        NextAction::array(0, new NextAction("alar ranged dps prioritize adds", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar boss engaged by healer",
        NextAction::array(0, new NextAction("alar position healer", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar embers of alar spawned",
        NextAction::array(0, new NextAction("alar add tank pick up embers", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar incoming flame quills",
        NextAction::array(0, new NextAction("alar jump from platform", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar rising from the ashes",
        NextAction::array(0, new NextAction("alar move away from rebirth", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar boss tank armor was melted",
        NextAction::array(0, new NextAction("alar swap tanks on boss", ACTION_EMERGENCY + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar boss is performing dive bomb sequence",
        NextAction::array(0, new NextAction("alar dive bomb spread and stay back", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar phase 2 encounter is at room center",
        NextAction::array(0, new NextAction("alar return to room center", ACTION_RAID + 1), nullptr)
    ));

    // Void Reaver
    triggers.push_back(new TriggerNode("void reaver boss engaged by tank",
        NextAction::array(0, new NextAction("void reaver position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("void reaver boss launches arcane orbs",
        NextAction::array(0, new NextAction("void reaver spread ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("void reaver tanks lost aggro",
        NextAction::array(0, new NextAction("void reaver use aggro dump ability", ACTION_EMERGENCY + 6), nullptr)
    ));

    // High Astromancer Solarian
    triggers.push_back(new TriggerNode("high astromancer solarian phase 1 and 2 movement",
        NextAction::array(0, new NextAction("high astromancer solarian stack bots", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian bot has wrath of the astromancer",
        NextAction::array(0, new NextAction("high astromancer solarian move away from group", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian solarium priests spawned",
        NextAction::array(0, new NextAction("high astromancer solarian target solarium priests", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian transformed into voidwalker",
        NextAction::array(0, new NextAction("high astromancer solarian tank voidwalker", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian boss casts psychic scream",
        NextAction::array(0, new NextAction("high astromancer solarian cast fear ward on main tank", ACTION_RAID + 2), nullptr)
    ));

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    triggers.push_back(new TriggerNode("kaelthas sunstrider thaladred is fixated on bot",
        NextAction::array(0, new NextAction("kaelthas sunstrider kite thaladred", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider sanguinar engaged by main tank",
        NextAction::array(0, new NextAction("kaelthas sunstrider main tank position sanguinar", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider sanguinar casts bellowing roar",
        NextAction::array(0, new NextAction("kaelthas sunstrider cast fear ward on sanguinar tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider capernian requires a warlock tank",
        NextAction::array(0, new NextAction("kaelthas sunstrider manage warlock tank strategy", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider capernian engaged by warlock tank",
        NextAction::array(0, new NextAction("kaelthas sunstrider warlock tank position capernian", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider capernian casts arcane burst",
        NextAction::array(0, new NextAction("kaelthas sunstrider move away from capernian", ACTION_RAID + 3), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider telonicus engaged by first assist tank",
        NextAction::array(0, new NextAction("kaelthas sunstrider first assist tank position telonicus", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider pulling tankable advisors",
        NextAction::array(0, new NextAction("kaelthas sunstrider misdirect advisors to tanks", ACTION_EMERGENCY + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider waiting for tanks to get aggro on advisors",
        NextAction::array(0, new NextAction("kaelthas sunstrider manage advisor dps timer", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider legendary weapons are alive",
        NextAction::array(0, new NextAction("kaelthas sunstrider group up legendary weapons", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider devastation channels whirlwind",
        NextAction::array(0, new NextAction("kaelthas sunstrider main tank move devastation away", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider netherstrand longbow fires multishot",
        NextAction::array(0, new NextAction("kaelthas sunstrider hunter turn away netherstrand longbow", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider legendary weapons are dead and lootable",
        NextAction::array(0, new NextAction("kaelthas sunstrider loot legendary weapons", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider legendary weapons are equipped",
        NextAction::array(0, new NextAction("kaelthas sunstrider use legendary weapons", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider legendary weapons were lost",
        NextAction::array(0, new NextAction("kaelthas sunstrider reequip gear", ACTION_EMERGENCY + 11), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider determining advisor kill order",
        NextAction::array(0, new NextAction("kaelthas sunstrider assign advisor dps priority", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider boss engaged by main tank",
        NextAction::array(0, new NextAction("kaelthas sunstrider main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider flame strike appeared under bot",
        NextAction::array(0, new NextAction("kaelthas sunstrider avoid flame strike", ACTION_EMERGENCY + 8), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider phoenixes and eggs are spawning",
        NextAction::array(0, new NextAction("kaelthas sunstrider round up phoenixes and focus down eggs", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider raid member is mind controlled",
        NextAction::array(0, new NextAction("kaelthas sunstrider break mind control with infinity blade", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider boss is casting pyroblast",
        NextAction::array(0, new NextAction("kaelthas sunstrider break through shock barrier", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaelthas sunstrider boss is manipulating gravity",
        NextAction::array(0, new NextAction("kaelthas sunstrider spread out in midair", ACTION_EMERGENCY + 1), nullptr)
    ));
}

void RaidTempestKeepStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AlarMoveBetweenPlatformsMultiplier(botAI));
    multipliers.push_back(new AlarDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new AlarStayAwayFromRebirthMultiplier(botAI));
    multipliers.push_back(new AlarPhase2NoTankingIfArmorMeltedMultiplier(botAI));
    multipliers.push_back(new VoidReaverMaintainPositionsMultiplier(botAI));
    multipliers.push_back(new HighAstromancerSolarianStayStackedMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderWaitForDpsMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderKiteThaladredMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderTryNonfatalBreakingOfMindControlMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderAllDpsOnBossDuringPyroblastMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderMeleeDpsDoNotAttackDuringGravityLapseMultiplier(botAI));
}
