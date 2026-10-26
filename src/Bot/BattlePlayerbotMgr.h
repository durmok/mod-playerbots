/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BATTLEPLAYERBOTMGR_H
#define PLAYERBOTS_BATTLEPLAYERBOTMGR_H

#include "Time/GameTime.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"
#include <unordered_map>

#define BATTLE_BOT_LOGIN_INACTIVE_WAIT_TIMEOUT 180
#define BATTLE_BOT_INACTIVE_WAIT_TIMEOUT 60

struct BracketInfo
{
    uint32 activeArenaBotCount = 0;
    uint32 activeAllianceBotCount = 0;
    uint32 activeHordeBotCount = 0;
};

struct BattleBotState
{
    uint32 id;
    time_t inactiveDeadline = 0;
    BattleGroundQueueTypeId queueTypeId = BATTLEGROUND_QUEUE_NONE;
    BattleGroundBracketId bracketId = BG_BRACKET_ID_FIRST;
    bool arena = false;
    bool alliance = false;
    uint32 level = 0;
    bool active = false;
};

class IdleBotFinder {
public:
    IdleBotFinder(std::vector<BattleBotState>* bots, bool alliance);

    std::optional<uint32> FindNextIndex();
    std::vector<BattleBotState>* GetBots() { return bots; }
    bool IsAlliance() { return alliance; }
private:
    std::vector<BattleBotState>* bots;
    bool alliance;
    uint32 position;
    uint32 randomOff;
};

class BattlePlayerbotMgr : public PlayerbotHolder
{
public:
    static BattlePlayerbotMgr& instance()
    {
        static BattlePlayerbotMgr instance;

        return instance;
    }

    void Init();
    void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;
    void CheckBgQueue();

    void OnPlayerLogout(Player* player);
    void OnPlayerLogin(Player* player);

protected:
    void OnBotLoginInternal(Player* const bot) override;

private:
    BattlePlayerbotMgr() : PlayerbotHolder()
    {
        this->BgCheckTimer = 0;
    }

    ~BattlePlayerbotMgr() = default;

    BattlePlayerbotMgr(BattlePlayerbotMgr const&) = delete;
    BattlePlayerbotMgr& operator=(BattlePlayerbotMgr const&) = delete;

    BattlePlayerbotMgr(BattlePlayerbotMgr&&) = delete;
    BattlePlayerbotMgr& operator=(BattlePlayerbotMgr&&) = delete;

    time_t BgCheckTimer;

    std::vector<BattleBotState> allianceBots;
    std::vector<BattleBotState> hordeBots;
    std::unordered_map<uint32, BattleBotState*> activeBots;
    std::unordered_map<uint32, std::unordered_map<uint32, BracketInfo>> brackets;

    void ActivateBots();
    void ActivateFactionBots(IdleBotFinder& finder, BattlegroundInfo& bgInfo, BracketInfo& bracket, BattleGroundQueueTypeId queueTypeId,
        BattleGroundBracketId bracketId, bool arena, uint32 addBotCount);
    void RemoveInactiveBots();
    void RemoveInactiveBot(uint32 id);

    static inline uint32 NowSeconds() { return static_cast<uint32>(GameTime::GetGameTime().count()); }
};

#define sBattlePlayerbotMgr BattlePlayerbotMgr::instance()

#endif