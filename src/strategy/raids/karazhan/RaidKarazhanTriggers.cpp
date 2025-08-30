#include "Playerbots.h"
#include "RaidKarazhanTriggers.h"

bool KarazhanMoroesTrigger::IsActive()
{
    Unit* moroes = AI_VALUE2(Unit*, "find target", "moroes");
    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");

    if (!moroes && !dorothea && !catriona && !keira && !rafe && !robin && !crispin)
        return false;

    return true;
}

bool KarazhanMaidenOfVirtueTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    if (!boss)
        return false;

    return true;
}

bool KarazhanBigBadWolfTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    if (!boss)
        return false;

    return true;
}

bool KarazhanRomuloJulianneTrigger::IsActive()
{
    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");

    if (!julianne || !romulo)
        return false;

    return true;
}

bool KarazhanWizardOfOzTrigger::IsActive()
{
    Unit* dorothee = AI_VALUE2(Unit*, "find target", "Dorothee");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");

    if (!dorothee && !strawman && !tinhead && !tito && !roar && !crone)
        return false;

    return true;
}

bool KarazhanTheCuratorTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");

    if (!boss)
        return false;

    return true;
}

bool KarazhanTerestianIllhoofTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "terestian illhoof");

    if (!boss)
        return false;

    return true;
}

bool KarazhanShadeOfAranTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");

    if (!boss)
        return false;

    return true;
}

/*bool KarazhanNetherspiteTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    if (!boss)
        return false;

    return true;
}*/
