/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WAITMOVECOMPLETEVALUE_H
#define PLAYERBOTS_WAITMOVECOMPLETEVALUE_H

#include "NamedObjectContext.h"
#include "TravelMgr.h"
#include "Value.h"

class PlayerbotAI;

class WaitMoveComplete
{
public:
    void Start(float x, float y, float z);

    void Stop() { startTime = 0; }

    bool IsStarted() { return startTime != 0; }

    time_t GetStartTime() { return startTime; }

    WorldPosition GetDestPosition() { return destPos; }

private:
    time_t startTime;
    WorldPosition destPos;
};

class WaitMoveCompleteValue : public ManualSetValue<WaitMoveComplete*>
{
public:
    WaitMoveCompleteValue(PlayerbotAI* botAI, std::string const name = "wait move complete");
    virtual ~WaitMoveCompleteValue();
};

#endif