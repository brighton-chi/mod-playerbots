#include "RaidGruulsLairActions.h"
#include "RaidGruulsLairHelpers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "UnitAI.h"

#include "Log.h"

bool HighKingMaulgarDPSPriorityAction::Execute(Event event)
{
    if (IsFirstTank(botAI, bot) || IsSecondTank(botAI, bot) || IsThirdTank(botAI, bot) || IsMageTank(botAI, bot) || botAI->IsHeal(bot))
    return false;

    Group* group = bot->GetGroup();
    if (!group) return false;

    // Set up priority list for each bot role
    std::vector<std::string> priorities;

    if (IsBoomkinTank(botAI, bot))
        priorities = { "kiggler the crazed", "blindeye the seer", "olm the summoner", "krosh firehand", "high king maulgar" };
    else if (botAI->IsMelee(bot))
        priorities = { "blindeye the seer", "olm the summoner", "kiggler the crazed", "high king maulgar" };
    else if (botAI->IsRanged(bot))
        priorities = { "blindeye the seer", "olm the summoner", "kiggler the crazed", "krosh firehand", "high king maulgar" };
    else
        return false;

    for (const auto& name : priorities)
    {
        Unit* target = AI_VALUE2(Unit*, "find target", name);
        if (target && target->IsAlive())
        {
            // Only switch if not already attacking the highest priority target
            if (bot->GetVictim() != target)
                return Attack(target);
            // Already attacking the correct target, do nothing
            return false;
        }
    }
    return false;
}

