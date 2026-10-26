/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WaitMoveCompleteValue.h"

void WaitMoveComplete::Start(float x, float y, float z)
{
    destPos.setX(x);
    destPos.setY(y);
    destPos.setZ(z);
    startTime = time(nullptr);
}

WaitMoveCompleteValue::WaitMoveCompleteValue(PlayerbotAI* botAI, std::string const name)
        : ManualSetValue<WaitMoveComplete*>(botAI, nullptr, name)
{
    value = new WaitMoveComplete();
}

WaitMoveCompleteValue::~WaitMoveCompleteValue() { delete value; }