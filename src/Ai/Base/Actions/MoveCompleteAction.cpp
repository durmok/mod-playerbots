/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MoveCompleteAction.h"
#include "Playerbots.h"
#include "WaitMoveCompleteValue.h"

class PlayerbotAI;

bool MoveCompleteAction::Execute(Event /*event*/)
{
    AI_VALUE(WaitMoveComplete*, "wait move complete")->Stop();
    return true;
}