bool HighKingMaulgarMaulgarTankAction::Execute(Event event)
{
    if (!IsFirstTank(botAI, bot))
        return false;
        
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (maulgar && maulgar->IsAlive()) {
        // Only switch if not already attacking Maulgar
        if (bot->GetVictim() != maulgar)
            Attack(maulgar);
        // Only move/facing if Maulgar is targeting the tank
        if (maulgar->GetVictim() == bot) {
            const TankSpot& spot = GruulsLairTankSpots::Maulgar;
            const float maxDistance = 3.0f;
            float distanceToBossPosition = maulgar->GetExactDist2d(spot.x, spot.y);

            float moveX = spot.x;
            float moveY = spot.y;
            if (distanceToBossPosition > maxDistance)
            {
                float dX = spot.x - maulgar->GetPositionX();
                float dY = spot.y - maulgar->GetPositionY();
                moveX = spot.x + (dX / distanceToBossPosition) * maxDistance;
                moveY = spot.y + (dY / distanceToBossPosition) * maxDistance;
            }
            MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            float orientation = atan2(maulgar->GetPositionY() - bot->GetPositionY(), maulgar->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
        return true;
    }
    // If Maulgar is dead, fallback to priority attack logic
    std::vector<std::string> priorities = { "blindeye the seer", "olm the summoner", "kiggler the crazed" };
    for (const auto& name : priorities)
    {
        Unit* target = AI_VALUE2(Unit*, "find target", name);
        if (target && target->IsAlive())
        {
            if (bot->GetVictim() != target)
                return Attack(target);

            return false;
        }
    }
    return false;
}

bool HighKingMaulgarOlmTankAction::Execute(Event event)
{
    if (!IsSecondTank(botAI, bot))
        return false;

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (olm && olm->IsAlive()) {
        // Only switch if not already attacking Olm
        if (bot->GetVictim() != olm)
            Attack(olm);
        // Only move/facing if Olm is targeting the tank
        if (olm->GetVictim() == bot) {
            const TankSpot& spot = GruulsLairTankSpots::Olm;
            const float maxDistance = 3.0f;
            float distanceToBossPosition = olm->GetExactDist2d(spot.x, spot.y);

            float moveX = spot.x;
            float moveY = spot.y;
            if (distanceToBossPosition > maxDistance)
            {
                float dX = spot.x - olm->GetPositionX();
                float dY = spot.y - olm->GetPositionY();
                moveX = spot.x + (dX / distanceToBossPosition) * maxDistance;
                moveY = spot.y + (dY / distanceToBossPosition) * maxDistance;
            }
            MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            float orientation = atan2(olm->GetPositionY() - bot->GetPositionY(), olm->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
        return true;
    }
    // If Olm is dead, fallback to priority attack logic
    std::vector<std::string> priorities = { "blindeye the seer", "kiggler the crazed", "high king maulgar" };
    for (const auto& name : priorities)
    {
        Unit* target = AI_VALUE2(Unit*, "find target", name);
        if (target && target->IsAlive())
        {
            if (bot->GetVictim() != target)
                return Attack(target);

            return false;
        }
    }
    return false;
}

bool HighKingMaulgarBlindeyeTankAction::Execute(Event event)
{
    if (!IsThirdTank(botAI, bot))
        return false;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (blindeye && blindeye->IsAlive()) {
        // Only switch if not already attacking Blindeye
        if (bot->GetVictim() != blindeye)
            Attack(blindeye);
        // Only move/facing if Blindeye is targeting the tank
        if (blindeye->GetVictim() == bot) {
            const TankSpot& spot = GruulsLairTankSpots::Blindeye;
            const float maxDistance = 3.0f;
            float distanceToBossPosition = blindeye->GetExactDist2d(spot.x, spot.y);

            float moveX = spot.x;
            float moveY = spot.y;
            if (distanceToBossPosition > maxDistance)
            {
                float dX = spot.x - blindeye->GetPositionX();
                float dY = spot.y - blindeye->GetPositionY();
                moveX = spot.x + (dX / distanceToBossPosition) * maxDistance;
                moveY = spot.y + (dY / distanceToBossPosition) * maxDistance;
            }
            MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            float orientation = atan2(blindeye->GetPositionY() - bot->GetPositionY(), blindeye->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
        return true;
    }
    // If Blindeye is dead, fallback to priority attack logic
    std::vector<std::string> priorities = { "olm the summoner", "kiggler the crazed", "high king maulgar" };
    for (const auto& name : priorities)
    {
        Unit* target = AI_VALUE2(Unit*, "find target", name);
        if (target && target->IsAlive())
        {
            if (bot->GetVictim() != target)
                return Attack(target);

            return false;
        }
    }
    return false;
}

bool HighKingMaulgarMageTankAction::Execute(Event event)
{
    if (!IsMageTank(botAI, bot))
        return false;
    
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (krosh && krosh->IsAlive()) {
        float distance = bot->GetDistance2d(krosh);
        if (distance > 30.0f)
        {
            return MoveTo(krosh, 30.0f);
        }
        // Only switch if not already attacking Krosh
 
        // 1. If Krosh has Spell Shield, cast Spellsteal
        if (krosh->HasAura(SPELL_AURA_SPELL_SHIELD))
        {
            botAI->CastSpell("spellsteal", krosh);
            return true;
        }
        // 2. If mage does not have Spell Shield, cast Fire Ward on self
        if (!bot->HasAura(SPELL_AURA_SPELL_SHIELD))
        {
            botAI->CastSpell("fire ward", bot);
            return true;
        }
        if (bot->GetVictim() != krosh)
            Attack(krosh);

        return true;
    }
    // If Krosh is dead, fallback to priority attack logic
    std::vector<std::string> priorities = { "blindeye the seer", "olm the summoner", "kiggler the crazed", "high king maulgar" };
    for (const auto& name : priorities)
    {
        Unit* target = AI_VALUE2(Unit*, "find target", name);
        if (target && target->IsAlive())
        {
            if (bot->GetVictim() != target)
                return Attack(target);

            return false;
        }
    }
    return false;
}

bool HighKingMaulgarAvoidBlastWaveAction::Execute(Event event)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    float safeDistance = 21.0f;
    if (bot->GetDistance(krosh) < safeDistance)
    {
        MoveTo(krosh, safeDistance);
        return true;
    }
    return false;
}

bool HighKingMaulgarAvoidBlastWaveAction::isUseful()
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");

    return krosh && krosh->IsAlive() && bot->GetDistance2d(krosh) < 21.0f;
}

bool HighKingMaulgarControlFelstalkerAction::Execute(Event event)
{
    if (Unit* charm = bot->GetCharm())
    {
        Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");
        if (!felStalker || !felStalker->IsAlive())
            return false;

        if (charm->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_ACTIVE) == NULL_MOTION_TYPE)
        {
            charm->GetMotionMaster()->Clear();
            charm->GetMotionMaster()->MoveChase(felStalker);
            charm->GetAI()->AttackStart(felStalker);
        }
        Aura* aura = botAI->GetAura("enslave demon", charm);
        
        if (!aura)
            return false;
        
        const GuidVector party = AI_VALUE(GuidVector, "party");
        for (const auto& guid : party)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || !member->IsAlive())
                continue;
            if (member->HasAura(SPELL_AURA_DARK_DECAY) && !charm->HasSpellCooldown(SPELL_DETERMINATION))
            {
                charm->CastSpell(member, SPELL_DETERMINATION, true);
                charm->AddSpellCooldown(SPELL_DETERMINATION, 0, 10 * 1000);
                return true;
            }
        }
        Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
        if (olm && olm->IsAlive())
        {
            if (!charm->HasSpellCooldown(SPELL_THREATEN))
            {
                charm->CastSpell(olm, SPELL_THREATEN, true);
                charm->AddSpellCooldown(SPELL_THREATEN, 0, 20 * 1000);
                return true;
            }
            if (!charm->HasSpellCooldown(SPELL_WILD_BITE))
            {
                charm->CastSpell(olm, SPELL_WILD_BITE, true);
                charm->AddSpellCooldown(SPELL_WILD_BITE, 0, 5 * 1000);
                return true;
            }
        }
        else
        {
            Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
            if (krosh && krosh->IsAlive() && !charm->HasSpellCooldown(SPELL_WILD_BITE))
            {
                charm->CastSpell(krosh, SPELL_WILD_BITE, true);
                charm->AddSpellCooldown(SPELL_WILD_BITE, 0, 5 * 1000);
                return true;
            }
            Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
            if (maulgar && maulgar->IsAlive() && !charm->HasSpellCooldown(SPELL_WILD_BITE))
            {
                charm->CastSpell(maulgar, SPELL_WILD_BITE, true);
                charm->AddSpellCooldown(SPELL_WILD_BITE, 0, 5 * 1000);
                return true;
            }
        }
    }
    else
    {
        {
            Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");
            if (felStalker && felStalker->IsAlive() && !felStalker->HasAura(SPELL_ENSLAVE_DEMON))
            {
                if (bot->GetDistance2d(felStalker) > sPlayerbotAIConfig->spellDistance)
                {
                    return MoveNear(felStalker, sPlayerbotAIConfig->spellDistance, MovementPriority::MOVEMENT_COMBAT);
                }
                else
                {
                    return botAI->CastSpell(SPELL_ENSLAVE_DEMON, felStalker);
                }
            }
        }
    }
    return false;
}

bool HighKingMaulgarControlFelstalkerAction::isUseful()
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    return felStalker && felStalker->IsAlive() && bot->getClass() == CLASS_WARLOCK;
}

bool HighKingMaulgarAvoidWhirlwindAction::Execute(Event event)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    float safeDistance = 46.0f;
    if (bot->GetDistance(maulgar) < safeDistance)
    {
        MoveTo(maulgar, safeDistance);
        return true;
    }
    return false;
}

bool HighKingMaulgarAvoidWhirlwindAction::isUseful()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    
    return maulgar && maulgar->IsAlive() && maulgar->HasUnitState(UNIT_STATE_CASTING) 
    && maulgar->FindCurrentSpellBySpellId(SPELL_WHIRLWIND) && !botAI->IsTank(bot);
}

bool HighKingMaulgarHunterMisdirectionAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot) continue;
        Aura* aura = member->GetAura(SPELL_AURA_MISDIRECTION);
        if (aura && aura->GetCasterGUID() == bot->GetGUID())
        {
            Unit* target = member->GetVictim();
            if (!target) return false;
            botAI->CastSpell("aimed shot", target);
            botAI->CastSpell("steady shot", target);
            return true;
        }
    }
    return false;
}
