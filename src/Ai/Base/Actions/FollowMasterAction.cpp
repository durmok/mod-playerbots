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
        !bot->IsBeingTeleported() &&
        !master->IsBeingTeleported();
}

bool FollowMasterAction::Execute(Event /*event*/)
{
    Player* master = botAI->GetMaster();
    if (!master)
        return false;

    if (master->isDead())
        return MoveNear(master, 10.f);

    if (requireTeleportToMaster(botAI))
    {
        /*
        Position pos = master->GetRandomNearPosition(5.0f);
        bot->TeleportTo({ master->GetMapId(), pos});
        if (bot->GetPet())
            bot->GetPet()->NearTeleportTo(pos);
        if (bot->GetMiniPet())
            bot->GetMiniPet()->NearTeleportTo(pos);
        */
        char name[128] = {};
        snprintf(name, sizeof(name), "%s", master->GetName());
        ChatHandler(bot).HandleGonameCommand(name);
        return true;
    }

    bool changePosition = false;
    time_t now = time(nullptr);
    if (now >= nextChangePositionTime)
    {
        nextChangePositionTime = now + urand(5, 20);
        float minDist = 1;
        float maxDist = 3.0f;
        if (bot->GetGroup() && bot->GetGroup()->GetMembersCount() >= 20)
        {
            minDist *= 2;
            maxDist *= 2;
        }
        lastDist = urand(minDist, maxDist);
        lastAngle = frand(0.0f, 6.0f);
        changePosition = true;
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
    {
        bot->GetMotionMaster()->Clear(false);
        bot->GetMotionMaster()->MoveFollow(master, lastDist, lastAngle);
    }
    else if (changePosition)
        bot->GetMotionMaster()->MoveFollow(master, lastDist, lastAngle);

    LastMovement& lastMovement = AI_VALUE(LastMovement&, "last movement");
    if (lastMovement.priority != MovementPriority::MOVEMENT_NORMAL)
        lastMovement.clear();

    return true;
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

    if (master->IsInFlight())
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
