/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PartykilllogAction.h"
#include "Event.h"
#include "LootObjectStack.h"
#include "Playerbots.h"

bool PartykilllogAction::ExecutePacket(WorldPacket& packet)
{
    if (Group const* group = bot->GetGroup())
    {
        if (group->GetLootMethod() == ROUND_ROBIN ||
            group->GetLootMethod() == GROUP_LOOT ||
            group->GetLootMethod() == NEED_BEFORE_GREED)
        {
            ObjectGuid guid = ObjectGuid(*(((uint64*)(packet).contents()) + 1));
            Unit* unit = botAI->GetUnit(guid);
            if (!unit)
                return false;
            Creature* creature = unit->ToCreature();
            if (!creature)
                return false;
            ObjectGuid roundRobinPlayer = creature->loot.roundRobinPlayer;
            if (roundRobinPlayer != bot->GetGUID())
                return false;
            return AI_VALUE(LootObjectStack*, "available loot")->Add(guid);
        }
    }
    return true;
}
