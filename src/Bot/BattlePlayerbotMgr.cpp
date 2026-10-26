/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BattlePlayerbotMgr.h"
#include "AiFactory.h"
#include "AiObjectContext.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "DBCStores.h"
#include "GridNotifiers.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotFactory.h"
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

    for (const Battleground* bg: sBattlegroundMgr->GetActiveBattlegrounds())
    {
        ArenaType arenaType = ArenaType(bg->GetArenaType());
        BattlegroundQueueTypeId queueTypeId = BattlegroundMgr::BGQueueTypeId(bg->GetBgTypeID(), arenaType);
        BattlegroundBracketId bracketId = bg->GetBracketId();
        BattlegroundInfo& bgInfo = sRandomPlayerbotMgr.BattlegroundData[queueTypeId][bracketId];
        bgInfo.minLevel = bg->GetMinLevel();
        bgInfo.maxLevel = bg->GetMaxLevel();
        bool isArena = arenaType != ARENA_TYPE_NONE;
        bool isRated = bg->isRated();

        // count players in the instance

        for (auto itr: bg->GetPlayers())
        {
            Player* player = itr.second;
            bool isBot = sPlayerbotsMgr.GetPlayerbotAI(player) != nullptr;

            if (isArena)
            {
                if (isBot)
                {
                    if (isRated)
                        bgInfo.ratedArenaBotCount++;
                    else
                        bgInfo.skirmishArenaBotCount++;
                }
                else
                {
                    if (isRated)
                        bgInfo.ratedArenaPlayerCount++;
                    else
                        bgInfo.skirmishArenaPlayerCount++;
                }
            }
            else 
            {
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

        }

        // count instance

        std::vector<uint32>* instanceIds;
        if (isArena)
        {
            if (isRated)
                instanceIds = &bgInfo.ratedArenaInstances;
            else
                instanceIds = &bgInfo.skirmishArenaInstances;
        }
        else
            instanceIds = &bgInfo.bgInstances;

        if (std::find(instanceIds->begin(), instanceIds->end(), bg->GetInstanceID()) == instanceIds->end())
            instanceIds->push_back(bg->GetInstanceID());

        if (isArena)
        {
            if (isRated)
                bgInfo.ratedArenaInstanceCount = instanceIds->size();
            else
                bgInfo.skirmishArenaInstanceCount = instanceIds->size();
        }
        else
            bgInfo.bgInstanceCount = instanceIds->size();
    }

    // scan battleground queue

    for (int i = BATTLEGROUND_QUEUE_AV; i < MAX_BATTLEGROUND_QUEUE_TYPES; ++i)
    {
        BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(i);
        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
        if (bgTypeId == BATTLEGROUND_TYPE_NONE)
            continue;
        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg)
            continue;
        uint32 mapId = bg->GetMapId();

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
        for (auto const& itr : bgQueue.m_QueuedPlayers)
        {
            GroupQueueInfo* ginfo = itr.second;
            for (auto playerGUID: ginfo->Players)
            {
                Player* player = ObjectAccessor::FindPlayer(playerGUID);
                if (!player)
                    continue;

                // // Skip player if not currently in a queue
                // if (!player->InBattlegroundQueue())
                //     continue;

                PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, player->GetLevel());
                if (!pvpDiff)
                    continue;

                BattlegroundBracketId bracketId = pvpDiff->GetBracketId();
                BattlegroundInfo& bgInfo = sRandomPlayerbotMgr.BattlegroundData[queueTypeId][bracketId];
                bgInfo.minLevel = pvpDiff->minLevel;
                bgInfo.maxLevel = pvpDiff->maxLevel;
                bool isBot = sPlayerbotsMgr.GetPlayerbotAI(player) != nullptr;
                bool isArena = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId)) != ARENA_TYPE_NONE;
                // bool isRated = ginfo->IsRated || bgQueue.IsPlayerInvitedToRatedArena(player->GetGUID()) || (player->InArena() && bg->isRated());
                bool isRated = ginfo->IsRated;

                if (isArena)
                {
                    if (isBot)
                    {
                        if (isRated)
                            bgInfo.ratedArenaBotCount++;
                        else
                            bgInfo.skirmishArenaBotCount++;
                    }
                    else
                    {
                        if (isRated)
                            bgInfo.ratedArenaPlayerCount++;
                        else
                            bgInfo.skirmishArenaPlayerCount++;
                    }
                }
                else
                {
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

                // check if queue is active

                if (!isBot && !player->IsInvitedForBattlegroundInstance() && !player->InBattleground())
                {
                    if (isArena)
                    {
                        if (isRated)
                            bgInfo.activeRatedArenaQueue = 1;
                        else
                            bgInfo.activeSkirmishArenaQueue = 1;
                    }
                    else
                    {
                        bgInfo.activeBgQueue = 1;
                    }
                }
            }
        }
    }

    sRandomPlayerbotMgr.LogBattlegroundInfo();

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
        BattlegroundBracketId bracketId = BattlegroundBracketId(i);
        for (int j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(j);
            BattlegroundInfo& bgInfo = sRandomPlayerbotMgr.BattlegroundData[queueTypeId][bracketId];
            BracketInfo& bracket = brackets[queueTypeId][bracketId];
            BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            if (!bg)
                continue;
            uint32 BracketSize = bg->GetMaxPlayersPerTeam() * 2;
            uint32 TeamSize = bg->GetMaxPlayersPerTeam();

            ArenaType arenaType = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId));
            if (arenaType != ARENA_TYPE_NONE)
            {
                BracketSize = (uint32)(arenaType * 2);
                TeamSize = (uint32)arenaType;

                if (bgInfo.skirmishArenaPlayerCount != 0)
                {
                    uint32 maxRequiredSkirmishBots = BracketSize * (bgInfo.activeSkirmishArenaQueue + bgInfo.skirmishArenaInstanceCount);
                    if (maxRequiredSkirmishBots != 0)
                        maxRequiredSkirmishBots = maxRequiredSkirmishBots + TeamSize;

                    uint32 exist = bracket.activeArenaBotCount + bgInfo.skirmishArenaPlayerCount;
                    if (maxRequiredSkirmishBots > exist)
                    {
                        for (uint32 i = 0; i < maxRequiredSkirmishBots - exist; i++)
                        {
                            if (urand(0, 1))
                                ActivateFactionBots(allianceFinder, bgInfo, bracket, queueTypeId, bracketId, true, 1);
                            else
                                ActivateFactionBots(hordeFinder, bgInfo, bracket, queueTypeId, bracketId, true, 1);
                        }
                    }
                }
                else if (bgInfo.ratedArenaPlayerCount != 0)
                {
                    // not implemented yet
                }
            }
            else if (bgInfo.bgAlliancePlayerCount + bgInfo.bgHordePlayerCount != 0)
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

