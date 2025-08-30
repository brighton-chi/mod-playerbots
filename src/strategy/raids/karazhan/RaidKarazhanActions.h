#ifndef _PLAYERBOT_RAIDKARAZHANACTIONS_H
#define _PLAYERBOT_RAIDKARAZHANACTIONS_H

#include "AttackAction.h"

enum KarazhanSpells
{
    // Maiden of Virtue
    SPELL_REPENTANCE                = 29511,

    // The Big Bad Wolf
    SPELL_LITTLE_RED_RIDING_HOOD    = 30756,

    // Shade of Aran
    SPELL_FLAME_WREATH              = 29946,
    SPELL_ARCANE_EXPLOSION          = 29973,

    // Netherspite
    //SPELL_RED_BUFF                  = 30421,
    //SPELL_GREEN_BUFF                = 30422,
    //SPELL_BLUE_BUFF                 = 30423,

    //SPELL_BLUE_CHANNEL              = 30463,
    //SPELL_GREEN_CHANNEL             = 30464,
    //SPELL_RED_CHANNEL               = 30465,

    //SPELL_RED_DEBUFF                = 38637,
    //SPELL_GREEN_DEBUFF              = 38638,
    //SPELL_BLUE_DEBUFF               = 38639,

    //SPELL_BANISH                    = 39833
};

enum KarazhanNpcs
{
    // Terestian Illhoof
    NPC_DEMON_CHAINS                = 17248,

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL          = 17167,

    // Netherspite
    //NPC_VOIDZONE                    = 16697,
    //NPC_RED_PORTAL                  = 17369,
    //NPC_BLUE_PORTAL                 = 17368,
    //NPC_GREEN_PORTAL                = 17367
};

const Position KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION = Position(-10945.881f, -2103.7817f, 92.71163f);
const Position KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[8] =
{
    { -10931.178f, -2116.58f, 92.1787f },
    { -10925.828f, -2102.425f, 92.18016f },
    { -10933.089f, -2088.5017f, 92.18028f },
    { -10947.59f, -2082.8147f, 92.18024f },
    { -10960.912f, -2090.4368f, 92.17964f },
    { -10966.017f, -2105.288f, 92.17582f },
    { -10959.242f, -2119.6172f, 92.18062f },
    { -10944.495f, -2123.857f, 92.18021f },
};

const Position KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[4] = {
    { -10913.391f, -1773.5083f, 90.47706f },
    { -10875.456f, -1779.0358f, 90.47706f },
    { -10872.281f, -1751.6376f, 90.47716f },
    { -10910.492f, -1747.401f, 90.477165f }
};

const Position KARAZHAN_THE_CURATOR_BOSS_POSITION = Position(-11139.463f, -1884.6451f, 165.76564f);

class KarazhanMoroesMarkTargetAction : public AttackAction
{
public:
    KarazhanMoroesMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan moroes mark target") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
};

class KarazhanMaidenOfVirtuePositionBossAction : public AttackAction
{
public:
    KarazhanMaidenOfVirtuePositionBossAction(PlayerbotAI* botAI, std::string const name = "karazhan maiden of virtue position boss") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
    bool isUseful() override;
};

class KarazhanMaidenOfVirtuePositionRangedAction : public AttackAction
{
public:
    KarazhanMaidenOfVirtuePositionRangedAction(PlayerbotAI* botAI, std::string const name = "karazhan maiden of virtue position ranged") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
    bool isUseful() override;
};

class KarazhanBigBadWolfRunAwayAction : public AttackAction
{
public:
    KarazhanBigBadWolfRunAwayAction(PlayerbotAI* botAI, std::string const name = "karazhan big bad wolf run away") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
    bool isUseful() override;

private:
    size_t currentIndex = 0;
};

class KarazhanRomuloJulianneMarkTargetAction : public AttackAction
{
public:
    KarazhanRomuloJulianneMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan romulo julianne mark target") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
};

class KarazhanWizardOfOzMarkTargetAction : public AttackAction
{
public:
    KarazhanWizardOfOzMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan wizard of oz mark target") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
};

class KarazhanTheCuratorMarkTargetAction : public AttackAction
{
public:
    KarazhanTheCuratorMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan the curator mark target") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
};

class KarazhanTheCuratorPositionBossAction : public AttackAction
{
public:
    KarazhanTheCuratorPositionBossAction(PlayerbotAI* botAI, std::string const name = "karazhan the curator position boss") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
    bool isUseful() override;
};

class KarazhanTheCuratorSpreadRangedAction : public AttackAction
{
public:
    KarazhanTheCuratorSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "karazhan the curator spread ranged") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
    bool isUseful() override;
};

class KarazhanTerestianIllhoofMarkTargetAction : public AttackAction
{
public:
    KarazhanTerestianIllhoofMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan terestian illhoof mark target") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
};

class KarazhanShadeOfAranArcaneExplosionAction : public AttackAction
{
public:
    KarazhanShadeOfAranArcaneExplosionAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran arcane explosion") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
    bool isUseful() override;
};

class KarazhanShadeOfAranFlameWreathAction : public AttackAction
{
public:
    KarazhanShadeOfAranFlameWreathAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran flame wreath") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
    bool isUseful() override;
};

class KarazhanShadeOfAranMarkTargetAction : public AttackAction
{
public:
    KarazhanShadeOfAranMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran mark target") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
};

class KarazhanShadeOfAranSpreadRangedAction : public AttackAction
{
public:
    KarazhanShadeOfAranSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran spread ranged") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*/) override;
    bool isUseful() override;
};

/*class KarazhanNetherspiteSoakBeamsAction : public AttackAction
{
public:
    KarazhanNetherspiteSoakBeamsAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite soak beams") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*//*) override;

private:
    uint32 lastMoveTime = 0;
};

class KarazhanNetherspiteAvoidVoidZoneAction : public AttackAction
{
public:
    KarazhanNetherspiteAvoidVoidZoneAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite avoid void zone") : AttackAction(botAI, name) {}

    bool Execute(Event /*event*//*) override;
};*/

#endif
