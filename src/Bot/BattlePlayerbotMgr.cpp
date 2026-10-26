/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BattlePlayerbotMgr.h"
#include "AiFactory.h"
#include "AiObjectContext.h"
#include "Battleground.h"
#include "BattleGroundMgr.h"
#include "DBCStores.h"
#include "GridNotifiers.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotFactory.h"
#include "QueryResult2.h"
#include "Random.h"
#include "RandomPlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include <ctime>
#include <unordered_map>


void BattlePlayerbotMgr::UpdateAIInternal(uint32 /*elapsed*/, bool /*minimal*/)
{
    if (time(nullptr) > (BgCheckTimer + 35))
    {
        RemoveInactiveBots();
        CheckBgQueue();
        ActivateBots();
        BgCheckTimer = time(nullptr);
    }
}

void BattlePlayerbotMgr::CheckBgQueue()
{
    LOG_DEBUG("playerbots", "Checking BG Queue...");

    // Initialize Battleground Data (do not clear here)

    for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
    {
        for (int queueType = BATTLEGROUND_QUEUE_AV; queueType < MAX_BATTLEGROUND_QUEUE_TYPES; ++queueType)
        {
            sRandomPlayerbotMgr.BattlegroundData[queueType][bracket] = BattlegroundInfo();
        }
    }

    // scan active battleground instances

    for (const BattleGround* bg: sBattlegroundMgr->GetActiveBattlegrounds())
    {
        BattleGroundQueueTypeId queueTypeId = BattlegroundMgr::BGQueueTypeId(bg->GetBgTypeID());
        BattleGroundBracketId bracketId = bg->GetBracketId();
        BattlegroundInfo& bgInfo = sRandomPlayerbotMgr.BattlegroundData[queueTypeId][bracketId];
        bgInfo.minLevel = bg->GetMinLevel();
        bgInfo.maxLevel = bg->GetMaxLevel();

        // count players in the instance

        for (auto itr: bg->GetPlayers())
        {
            Player* player = ObjectAccessor::FindConnectedPlayer(itr.first);
            if (!player)
                continue;

            bool isBot = sPlayerbotsMgr.GetPlayerbotAI(player) != nullptr;

            TeamId teamId = player->GetTeamId();
            if (isBot)
            {
                if (teamId == TEAM_ALLIANCE)
                    bgInfo.bgAllianceBotCount++;
                else
                    bgInfo.bgHordeBotCount++;
            }
            else
            {
                if (teamId == TEAM_ALLIANCE)
                    bgInfo.bgAlliancePlayerCount++;
                else
                    bgInfo.bgHordePlayerCount++;
            }

        }

        // count instance

        std::vector<uint32>* instanceIds = &bgInfo.bgInstances;

        if (std::find(instanceIds->begin(), instanceIds->end(), bg->GetInstanceID()) == instanceIds->end())
            instanceIds->push_back(bg->GetClientInstanceID());

        bgInfo.bgInstanceCount = instanceIds->size();
    }

    // scan battleground queue

    for (int i = BATTLEGROUND_QUEUE_AV; i < MAX_BATTLEGROUND_QUEUE_TYPES; ++i)
    {
        BattleGroundQueueTypeId queueTypeId = BattleGroundQueueTypeId(i);
        BattleGroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
        if (bgTypeId == BATTLEGROUND_TYPE_NONE)
            continue;
        BattleGround* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg)
            continue;
        uint32 mapId = bg->GetMapId();

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
        for (auto const& itr : bgQueue.m_queuedPlayers)
        {
            GroupQueueInfo* ginfo = itr.second.groupInfo;
            for (auto itr: ginfo->players)
            {
                ObjectGuid playerGUID = itr.first;
                Player* player = ObjectAccessor::FindPlayer(playerGUID);
                if (!player)
                    continue;

                // // Skip player if not currently in a queue
                // if (!player->InBattlegroundQueue())
                //     continue;

                BattleGroundBracketId bracketId = player->GetBattleGroundBracketIdFromLevel(bgTypeId);
                if (bracketId == BG_BRACKET_ID_NONE)
                    continue;
                uint32 const minLevel = bg->GetMinLevel() + 10 * bracketId;
                ASSERT(minLevel <= PLAYER_MAX_LEVEL);
                uint32 const maxLevel = std::min<uint32>(minLevel + 9, PLAYER_MAX_LEVEL);

                BattlegroundInfo& bgInfo = sRandomPlayerbotMgr.BattlegroundData[queueTypeId][bracketId];
                bgInfo.minLevel = minLevel;
                bgInfo.maxLevel = maxLevel;
                bool isBot = sPlayerbotsMgr.GetPlayerbotAI(player) != nullptr;

                TeamId teamId = player->GetTeamId();
                if (isBot)
                {
                    if (teamId == TEAM_ALLIANCE)
                        bgInfo.bgAllianceBotCount++;
                    else
                        bgInfo.bgHordeBotCount++;
                }
                else
                {
                    if (teamId == TEAM_ALLIANCE)
                        bgInfo.bgAlliancePlayerCount++;
                    else
                        bgInfo.bgHordePlayerCount++;
                }

                // check if queue is active

                if (!isBot && !player->InBattleGround() && !player->InBattleground())
                {
                    bgInfo.activeBgQueue = 1;
                }
            }
        }
    }

    // sRandomPlayerbotMgr.LogBattlegroundInfo();

    uint32 activeAlliance = 0;
    uint32 activeHorde = 0;
    for (auto itr: activeBots)
    {
        if (itr.second->alliance)
            activeAlliance++;
        else
            activeHorde++;
    }
    if (activeAlliance + activeHorde != 0)
        LOG_INFO("playerbots", "BattleBot Activity (A:{} H:{})", activeAlliance, activeHorde);
}

