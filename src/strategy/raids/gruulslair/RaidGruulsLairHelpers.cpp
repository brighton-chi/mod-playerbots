#include "RaidGruulsLairHelpers.h"
#include "AiFactory.h"
#include "PlayerbotAI.h"
#include "Group.h"
#include "GroupReference.h"
#include "Unit.h"

// Returns true if the bot is the mage with the highest max HP in the group (mage tank)
bool IsMageTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* highestHpMage = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;
        if (member->getClass() == CLASS_MAGE)
        {
            uint32 hp = member->GetMaxHealth();
            if (!highestHpMage || hp > highestHp)
            {
                highestHpMage = member;
                highestHp = hp;
            }
        }
    }
    return highestHpMage == bot;
}

// Returns true if the bot is the balance druid with the highest max HP in the group (boomkin tank)
bool IsBoomkinTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* highestHpBoomkin = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;
        // Check if member is a balance druid (tab == DRUID_TAB_BALANCE)
        if (member->getClass() == CLASS_DRUID)
        {
            int tab = AiFactory::GetPlayerSpecTab(member);
            if (tab == DRUID_TAB_BALANCE)
            {
                uint32 hp = member->GetMaxHealth();
                if (!highestHpBoomkin || hp > highestHp)
                {
                    highestHpBoomkin = member;
                    highestHp = hp;
                }
            }
        }
    }
    return highestHpBoomkin == bot;
}

bool IsFirstTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
            return member == bot;
    }
    return false;
}

bool IsSecondTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
        return false;

    int tankIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            if (tankIndex == 1)
                return member == bot;
            ++tankIndex;
        }
    }
    return false;
}

bool IsThirdTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
        return false;

    int tankIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            if (tankIndex == 2)
                return member == bot;
            ++tankIndex;
        }
    }
    return false;
}

// Helper function to check if a position is safe from all boss mechanics
bool IsPositionSafe(PlayerbotAI* botAI, Unit* bot, Position pos)
{
    // Safety distances
    const float KROSH_SAFE_DISTANCE = 21.0f;
    const float MAULGAR_WHIRLWIND_DISTANCE = 46.0f;
    
    bool isSafe = true;

    // Find Krosh using botAI
    Unit* krosh = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "krosh firehand")->Get();
    
    // Check Krosh's blast wave safety
    if (krosh && krosh->IsAlive())
    {
        float dist = sqrt(pow(pos.GetPositionX() - krosh->GetPositionX(), 2) + 
                          pow(pos.GetPositionY() - krosh->GetPositionY(), 2));
        if (dist < KROSH_SAFE_DISTANCE)
        {
            isSafe = false;
            LOG_DEBUG("playerbots", "Position unsafe from Krosh - distance: {}", dist);
        }
    }

    // Find Maulgar using botAI
    Unit* maulgar = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "high king maulgar")->Get();
    
    // Check Maulgar's whirlwind safety (only if he's casting it)
    if (maulgar && maulgar->IsAlive() && maulgar->HasUnitState(UNIT_STATE_CASTING) && 
        maulgar->FindCurrentSpellBySpellId(33238)) // Use actual spell ID for Whirlwind
    {
        float dist = sqrt(pow(pos.GetPositionX() - maulgar->GetPositionX(), 2) + 
                          pow(pos.GetPositionY() - maulgar->GetPositionY(), 2));
        if (dist < MAULGAR_WHIRLWIND_DISTANCE)
        {
            isSafe = false;
            LOG_DEBUG("playerbots", "Position unsafe from Maulgar's Whirlwind - distance: {}", dist);
        }
    }
    
    return isSafe;
}

// Helper function to find a safe position while still being optimal for target
Position FindSafePosition(PlayerbotAI* botAI, Unit* bot, Unit* target, float optimalDistance)
{
    Position bestPos;
    bestPos.m_positionX = bot->GetPositionX();
    bestPos.m_positionY = bot->GetPositionY();
    bestPos.m_positionZ = bot->GetPositionZ();
    
    // Find the dangerous bosses using botAI
    Unit* krosh = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "krosh firehand")->Get();
    Unit* maulgar = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "high king maulgar")->Get();

    // If no dangerous bosses are alive, return current position
    bool dangerousKrosh = krosh && krosh->IsAlive();
    bool dangerousMaulgar = maulgar && maulgar->IsAlive() && 
                          maulgar->HasUnitState(UNIT_STATE_CASTING) && 
                          maulgar->FindCurrentSpellBySpellId(33238);
    
    if (!dangerousKrosh && !dangerousMaulgar)
    {
        return bestPos;
    }
    
    // Check if current position is safe
    if (IsPositionSafe(botAI, bot, bestPos))
    {
        return bestPos;
    }
    
    // Try to find a safe position in a circle around the target
    const int NUM_POSITIONS = 16; // Try 16 positions around the circle
    float bestScore = 99999.0f;
    bool foundSafeSpot = false;
    
    for (int i = 0; i < NUM_POSITIONS; i++)
    {
        float angle = 2 * M_PI * i / NUM_POSITIONS;
        Position candidatePos;
        candidatePos.m_positionX = target->GetPositionX() + optimalDistance * cos(angle);
        candidatePos.m_positionY = target->GetPositionY() + optimalDistance * sin(angle);
        candidatePos.m_positionZ = target->GetPositionZ();
        
        if (IsPositionSafe(botAI, bot, candidatePos))
        {
            // Calculate score based on how far this position is from the bot's current position
            float movementDistance = sqrt(pow(candidatePos.GetPositionX() - bot->GetPositionX(), 2) + 
                                         pow(candidatePos.GetPositionY() - bot->GetPositionY(), 2));
            
            // Prefer positions that require less movement
            if (movementDistance < bestScore)
            {
                bestScore = movementDistance;
                bestPos = candidatePos;
                foundSafeSpot = true;
            }
        }
    }
    
    // If we found a safe position, use it
    if (foundSafeSpot)
    {
        LOG_DEBUG("playerbots", "Found safe position for {} at ({}, {})", 
                bot->GetName(), bestPos.GetPositionX(), bestPos.GetPositionY());
        return bestPos;
    }
    
    // If no safe positions were found, at least move away from dangerous bosses
    LOG_DEBUG("playerbots", "No safe position found for {}, attempting escape maneuver", bot->GetName());
    
    // Calculate vector away from dangerous bosses
    float dx = 0, dy = 0;
    if (dangerousKrosh && bot->GetDistance(krosh) < 30.0f)
    {
        dx += bot->GetPositionX() - krosh->GetPositionX();
        dy += bot->GetPositionY() - krosh->GetPositionY();
    }
    
    if (dangerousMaulgar && bot->GetDistance(maulgar) < 50.0f)
    {
        dx += bot->GetPositionX() - maulgar->GetPositionX();
        dy += bot->GetPositionY() - maulgar->GetPositionY();
    }
    
    // Normalize and scale the escape vector
    float dist = sqrt(dx*dx + dy*dy);
    if (dist > 0.1f)
    {
        float escapeDistance = 20.0f; // Move this far away
        bestPos.m_positionX = bot->GetPositionX() + (dx/dist) * escapeDistance;
        bestPos.m_positionY = bot->GetPositionY() + (dy/dist) * escapeDistance;
        bestPos.m_positionZ = bot->GetPositionZ();
    }
    
    return bestPos;
}
