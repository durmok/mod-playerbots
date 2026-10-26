/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CheckMailAction.h"
#include "Event.h"
// #include "GuildTaskMgr.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Mail.h"

bool CheckMailAction::Execute(Event /*event*/)
{
    // WorldPacket p;
    bot->GetSession()->HandleQueryNextMailTime(NullClientPacket(MSG_QUERY_NEXT_MAIL_TIME));

    MasterPlayer* mplayer = bot->GetSession()->GetMasterPlayer();
    if (!mplayer)
        return false;

    if (!CharacterDatabase.BeginTransaction())
        return false;

    std::vector<uint32> ids;
    for (PlayerMails::const_iterator i = mplayer->GetMailBegin(); i != mplayer->GetMailBegin(); ++i)
    {
        Mail* mail = *i;
        if (!mail || mail->state == MAIL_STATE_DELETED)
            continue;

        Player* owner = ObjectAccessor::FindConnectedPlayer(ObjectGuid(HighGuid::HIGHGUID_PLAYER, mail->sender));
        if (!owner)
            continue;

        uint32 account = owner->GetSession()->GetAccountId();
        if (PlayerbotAIConfig::Instance().IsInRandomAccountList(account))
            continue;

        ProcessMail(mplayer, mail, owner);
        ids.push_back(mail->messageID);
        mail->state = MAIL_STATE_DELETED;
    }

    for (uint32 id : ids)
    {
        // bot->SendMailResult(id, MAIL_DELETED, MAIL_OK);

        // CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_BY_ID);
        // stmt->SetData(0, id);
        // trans->Append(stmt);
        CharacterDatabase.PExecute("DELETE FROM mail WHERE id = %u", id);

        // stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_ITEM_BY_ID);
        // stmt->SetData(0, id);
        // trans->Append(stmt);
        CharacterDatabase.PExecute("DELETE FROM mail_items WHERE mail_id = %u", id);

        mplayer->RemoveMail(id);
    }

    CharacterDatabase.CommitTransaction();

    return true;
}

bool CheckMailAction::isUseful()
{
    MasterPlayer* mplayer = bot->GetSession()->GetMasterPlayer();
    if (!mplayer)
        return false;

    if (botAI->GetMaster() || !mplayer->GetMailSize() || bot->InBattleground())
        return false;

    return true;
}

void CheckMailAction::ProcessMail(MasterPlayer* mplayer, Mail* mail, Player* owner)
{
    if (mail->items.empty())
    {
        return;
    }

    if (mail->subject.find("Item(s) you asked for") != std::string::npos)
        return;

    for (MailItemInfoVec::iterator i = mail->items.begin(); i != mail->items.end(); ++i)
    {
        Item* item = mplayer->GetMItem(i->itemGuid);
        if (!item)
            continue;

        // if (!GuildTaskMgr::instance().CheckItemTask(i->item_template, item->GetCount(), owner, bot, true))
        if (true)
        {
            std::ostringstream body;
            body << "Hello, " << owner->GetName() << ",\n";
            body << "\n";
            body << "Here are the item(s) you've sent me by mistake";
            body << "\n";
            body << "Thanks,\n";
            body << bot->GetName() << "\n";

            MailDraft draft("Item(s) you've sent me", body.str());
            draft.AddItem(item);
            mplayer->RemoveMItem(i->itemGuid);
            draft.SendMailTo(MailReceiver(owner), MailSender(bot));
            return;
        }

        mplayer->RemoveMItem(i->itemGuid);
        item->DestroyForPlayer(bot);
    }
}
