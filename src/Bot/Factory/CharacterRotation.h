/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHARACTERROTATION_H
#define PLAYERBOTS_CHARACTERROTATION_H

#include "Playerbots.h"

struct RaceGenderRotation
{
    uint8 race;
    uint8 gender;
};

struct RaceRotation
{
    std::vector<RaceGenderRotation> races;
    uint8 index = 0;
};

class CharacterRotation
{
    public:
        CharacterRotation(bool alliance);

        ~CharacterRotation() {}

        bool GetNextRaceGender(uint8 cls, uint8& race, uint8& gender);

    private:
        RaceRotation classes[MAX_CLASSES];
};

#endif