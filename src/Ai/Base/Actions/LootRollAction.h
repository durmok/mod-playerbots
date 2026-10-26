/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOOTROLLACTION_H
#define PLAYERBOTS_LOOTROLLACTION_H

#include "QueryItemUsageAction.h"
#include "Group.h"

class PlayerbotAI;

struct ItemTemplate;

// enum RollVote : uint8;

class LootRollAction : public QueryItemUsageAction
{
public:
    LootRollAction(PlayerbotAI* botAI, std::string const name = "loot roll") : QueryItemUsageAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    RollVote CalculateRollVote(ItemTemplate const* proto, ItemUsage usage = ITEM_USAGE_NONE);
};

bool CanBotUseToken(ItemTemplate const* proto, Player* bot);
bool RollUniqueCheck(ItemTemplate const* proto, Player* bot);

class MasterLootRollAction : public LootRollAction
{
public:
    MasterLootRollAction(PlayerbotAI* botAI) : LootRollAction(botAI, "master loot roll") {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class RollAction : public Action
{
public:
    RollAction(PlayerbotAI* botAI) : Action(botAI, "roll") {}

    bool Execute(Event event) override;
};

class RollDependChecker
{
public:
    RollDependChecker(ItemTemplate const* itemProto, Player* bot, Group* group, ObjectGuid const* itemGUID = nullptr);

    bool Check();

    ItemTemplate const* itemProto;
    bool depend;
    std::vector<uint32> dependPlayers;
    std::ostringstream verbose;

private:
    void CheckClass(std::initializer_list<uint32> preferedClasses);

    Player* bot;
    Group* group;
    const Roll* itemRoll;
};

class LootStartRollAction : public LootRollAction
{
public:
    LootStartRollAction(PlayerbotAI* botAI) : LootRollAction(botAI, "loot start roll") {}

    bool Execute(Event event) override;
    bool ExecutePacket(WorldPacket& packet);
};

#endif
