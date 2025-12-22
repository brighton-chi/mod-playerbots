#include "RaidTempestKeepStrategy.h"
#include "RaidTempestKeepMultipliers.h"

void RaidTempestKeepStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash
    triggers.push_back(new TriggerNode("crimson hand centurion casts arcane volley",
        NextAction::array(0, new NextAction("crimson hand centurion cast polymorph", ACTION_RAID + 1), nullptr)
    ));

    // Al'ar
    triggers.push_back(new TriggerNode("al'ar pulling boss",
        NextAction::array(0, new NextAction("al'ar misdirect boss to main tank", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("al'ar boss is flying between platforms",
        NextAction::array(0,
            new NextAction("al'ar boss tanks move between platforms", ACTION_RAID + 1),
            new NextAction("al'ar melee dps move between platforms", ACTION_RAID + 1),
            new NextAction("al'ar ranged and ember tank move under platforms", ACTION_RAID + 4), nullptr)
    ));
    triggers.push_back(new TriggerNode("al'ar boss spawns embers of alar",
        NextAction::array(0,
            new NextAction("al'ar assist tanks pick up embers", ACTION_RAID + 3),
            new NextAction("al'ar ranged dps prioritize embers", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("al'ar incoming flame quills",
        NextAction::array(0, new NextAction("al'ar jump from platform", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("al'ar rising from the ashes",
        NextAction::array(0, new NextAction("al'ar move away from rebirth", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("al'ar everything is on fire in phase 2",
        NextAction::array(0,
            new NextAction("al'ar swap tanks on boss", ACTION_EMERGENCY + 2),
            new NextAction("al'ar avoid flame patches and dive bombs", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("al'ar phase 2 encounter is at room center",
        NextAction::array(0, new NextAction("al'ar return to room center", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("al'ar strategy changes between phases",
        NextAction::array(0, new NextAction("al'ar manage phase tracker", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Void Reaver
    triggers.push_back(new TriggerNode("void reaver knock away reduces tank aggro",
        NextAction::array(0,
            new NextAction("void reaver tanks position boss", ACTION_RAID + 1),
            new NextAction("void reaver ranged use aggro dump ability", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("void reaver boss launches arcane orbs",
        NextAction::array(0, new NextAction("void reaver spread ranged", ACTION_RAID + 1), nullptr)
    ));

    // High Astromancer Solarian
    triggers.push_back(new TriggerNode("high astromancer solarian boss casts wrath of the astromancer",
        NextAction::array(0, new NextAction("high astromancer solarian ranged leave space for melee", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian bot has wrath of the astromancer",
        NextAction::array(0, new NextAction("high astromancer solarian move away from group", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian boss has vanished",
        NextAction::array(0, new NextAction("high astromancer solarian stack for aoe", ACTION_RAID + 1), nullptr)
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
    triggers.push_back(new TriggerNode("kael'thas sunstrider thaladred is fixated on bot",
        NextAction::array(0, new NextAction("kael'thas sunstrider kite thaladred", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider pulling tankable advisors",
        NextAction::array(0, new NextAction("kael'thas sunstrider misdirect advisors to tanks", ACTION_EMERGENCY + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider sanguinar engaged by main tank",
        NextAction::array(0, new NextAction("kael'thas sunstrider main tank position sanguinar", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider sanguinar casts bellowing roar",
        NextAction::array(0, new NextAction("kael'thas sunstrider cast fear ward on sanguinar tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider capernian requires a warlock tank",
        NextAction::array(0, new NextAction("kael'thas sunstrider manage warlock tank strategy", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider capernian engaged by warlock tank",
        NextAction::array(0, new NextAction("kael'thas sunstrider warlock tank position capernian", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider capernian casts arcane burst and conflagration",
        NextAction::array(0, new NextAction("kael'thas sunstrider spread and move away from capernian", ACTION_RAID + 3), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider telonicus engaged by first assist tank",
        NextAction::array(0, new NextAction("kael'thas sunstrider first assist tank position telonicus", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider melee tanks need dedicated healer in phase 3",
        NextAction::array(0, new NextAction("kael'thas sunstrider position phase 3 tank healer", ACTION_RAID + 2), nullptr)
    ));
        triggers.push_back(new TriggerNode("kael'thas sunstrider determining advisor kill order",
        NextAction::array(0, new NextAction("kael'thas sunstrider assign advisor dps priority", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider waiting for tanks to get aggro on advisors",
        NextAction::array(0, new NextAction("kael'thas sunstrider manage advisor dps timer", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider legendary weapons are alive",
        NextAction::array(0,
            new NextAction("kael'thas sunstrider assign legendary weapon dps priority", ACTION_RAID + 1),
            new NextAction("kael'thas sunstrider main tank move devastation away", ACTION_EMERGENCY + 1),
            new NextAction("kael'thas sunstrider hunter turn away netherstrand longbow", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider legendary weapons are dead and lootable",
        NextAction::array(0, new NextAction("kael'thas sunstrider loot legendary weapons", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider legendary weapons are equipped",
        NextAction::array(0, new NextAction("kael'thas sunstrider use legendary weapons", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider legendary weapons were lost",
        NextAction::array(0, new NextAction("kael'thas sunstrider reequip gear", ACTION_EMERGENCY + 11), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider boss has entered the fight",
        NextAction::array(0,
            new NextAction("kael'thas sunstrider main tank position boss", ACTION_RAID + 1),
            new NextAction("kael'thas sunstrider avoid flame strike", ACTION_EMERGENCY + 8), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider phoenixes and eggs are spawning",
        NextAction::array(0, new NextAction("kael'thas sunstrider round up phoenixes and focus down eggs", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider raid member is mind controlled",
        NextAction::array(0, new NextAction("kael'thas sunstrider break mind control with infinity blade", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider boss is casting pyroblast",
        NextAction::array(0, new NextAction("kael'thas sunstrider break through shock barrier", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("kael'thas sunstrider boss is manipulating gravity",
        NextAction::array(0, new NextAction("kael'thas sunstrider spread out in midair", ACTION_EMERGENCY + 1), nullptr)
    ));
}

void RaidTempestKeepStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AlarMoveBetweenPlatformsMultiplier(botAI));
    multipliers.push_back(new AlarDisableDisperseMultiplier(botAI));
    multipliers.push_back(new AlarDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new AlarStayAwayFromRebirthMultiplier(botAI));
    // multipliers.push_back(new AlarPhase2NoTankingIfArmorMeltedMultiplier(botAI));

    multipliers.push_back(new VoidReaverDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new VoidReaverMaintainPositionsMultiplier(botAI));

    multipliers.push_back(new HighAstromancerSolarianDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new HighAstromancerSolarianMaintainPositionMultiplier(botAI));

    multipliers.push_back(new KaelthasSunstriderKiteThaladredMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderWaitForDpsMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDisableShadowWardMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderManageTankTargetingMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDisableDisperseMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderTryNonfatalBreakingOfMindControlMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderAllDpsOnBossDuringPyroblastMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier(botAI));
}
