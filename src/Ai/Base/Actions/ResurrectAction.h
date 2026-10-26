/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RESURRECTACTION_H
#define PLAYERBOTS_RESURRECTACTION_H

#include "MovementActions.h"

class Player;
class PlayerbotAI;

class ResurrectAction : public MovementAction
{
public:
    ResurrectAction(PlayerbotAI* botAI, std::string const name = "resurrect") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
