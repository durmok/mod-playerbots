/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WaitMoveCompleteMultiplier.h"
#include "WaitMoveCompleteValue.h"
#include "Action.h"
#include "Playerbots.h"
#include "PullStrategy.h"

WaitMoveCompleteMultiplier::WaitMoveCompleteMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "wait move complete")
{
}

float WaitMoveCompleteMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    PullStrategy const* strategy = PullStrategy::Get(botAI);
    if (strategy && strategy->HasPullStarted())
        return 1.0f;

    WaitMoveComplete* wmc = AI_VALUE(WaitMoveComplete*, "wait move complete");

    if (!wmc->IsStarted())
        return 1.0f;

    std::string const actionName = action->getName();
    if (actionName == "move complete" ||
        actionName == "flee chat shortcut")
        return 1.0f;

    return 0;
}
