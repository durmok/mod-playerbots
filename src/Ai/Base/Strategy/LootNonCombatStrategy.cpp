/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LootNonCombatStrategy.h"
#include "Playerbots.h"
#include "LootObjectStack.h"
#include "LootTriggers.h"

void LootNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("loot available", { NextAction("loot", 6.0f) }));
    triggers.push_back(
        new TriggerNode("far from loot target", { NextAction("move to loot", 7.0f) }));
    triggers.push_back(new TriggerNode("can loot", { NextAction("open loot", 8.0f) }));
    if (!botAI->IsAltBot())
        triggers.push_back(new TriggerNode("often", { NextAction("add all loot", 5.0f) }));
}

void LootNonCombatStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new LootMultiplier(botAI));
}

void GatherStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("timer", { NextAction("add gathering loot", 5.0f) }));
}

void RevealStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("often", { NextAction("reveal gathering item", 50.0f) }));
}

void UseBobberStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
     triggers.push_back(
        new TriggerNode("can use fishing bobber", { NextAction("use fishing bobber", 20.0f) }));
    triggers.push_back(
        new TriggerNode("random", { NextAction("remove bobber strategy", 20.0f) }));
}

float LootMultiplier::GetValue(Action* action)
{
    std::string const actionName = action->getName();
    if (actionName == "follow" || actionName == "eat drink")
    {
        auto lootTarget= AI_VALUE(LootObject, "loot target");
        if (!lootTarget.IsEmpty() && lootTarget.IsLootPossible(bot))
            return 0.0f;
        if (!AI_VALUE(LootObjectStack*, "available loot")->GetLoot(sPlayerbotAIConfig.LootDistance).IsEmpty())
            return 0.0f;
    }
    return 1.0f;
}