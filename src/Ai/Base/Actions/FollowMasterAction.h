/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOLLOWGRACEFULLYACTIONS_H
#define PLAYERBOTS_FOLLOWGRACEFULLYACTIONS_H

#include "MovementActions.h"

class PlayerbotAI;

class FollowMasterAction : public MovementAction
{
public:
    FollowMasterAction(PlayerbotAI* botAI, std::string const name = "follow") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
    void SetUseless() override;

private:
    time_t nextChangePositionTime = 0;
    float lastDist = 0;
    float lastAngle = 0;
};

#endif
