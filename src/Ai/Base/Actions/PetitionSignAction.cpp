/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PetitionSignAction.h"
// #include "ArenaTeam.h"
#include "Event.h"
#include "Playerbots.h"

bool PetitionSignAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);
    ObjectGuid petitionGuid;
    ObjectGuid inviter;
    uint8 unk = 0;
    bool isArena = false;
    p >> petitionGuid >> inviter;
    uint32 type = 9;

    /*
    QueryResult2 result =
        CharacterDatabase2.Query("SELECT `type` FROM `petition` WHERE `petition_guid` = {}", petitionGuid.GetCounter());
    if (!result)
    {
        return false;
    }

    Field2* fields = result->Fetch();
    uint32 type = fields[0].Get<uint32>();
    */

    bool accept = true;

    if (type != 9)
    {
        /*
        isArena = true;
        uint8 slot = ArenaTeam::GetSlotByType(ArenaType(type));
        if (bot->GetArenaTeamId(slot))
        {
            // player is already in an arena team
            botAI->TellError("Sorry, I am already in such team");
            accept = false;
        }
        */
    }
    else
    {
        if (bot->GetGuildId())
        {
            botAI->TellError("Sorry, I am in a guild already");
            accept = false;
        }

        if (bot->GetGuildIdInvited())
        {
            botAI->TellError("Sorry, I am invited to a guild already");
            accept = false;
        }

        // check for same acc id
        /*if (QueryResult* result = CharacterDatabase.Query("SELECT playerguid FROM petition_sign WHERE player_account =
        {} AND petitionguid = {}'", bot->GetSession()->GetAccountId(), petitionGuid.GetCounter()))
        {
            botAI->TellError("Sorry, I already signed this pettition");
            accept = false;
        }
        */
    }

    Player* _inviter = ObjectAccessor::FindPlayer(inviter);
    if (!_inviter)
        return false;

    if (_inviter == bot)
        return false;

    if (!accept || !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, _inviter, true))
    {
        WorldPackets::Petition::PetitionDecline data;
        data.itemGuid = petitionGuid;
        bot->GetSession()->HandlePetitionDeclineOpcode(data);
        LOG_INFO("playerbots", "Bot {} <{}> declines {} invite", bot->GetObjectGuid().ToString().c_str(),
                 bot->GetName(), isArena ? "Arena" : "Guild");
        return false;
    }

    if (accept)
    {
        WorldPackets::Petition::PetitionSign data;
        data.itemGuid = petitionGuid;
        bot->GetSession()->HandlePetitionSignOpcode(data);
        bot->Say("Thanks for the invite!", LANG_UNIVERSAL);
        LOG_INFO("playerbots", "Bot {} <{}> accepts {} invite", bot->GetObjectGuid().ToString().c_str(),
                 bot->GetName(), isArena ? "Arena" : "Guild");
        return true;
    }

    return false;
}
