/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AreaTriggerAction.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "Transport.h"

bool ReachAreaTriggerAction::Execute(Event event)
{
    if (IsSelfBot(bot))  // SelfBots reach their own area triggers through the client.
        return false;

    uint32 triggerId;
    WorldPacket p(event.getPacket());
    p.rpos(0);
    p >> triggerId;

    AreaTriggerEntry const* at = sObjectMgr.GetAreaTrigger(triggerId);
    if (!at)
        return false;

    if (!sObjectMgr.GetAreaTriggerTeleport(triggerId))
    {
        WorldPackets::Misc::AreaTrigger packet;
        packet.triggerId = triggerId;
        bot->GetSession()->HandleAreaTriggerOpcode(packet);

        return true;
    }

    if (bot->GetMapId() != at->map_id)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "area_trigger_follow_too_far_error", "I won't follow: too far away", {}));
        return true;
    }

    bot->GetMotionMaster()->MovePoint(
        /*id*/ at->map_id,
        /*coords*/ at->x, at->y, at->z,
        /*forcedMovement*/ MOVE_RUN_MODE | MOVE_PATHFINDING,
        /*speed*/ 0.0f,             // default speed (not handled here)
        /*orientation*/ 0.0f);       // keep current orientation of bot

    float distance = bot->GetDistance(at->x, at->y, at->z);
    float delay = IN_MILLISECONDS * distance / bot->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig.ReactDelay;
    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "area_trigger_wait_for_me", "Wait for me", {}));
    botAI->SetNextCheckDelay(delay);
    context->GetValue<LastMovement&>("last area trigger")->Get().lastAreaTrigger = triggerId;

    return true;
}

bool AreaTriggerAction::Execute(Event /*event*/)
{
    LastMovement& movement = context->GetValue<LastMovement&>("last area trigger")->Get();

    uint32 triggerId = movement.lastAreaTrigger;
    movement.lastAreaTrigger = 0;

    if (!sObjectMgr.GetAreaTrigger(triggerId))
        return false;

    if (!sObjectMgr.GetAreaTriggerTeleport(triggerId))
        return true;

    WorldPackets::Misc::AreaTrigger p;
    p.triggerId = triggerId;
    bot->GetSession()->HandleAreaTriggerOpcode(p);

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault("hello", "Hello", {}));
    return true;
}
