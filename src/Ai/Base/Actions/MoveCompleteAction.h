/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MOVECOMPLETEACTION_H
#define PLAYERBOTS_MOVECOMPLETEACTION_H

#include "Action.h"
#include "Playerbots.h"

class PlayerbotAI;

class MoveCompleteAction : public Action
{
public:
    MoveCompleteAction(PlayerbotAI* botAI) : Action(botAI, "move complete") {}

    bool Execute(Event event);
};

#endif