void BattlePlayerbotMgr::ActivateFactionBots(IdleBotFinder& finder, BattlegroundInfo& bgInfo, BracketInfo& bracket, BattlegroundQueueTypeId queueTypeId,
        BattlegroundBracketId bracketId, bool arena, uint32 addBotCount)
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
        AddPlayerBot(ObjectGuid::Create<HighGuid::Player>(state->id), 0);
    }
}

void BattlePlayerbotMgr::RemoveInactiveBots()
{
    std::vector<uint32> removeIds;
    for (auto itr = activeBots.begin(); itr != activeBots.end(); itr++)
    {
        BattleBotState *state = itr->second;
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(state->id);
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
    uint32 expansion = sWorld->getIntConfig(CONFIG_EXPANSION);

    for (uint32 accountId : sRandomPlayerbotMgr.GetBattleTypeAccounts())
    {
        CharacterDatabasePreparedStatement* stmt =
            CharacterDatabase.GetPreparedStatement(CHAR_SEL_ACCOUNT_INFO_CHARS);
        stmt->SetData(0, accountId);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (!result)
            continue;

        do
        {
            Field* fields = result->Fetch();
            CharacterInfo info;
            info.guid = fields[0].Get<uint32>();
            info.rRace = fields[3].Get<uint8>();
            info.rClass = fields[4].Get<uint8>();
            if ((expansion < EXPANSION_THE_BURNING_CRUSADE && (info.rRace == RACE_BLOODELF || info.rRace == RACE_DRAENEI)) ||
                (expansion < EXPANSION_WRATH_OF_THE_LICH_KING && info.rClass == CLASS_DEATH_KNIGHT))
                continue;
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
    uint32 id = bot->GetGUID().GetCounter();
    auto activeItr = activeBots.find(id);
    if (activeItr == activeBots.end())
    {
        LOG_ERROR("playerbots", "BattleBot {} logged in but not found in active list", id);
        LogoutPlayerBot(bot->GetGUID());
        RemoveInactiveBot(id);
        return;
    }

    PlayerbotAI* botAI = sPlayerbotsMgr.GetPlayerbotAI(bot);
    if (!botAI)
    {
        LOG_ERROR("playerbots", "BattleBot {} logged in but not found in PlayerbotsMgr", id);
        LogoutPlayerBot(bot->GetGUID());
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
