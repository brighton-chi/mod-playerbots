#include "MagHelpers.h"
#include "AllSpellScript.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace MagtheridonHelpers;

class MagtheridonBotSpellScript : public AllSpellScript
{
public:
    MagtheridonBotSpellScript() : AllSpellScript("MagtheridonBotSpellScript") {}

    void OnSpellCast(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        const uint32 instanceId = caster->GetMap()->GetInstanceId();

        if (spellInfo->Id == static_cast<uint32>(MagtheridonSpells::SPELL_DEBRIS_SPAWN))
        {
            // Debris is a one-shot that has no prior warning other than a visual effect,
            // which necessitates this spell hook to track debris spawn positions
            const uint32 now = getMSTime();

            activeDebrisPositions[instanceId].push_back({ caster->GetPosition(), now });

            // Purge entries older than 10 seconds
            auto& positions = activeDebrisPositions[instanceId];
            positions.erase(std::remove_if(positions.begin(), positions.end(),
                [now](DebrisData const& d) { return getMSTimeDiff(d.spawnTime, now) > 10000; }),
                positions.end());
        }
        else if (spellInfo->Id == static_cast<uint32>(MagtheridonSpells::SPELL_QUAKE))
        {
            auto it = blastNovaTimer.find(instanceId);
            if (it != blastNovaTimer.end())
            {
                it->second += 7;
                LOG_INFO("playerbots", "Mag: Quake +7s, blastNovaTimer={}", it->second);
            }
        }
    }
};

void AddSC_MagtheridonBotScripts()
{
    new MagtheridonBotSpellScript();
}