uint32 GetBattleGroundMinTeamSize(uint32 teamSize)
{
    if (teamSize < 40)
        return teamSize * 0.8;
    else
        return teamSize * 0.5;
}

void BattlePlayerbotMgr::ActivateBots()
{
    IdleBotFinder allianceFinder(&allianceBots, true);
    IdleBotFinder hordeFinder(&hordeBots, false);

    for (int i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        BattleGroundBracketId bracketId = BattleGroundBracketId(i);
        for (int j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BattleGroundQueueTypeId queueTypeId = BattleGroundQueueTypeId(j);
            BattlegroundInfo& bgInfo = sRandomPlayerbotMgr.BattlegroundData[queueTypeId][bracketId];
            BracketInfo& bracket = brackets[queueTypeId][bracketId];
            BattleGroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
            BattleGround* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            if (!bg)
                continue;
            uint32 BracketSize = bg->GetMaxPlayersPerTeam() * 2;
            uint32 TeamSize = bg->GetMaxPlayersPerTeam();

            if (bgInfo.bgAlliancePlayerCount + bgInfo.bgHordePlayerCount != 0)
            {
                uint32 TeamSizeMin;
                if (TeamSize < 40)
                    TeamSizeMin = TeamSize * 0.8;
                else
                    TeamSizeMin = TeamSize * 0.5;

                uint32 factionNeed = TeamSizeMin * bgInfo.activeBgQueue + TeamSize * bgInfo.bgInstanceCount;

                uint32 allianceExist = bracket.activeAllianceBotCount + bgInfo.bgAlliancePlayerCount;
                if (factionNeed > allianceExist)
                    ActivateFactionBots(allianceFinder, bgInfo, bracket, queueTypeId, bracketId, false, factionNeed - allianceExist);
            
                uint32 hordeExist = bracket.activeHordeBotCount + bgInfo.bgHordePlayerCount;
                if (factionNeed > hordeExist)
                    ActivateFactionBots(hordeFinder, bgInfo, bracket, queueTypeId, bracketId, false, factionNeed - hordeExist);
            }
        }
    }
}

void BattlePlayerbotMgr::ActivateFactionBots(IdleBotFinder& finder, BattlegroundInfo& bgInfo, BracketInfo& bracket, BattleGroundQueueTypeId queueTypeId,
        BattleGroundBracketId bracketId, bool arena, uint32 addBotCount)
{
    for (uint32 i = 0; i < addBotCount; i++)
    {
        std::optional<uint32> index = finder.FindNextIndex();
        if (!index.has_value())
            break;
        BattleBotState* state = &(*finder.GetBots())[index.value()];
        state->inactiveDeadline = time(nullptr) + BATTLE_BOT_LOGIN_INACTIVE_WAIT_TIMEOUT;
        state->queueTypeId = queueTypeId;
        state->bracketId = bracketId;

        if (arena)
        {
            state->arena = true;
        }
        else
        {
            state->arena = false;
            if (finder.IsAlliance())
            {
                state->alliance = true;
                bracket.activeAllianceBotCount++;
            }
            else
            {
                state->alliance = false;
                bracket.activeHordeBotCount++;
            }
        }

        state->level = urand(bgInfo.minLevel, bgInfo.maxLevel);
        state->active = true;
        activeBots.insert_or_assign(state->id, state);

        LOG_INFO("playerbots", "BattleBot {} became active {}:{}", state->id, state->alliance ? "A" : "H", state->level);
        AddPlayerBot(ObjectGuid(HighGuid::HIGHGUID_PLAYER, state->id), 0);
    }
}

void BattlePlayerbotMgr::RemoveInactiveBots()
{
    std::vector<uint32> removeIds;
    for (auto itr = activeBots.begin(); itr != activeBots.end(); itr++)
    {
        BattleBotState *state = itr->second;
        ObjectGuid guid = ObjectGuid(HighGuid::HIGHGUID_PLAYER, state->id);
        Player* bot = ObjectAccessor::FindConnectedPlayer(guid);
        if (bot)
        {
            if (!bot->IsBeingTeleported() && !bot->InBattlegroundQueue() && !bot->InBattleground())
            {
                if (state->inactiveDeadline == 0)
                    state->inactiveDeadline = time(nullptr) + BATTLE_BOT_INACTIVE_WAIT_TIMEOUT;
                else if (time(nullptr) > state->inactiveDeadline)
                {
                    LOG_INFO("playerbots", "BattleBot {} became inactive {}:{}", state->id, state->alliance ? "A" : "H", state->level);
                    LogoutPlayerBot(guid);
                    removeIds.push_back(itr->first);
                }
            }
            else if (state->inactiveDeadline != 0)
                state->inactiveDeadline = 0;
        }
        else
            removeIds.push_back(itr->first);
    }

    for (uint32 i = 0; i < removeIds.size(); i++)
    {
        RemoveInactiveBot(removeIds[i]);
    }
}

