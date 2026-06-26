#include "MagHelpers.h"
#include "AllSpellScript.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace MagtheridonHelpers;

// Tracks SPELL_DEBRIS_SPAWN positions for avoidance and SPELL_QUAKE to adjust
// Blast Nova timing (Quake's DelayAll pushes the next scheduled Blast Nova by 7s).
class MagtheridonBotSpellScript : public AllSpellScript
{
public:
    MagtheridonBotSpellScript() : AllSpellScript("MagtheridonBotSpellScript") {}

    void OnSpellCast(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id == static_cast<uint32>(MagtheridonSpells::SPELL_DEBRIS_SPAWN))
        {
            uint32 const instanceId = caster->GetMap()->GetInstanceId();
            uint32 const now = getMSTime();

            activeDebrisPositions[instanceId].push_back({ caster->GetPosition(), now });

            // Purge entries older than 10 seconds
            auto& positions = activeDebrisPositions[instanceId];
            positions.erase(std::remove_if(positions.begin(), positions.end(),
                [now](DebrisData const& d) { return getMSTimeDiff(d.spawnTime, now) > 10000; }),
                positions.end());
        }
        else if (spellInfo->Id == static_cast<uint32>(MagtheridonSpells::SPELL_QUAKE))
        {
            // Quake's DelayAll(6999ms) pushes the next scheduled Blast Nova by 7 seconds
            uint32 const instanceId = caster->GetMap()->GetInstanceId();
            auto it = blastNovaTimer.find(instanceId);
            if (it != blastNovaTimer.end())
                it->second += 7;
        }
    }
};

void AddSC_MagtheridonBotScripts()
{
    new MagtheridonBotSpellScript();
}
