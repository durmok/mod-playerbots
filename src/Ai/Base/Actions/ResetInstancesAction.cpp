/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ResetInstancesAction.h"
// #include "InstancePackets.h"
#include "PlayerbotAI.h"

bool ResetInstancesAction::Execute(Event /*event*/)
{
    NullClientPacket packet(CMSG_RESET_INSTANCES);
    // WorldPackets::Instance::ResetInstances resetInstance(std::move(packet));
    bot->GetSession()->HandleResetInstancesOpcode(packet);

    return true;
}

bool ResetInstancesAction::isUseful() { return botAI->GetGroupLeader() == bot; };