void BattlePlayerbotMgr::RemoveInactiveBot(uint32 id)
{
    BattleBotState* state = activeBots[id];
    if (!state)
        return;
    activeBots.erase(id);
    BracketInfo& bracket = brackets[state->queueTypeId][state->bracketId];
    if (state->arena)
        bracket.activeArenaBotCount--;
    else
    {
        if (state->alliance)
            bracket.activeAllianceBotCount--;
        else
            bracket.activeHordeBotCount--;
    }
    state->active = false;
}

TeamId TeamIdForRace(uint8 race)
{
    if (ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race))
    {
        switch (rEntry->TeamID)
        {
            case 1:
                return TEAM_HORDE;
            case 7:
                return TEAM_ALLIANCE;
        }
        LOG_ERROR("entities.player", "Race ({}) has wrong teamid ({}) in DBC: wrong DBC files?", uint32(race), rEntry->TeamID);
    }
    else
        LOG_ERROR("entities.player", "Race ({}) not found in DBC: wrong DBC files?", uint32(race));

    return TEAM_ALLIANCE;
}

void BattlePlayerbotMgr::Init()
{
    BgCheckTimer = time(nullptr);

    // init fixed brackets map structure
    for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
    {
        for (int queueType = BATTLEGROUND_QUEUE_AV; queueType < MAX_BATTLEGROUND_QUEUE_TYPES; ++queueType)
        {
            brackets[queueType][bracket] = BracketInfo();
        }
    }

    // load all battle bot characters
    struct CharacterInfo
    {
        uint32 guid;
        uint8 rClass;
        uint8 rRace;
        uint32 accountId;
    };
    for (uint32 accountId : sRandomPlayerbotMgr.GetBattleTypeAccounts())
    {
        CharacterDatabasePreparedStatement* stmt =
            CharacterDatabase2.GetPreparedStatement(CHAR_SEL_ACCOUNT_INFO_CHARS);
        stmt->SetData(0, accountId);
        PreparedQueryResult result = CharacterDatabase2.Query(stmt);
        if (!result)
            continue;

        do
        {
            Field2* fields = result->Fetch();
            CharacterInfo info;
            info.guid = fields[0].Get<uint32>();
            info.rRace = fields[3].Get<uint8>();
            info.rClass = fields[4].Get<uint8>();
            info.accountId = accountId;


            TeamId team = TeamIdForRace(info.rRace);
            BattleBotState state(info.guid, 0, BATTLEGROUND_QUEUE_NONE, BG_BRACKET_ID_FIRST, false, 0, false);
            if (team == TEAM_ALLIANCE)
                allianceBots.push_back(state);
            else if (team == TEAM_HORDE)
                hordeBots.push_back(state);
        } while (result->NextRow());
    }
}

IdleBotFinder::IdleBotFinder(std::vector<BattleBotState>* bots, bool alliance): bots(bots), alliance(alliance)
{
    position = 0;
    randomOff = urand(0, bots->size() != 0 ? bots->size() - 1 : 0);
}

std::optional<uint32> IdleBotFinder::FindNextIndex()
{
    while (position < bots->size())
    {
        uint32 index = (position + randomOff) % bots->size();
        if (!(*bots)[index].active)
        {
            position++;
            return index;
        }
        position++;
    }
    return std::nullopt;
}

void BattlePlayerbotMgr::OnBotLoginInternal(Player* const bot)
{
    uint32 id = bot->GetObjectGuid().GetCounter();
    auto activeItr = activeBots.find(id);
    if (activeItr == activeBots.end())
    {
        LOG_ERROR("playerbots", "BattleBot {} logged in but not found in active list", id);
        LogoutPlayerBot(bot->GetObjectGuid());
        RemoveInactiveBot(id);
        return;
    }

    PlayerbotAI* botAI = sPlayerbotsMgr.GetPlayerbotAI(bot);
    if (!botAI)
    {
        LOG_ERROR("playerbots", "BattleBot {} logged in but not found in PlayerbotsMgr", id);
        LogoutPlayerBot(bot->GetObjectGuid());
        RemoveInactiveBot(id);
        return;
    }

    BattleBotState* state = activeItr->second;
    PlayerbotFactory factory(bot, state->level);
    factory.Randomize(false);
    botAI->GetAiObjectContext()->GetValue<uint32>("bg join queue type id")->Set(state->queueTypeId);
    botAI->GetAiObjectContext()->GetValue<uint32>("bg join bracket id")->Set(state->bracketId);
}

void BattlePlayerbotMgr::OnPlayerLogin(Player* /*player*/)
{

}

void BattlePlayerbotMgr::OnPlayerLogout(Player* /*player*/)
{

}
