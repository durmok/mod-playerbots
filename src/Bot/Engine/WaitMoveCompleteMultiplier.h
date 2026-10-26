/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WAITMOVECOMPLETEMULTIPLIER_H
#define PLAYERBOTS_WAITMOVECOMPLETEMULTIPLIER_H

#include "Multiplier.h"

class Action;
class PlayerbotAI;

class WaitMoveCompleteMultiplier : public Multiplier
{
public:
    WaitMoveCompleteMultiplier(PlayerbotAI* botAI);

    float GetValue(Action* action) override;
};

#endif
