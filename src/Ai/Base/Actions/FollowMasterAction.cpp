/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FollowMasterAction.h"
#include "Corpse.h"
#include "Event.h"
#include "MotionMaster.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool requireTeleportToMaster(PlayerbotAI* botAI)
{
    Player* master = botAI->GetMaster();
    Player* bot = botAI->GetBot();
    return master &&
        master->IsInWorld() &&
        bot->IsInWorld() &&
        !bot->IsWithinDistInMap(master, sPlayerbotAIConfig.SightDistance) &&
        !bot->isDead() &&
        !master->isDead() &&
        (!bot->IsInCombat() || botAI->HasStrategy("passive", BOT_STATE_COMBAT)) &&
        !bot->GetVehicle() &&
        !bot->IsBeingTeleported() &&
        !master->IsBeingTeleported();
}

bool FollowMasterAction::Execute(Event /*event*/)
{
    Player* master = botAI->GetMaster();
    if (!master)
        return false;

    if (master->isDead())
        return MoveNear(master, MOVE_CORPSE_DIST);

    if (requireTeleportToMaster(botAI))
    {
        Position pos = master->GetRandomNearPosition(5.0f);
        bot->TeleportTo({ master->GetMapId(), pos});
        if (bot->GetPet())
            bot->GetPet()->NearTeleportTo(pos);
        if (bot->GetGuardianPet())
            bot->GetGuardianPet()->NearTeleportTo(pos);
        return true;
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
        return false;

    time_t now = time(nullptr);
    if (now <= lastCheckTime)
        return false;
    lastCheckTime = now;
    if (now >= nextChangeTime)
    {
        nextChangeTime = now + urand(5, 20);
        float minDist = 0.5;
        float maxDist = 3.0f;
        if (bot->GetGroup() && bot->GetGroup()->GetMembersCount() >= MOVE_CORPSE_DIST)
        {
            minDist += 1.0f;
            maxDist += 2.0f;
        }
        lastDist = urand(minDist, maxDist);
        lastAngle = frand(0.0f, 6.0f);
    }

    return Follow(master, lastDist, lastAngle);
}

bool FollowMasterAction::isUseful()
{
    // move from group takes priority over follow as it's added and removed automatically
    // (without removing/adding follow)
    if (botAI->HasStrategy("move from group", BOT_STATE_COMBAT) ||
        botAI->HasStrategy("move from group", BOT_STATE_NON_COMBAT))
        return false;

    if (bot->isDead())
        return false;

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        return false;

    Player* master = botAI->GetMaster();
    if (!master)
        return false;

    if (master->isDead())
    {
        Corpse* corpse = master->GetCorpse();
        return corpse && bot->GetDistance2d(corpse) >= 30.0;
    }

    if (master->IsInFlight() || master->GetTransport())
        return false;

    if (master->IsMounted() != bot->IsMounted())
    {
        if ((master->IsMounted() && bot->IsOutdoors()) || !master->IsMounted())
            return false;
    }

    if (requireTeleportToMaster(botAI))
        return true;

    return bot->GetMotionMaster() != nullptr;
}

void FollowMasterAction::SetUseless()
{
    MotionMaster* mm = bot->GetMotionMaster();
    if (!mm)
        return;
    if (mm->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        mm->Clear(false);
        bot->StopMoving();
    }
}
