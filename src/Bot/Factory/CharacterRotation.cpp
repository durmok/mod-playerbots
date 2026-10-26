/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CharacterRotation.h"
#include "Playerbots.h"
#include "RaceMgr.h"

CharacterRotation::CharacterRotation(bool alliance)
{
    uint8 gender = 0;
    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        if ((alliance && cls == CLASS_SHAMAN) || (!alliance && cls == CLASS_PALADIN)) {
            continue;
        }

        // skip nonexistent classes
        if (!((1 << (cls - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(cls))
            continue;

        for (uint8 race = RACE_HUMAN; race < MAX_RACES; ++race)
        {
            if (alliance == IsAlliance(race))
            {
                if (PlayerInfo const* info = sObjectMgr.GetPlayerInfo(race, cls))
                {
                    classes[cls].races.push_back({race, gender});
                    gender = gender == 0 ? 1 : 0;
                }
            }
        }
    }

    // mixure start race of all classes 
    std::unordered_set<uint8> raceSet;
    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        RaceRotation& clsR = classes[cls];
        if (clsR.races.empty())
            continue;
        bool newRace = false;
        for (uint8 i = 0; i < clsR.races.size(); i++)
        {
            RaceGenderRotation raceR = clsR.races[i];
            if (raceSet.insert(raceR.race).second)
            {
                clsR.index = i;
                newRace = true;
                break;
            }

        }
        if (!newRace)
        {
            raceSet.clear();
            raceSet.insert(clsR.races[0].race);
        }
    }
}

bool CharacterRotation::GetNextRaceGender(uint8 cls, uint8& race, uint8& gender)
{
    RaceRotation& clsR = classes[cls];

    if (clsR.races.empty())
        return false;

    RaceGenderRotation& raceR = clsR.races[clsR.index];
    clsR.index++;

    race = raceR.race;
    gender = raceR.gender;

    if (clsR.index >= clsR.races.size())
        clsR.index = 0;
    raceR.gender = gender == 0 ? 1 : 0;

    return true;
}

