/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PARTYKILLLOGACTION_H
#define PLAYERBOTS_PARTYKILLLOGACTION_H

#include "Action.h"

class PartykilllogAction : public Action
{
public:
    PartykilllogAction(PlayerbotAI* botAI) : Action(botAI, "partykilllog") {}

    bool Execute(Event event) {
        bool ret = false;
        auto packets = event.getAllPackets();
        for (auto itr = packets.begin(); itr != packets.end(); ++itr)
            ret |= ExecutePacket(itr->first);
        return ret;
    }

    bool ExecutePacket(WorldPacket& packet);
};

#endif
