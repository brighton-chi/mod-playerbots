#include "MagHelpers.h"
#include "AllSpellScript.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace MagtheridonHelpers;

// Records the position of each SPELL_DEBRIS_SPAWN cast so that cube clickers
// can avoid standing where debris will land. The spell is cast by the targeted
// NPC_TARGET_TRIGGER on itself, so the caster's position is the debris location.
class MagtheridonDebrisScript : public AllSpellScript
{
public:
    MagtheridonDebrisScript() : AllSpellScript("MagtheridonDebrisScript") {}

    void OnSpellCast(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id != static_cast<uint32>(MagtheridonSpells::SPELL_DEBRIS_SPAWN))
            return;

        uint32 const instanceId = caster->GetMap()->GetInstanceId();
        uint32 const now = getMSTime();

        activeDebrisPositions[instanceId].push_back({ caster->GetPosition(), now });

        // Purge entries older than 10 seconds
        auto& positions = activeDebrisPositions[instanceId];
        positions.erase(std::remove_if(positions.begin(), positions.end(),
            [now](DebrisData const& d) { return getMSTimeDiff(d.spawnTime, now) > 10000; }),
            positions.end());
    }
};

void AddSC_MagtheridonBotScripts()
{
    new MagtheridonDebrisScript();
}
