/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BattleGroundTactics.h"
#include "Chat/Chat.h"
// #include "GuildTaskMgr.h"
#include "PerfMonitor.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "Playerbots.h"


// using namespace MaNGOS::ChatCommands;

class playerbots_commandscript : public CommandScript
{
public:
    playerbots_commandscript() : CommandScript("playerbots_commandscript") {}

    std::vector<ChatCommand> GetCommands() const override
    {
        static ChatCommand playerbotsDebugCommandTable[] = {
            {"bg", SEC_GAMEMASTER, true, &ChatHandler::HandleDebugBGCommand, "", nullptr},
            { nullptr, 0, false, nullptr, "", nullptr }
        };

        static ChatCommand playerbotsAccountCommandTable[] = {
            {"setKey", SEC_PLAYER, false, &ChatHandler::HandleSetSecurityKeyCommand, "", nullptr},
            {"link", SEC_PLAYER, false, &ChatHandler::HandleLinkAccountCommand, "", nullptr},
            {"linkedAccounts", SEC_PLAYER, false, &ChatHandler::HandleViewLinkedAccountsCommand, "", nullptr},
            {"unlink", SEC_PLAYER, false, &ChatHandler::HandleUnlinkAccountCommand, "", nullptr},
            { nullptr, 0, false, nullptr, "", nullptr }
        };

        static ChatCommand playerbotsCommandTable[] = {
            {"bot", SEC_PLAYER, false, &ChatHandler::HandlePlayerbotCommand, "", nullptr},
            // {"gtask", SEC_GAMEMASTER, true, HandleGuildTaskCommand, "", nullptr},
            {"pmon", SEC_GAMEMASTER, true, &ChatHandler::HandlePerfMonCommand, "", nullptr},
            {"rndbot", SEC_GAMEMASTER, true, &ChatHandler::HandleRandomPlayerbotCommand, "", nullptr},
            {"debug", SEC_GAMEMASTER, true, nullptr, "", playerbotsDebugCommandTable},
            {"account", SEC_GAMEMASTER, true, nullptr, "", playerbotsAccountCommandTable},
            { nullptr, 0, false, nullptr, "", nullptr }
        };

        std::vector<ChatCommand> commandTable;
        commandTable.push_back({"playerbots", SEC_PLAYER, true, nullptr, "mod-playerbot commands", playerbotsCommandTable});

        return commandTable;
    }

};

bool ChatHandler::HandlePlayerbotCommand(char * args)
{
    auto lock = LockPlayerbots();
    return PlayerbotMgr::HandlePlayerbotMgrCommand(this, args);
}

bool ChatHandler::HandleRandomPlayerbotCommand(char * args)
{
    auto lock = LockPlayerbots();
    return RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(this, args);
}

/*
bool ChatHandler::HandleGuildTaskCommand(char * args)
{
    return GuildTaskMgr::HandleConsoleCommand(this, args);
}
*/

bool ChatHandler::HandlePerfMonCommand(char * args)
{
    if (!strcmp(args, "reset"))
    {
        sPerfMonitor.Reset();
        return true;
    }

    if (!strcmp(args, "tick"))
    {
        sPerfMonitor.PrintStats(true, false);
        sPerfMonitor.DumpJson(true);
        return true;
    }

    if (!strcmp(args, "stack"))
    {
        sPerfMonitor.PrintStats(false, true);
        sPerfMonitor.DumpJson(false);
        return true;
    }

    if (!strcmp(args, "toggle"))
    {
        sPlayerbotAIConfig.PerfMonEnabled = !sPlayerbotAIConfig.PerfMonEnabled;
        if (sPlayerbotAIConfig.PerfMonEnabled)
            LOG_INFO("playerbots", "Performance monitor enabled");
        else
            LOG_INFO("playerbots", "Performance monitor disabled");
        return true;
    }

    sPerfMonitor.PrintStats();
    sPerfMonitor.DumpJson(false);
    return true;
}

bool ChatHandler::HandleDebugBGCommand(char * args)
{
    auto lock = LockPlayerbots();
    return BGTactics::HandleConsoleCommand(this, args);
}

bool ChatHandler::HandleSetSecurityKeyCommand(char * args)
{
    if (!args || !*args)
    {
        PSendSysMessage("Usage: .playerbots account setKey <securityKey>");
        return false;
    }

    Player* player = GetSession()->GetPlayer();
    std::string key = args;

    auto lock = LockPlayerbots();

    PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
    if (mgr)
    {
        mgr->HandleSetSecurityKeyCommand(player, key);
        return true;
    }
    else
    {
        PSendSysMessage("PlayerbotMgr instance not found.");
        return false;
    }
}

bool ChatHandler::HandleLinkAccountCommand(char * args)
{
    if (!args || !*args)
        return false;

    char* accountName = strtok((char*)args, " ");
    char* key = strtok(nullptr, " ");

    if (!accountName || !key)
    {
        PSendSysMessage("Usage: .playerbots account link <accountName> <securityKey>");
        return false;
    }

    Player* player = GetSession()->GetPlayer();

    auto lock = LockPlayerbots();

    PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
    if (mgr)
    {
        mgr->HandleLinkAccountCommand(player, accountName, key);
        return true;
    }
    else
    {
        PSendSysMessage("PlayerbotMgr instance not found.");
        return false;
    }
}

bool ChatHandler::HandleViewLinkedAccountsCommand(char * args)
{
    Player* player = GetSession()->GetPlayer();

    auto lock = LockPlayerbots();

    PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
    if (mgr)
    {
        mgr->HandleViewLinkedAccountsCommand(player);
        return true;
    }
    else
    {
        PSendSysMessage("PlayerbotMgr instance not found.");
        return false;
    }
}

bool ChatHandler::HandleUnlinkAccountCommand(char * args)
{
    if (!args || !*args)
        return false;

    char* accountName = strtok((char*)args, " ");
    if (!accountName)
    {
        PSendSysMessage("Usage: .playerbots account unlink <accountName>");
        return false;
    }

    Player* player = GetSession()->GetPlayer();

    auto lock = LockPlayerbots();

    PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
    if (mgr)
    {
        mgr->HandleUnlinkAccountCommand(player, accountName);
        return true;
    }
    else
    {
        PSendSysMessage("PlayerbotMgr instance not found.");
        return false;
    }
}

void AddPlayerbotsCommandscripts() { new playerbots_commandscript(); }
