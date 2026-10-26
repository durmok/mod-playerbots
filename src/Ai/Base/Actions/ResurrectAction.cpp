/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ResurrectAction.h"

#include "Corpse.h"
#include "Event.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "Pet.h"
#include "PlayerbotAI.h"

// class ResurrectVisualEvent: public BasicEvent
// {
// public:
//     ResurrectVisualEvent(Player* bot) : _bot(bot) { }

//     bool Execute(uint64 /*time*/, uint32 /*diff*/)
//     {
//         _bot->CastSpell(_bot, SPELL_RESURRECTION_VISUAL, true);
//         return true;
//     }

// private:
//     Player* _bot;
// };

bool ResurrectAction::Execute(Event /*event*/)
{
    if (!bot->isDead())
    {
        botAI->TellMaster("I am not dead yet");
        return false;
    }

    bool isInCombat = bot->IsInCombat();
    if (!isInCombat)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* player = itr->GetSource();
                if (!player || player == bot)
                    continue;
                if (player->IsInCombat())
                {
                    isInCombat = true;
                    break;
                }
            }
        }
    }
    if (isInCombat)
    {
        botAI->TellMaster("Cannot resurrect in combat");
        return false;
    }

    Player* master = GetMaster();
    if (!master)
        return false;

    Position pos = master->GetPosition();
    if (master->HasPlayerFlag(PLAYER_FLAGS_GHOST))
    {
        if (Corpse* corpse = master->GetCorpse())
            pos = corpse->GetPosition();
        else
            return false;
    }

    // if (!bot->IsWithinDistInMap(master, 40) || !bot->IsWithinLOSInMap(master))
    // {
    //     botAI->TellMaster("Require within your line of sight to revive me");
    //     return false;
    // }

    // bot->CastSpell(bot, SPELL_RESURRECTION_VISUAL, true);
    bot->CastSpell(bot, SPELL_RESURRECTION_VISUAL, true);
    bot->ResurrectPlayer(0.35f, false);
    bot->SpawnCorpseBones();
    bot->DurabilityRepairAll(false, 1.0f, false);

    bot->TeleportTo({ master->GetMapId(), pos});
    bot->AttackStop();
    if (bot->GetPet())
    {
        bot->GetPet()->NearTeleportTo(pos);
        bot->GetPet()->AttackStop();
    }
    if (bot->GetGuardianPet())
    {
        bot->GetGuardianPet()->NearTeleportTo(pos);
        bot->GetGuardianPet()->AttackStop();
    }

    // bot->m_Events.AddEvent(
    //     new ResurrectVisualEvent(bot),
    //     bot->m_Events.CalculateTime(100)
    // );

    botAI->TellMaster("I'm resurrected");
    
    return true;
}

