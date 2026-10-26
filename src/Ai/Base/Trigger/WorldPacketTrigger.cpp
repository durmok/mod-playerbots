/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WorldPacketTrigger.h"
#include "Playerbots.h"

void WorldPacketTrigger::ExternalEvent(WorldPacket& revData, Player* eventOwner)
{
    packet = revData;
    owner = eventOwner;
    triggered = true;
    allPackets.emplace_back(packet, owner);
}

void WorldPacketTrigger::ExternalEvent(std::shared_ptr<ClientPacket const> revData, Player* eventOwner)
{
    clientPacket = revData->CloneShared();
    owner = eventOwner;
    triggered = true;
    allClientPackets.emplace_back(revData->CloneShared(), owner);

}

Event WorldPacketTrigger::Check()
{
    if (!triggered)
        return Event();

    if (!packet.empty())
        return Event(getName(), packet, std::move(allPackets), owner);
    else if (clientPacket)
        return Event(getName(), clientPacket, std::move(allClientPackets), owner);
}

void WorldPacketTrigger::Reset() {
    triggered = false;
    packet.clear();
    clientPacket = nullptr;
    allPackets.clear();
    allClientPackets.clear();
}
