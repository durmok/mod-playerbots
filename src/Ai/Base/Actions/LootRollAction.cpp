/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LootRollAction.h"
#include "Event.h"
#include "Group.h"
#include "ItemUsageValue.h"
#include "LootAction.h"
#include "ObjectMgr.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool LootRollAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Roll*> rolls = group->GetRolls();
    bool voted = false;
    for (Roll const* roll : rolls)
    {
        auto voteItr = roll->playerVote.find(bot->GetObjectGuid());
        if (voteItr == roll->playerVote.end() || voteItr->second != ROLL_NOT_EMITED_YET)
            continue;

        // ObjectGuid guid = roll->itemGUID;
        ObjectGuid guid = roll->lootedTargetGUID;
        uint32 itemId = roll->itemid;
        int32 randomProperty = 0;
        if (roll->itemRandomPropId)
            randomProperty = roll->itemRandomPropId;
        /*
        else if (roll->itemRandomSuffix)
            randomProperty = -((int)roll->itemRandomSuffix);
        */

        RollVote vote = ROLL_PASS;
        ItemTemplate const* proto = sObjectMgr.GetItemTemplate(itemId);
        if (!proto)
            continue;

        std::string itemUsageParam;
        if (randomProperty != 0)
            itemUsageParam = std::to_string(itemId) + "," + std::to_string(randomProperty);
        else
            itemUsageParam = std::to_string(itemId);

        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemUsageParam);

        // Armor Tokens are classed as MISC JUNK (Class 15, Subclass 0), luckily no other items I found have class bits and epic quality.
        if (proto->Class == ITEM_CLASS_JUNK && proto->SubClass == ITEM_SUBCLASS_JUNK && proto->Quality == ITEM_QUALITY_EPIC)
        {
            if (CanBotUseToken(proto, bot))
                vote = ROLL_NEED; // Eligible for "Need"
            else
                vote = ROLL_GREED; // Not eligible, so "Greed"
        }
        else if (usage == ITEM_USAGE_DISENCHANT)
            // vote = sPlayerbotAIConfig.LootRollDisenchant ? DISENCHANT : GREED;
            vote = ROLL_PASS;
        else
        {
            switch (proto->Class)
            {
                case ITEM_CLASS_WEAPON:
                case ITEM_CLASS_ARMOR:
                    if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_BAD_EQUIP)
                        vote = ROLL_NEED;
                    else if (usage != ITEM_USAGE_NONE)
                        vote = ROLL_GREED;
                    break;
                case ITEM_CLASS_RECIPE:
                    if (!sPlayerbotAIConfig.LootRollRecipe)
                        vote = ROLL_PASS;
                    else if (usage == ITEM_USAGE_SKILL)
                        vote = ROLL_NEED;  // Bot can learn this recipe
                    else if (proto->Bonding != BIND_WHEN_PICKED_UP)
                        vote = ROLL_GREED;  // BoE recipe bot can't learn - GREED for AH/trade
                    break;
                default:
                    if (StoreLootAction::IsLootAllowed(itemId, botAI))
                        vote = CalculateRollVote(proto, usage);
                    break;
            }
        }
        if (vote == ROLL_NEED)
        {
            if (sPlayerbotAIConfig.LootNeedRollLevel == 0 || RollUniqueCheck(proto, bot))
                vote = ROLL_PASS;
            else if (sPlayerbotAIConfig.LootNeedRollLevel == 1)
                vote = ROLL_GREED;
        }
        else if (vote == ROLL_GREED && !sPlayerbotAIConfig.LootGreedRollLevel)
            vote = ROLL_PASS;

        switch (group->GetLootMethod())
        {
            case MASTER_LOOT:
            case FREE_FOR_ALL:
                group->CountRollVote(bot, guid, roll->itemSlot, ROLL_PASS);
                break;
            default:
                group->CountRollVote(bot, guid, roll->itemSlot, vote);
                break;
        }
        voted = true;
    }

    return voted;
}

RollVote LootRollAction::CalculateRollVote(ItemTemplate const* proto, ItemUsage usage)
{
    if (usage == ITEM_USAGE_NONE)
    {
        std::ostringstream out;
        out << proto->ItemId;
        usage = AI_VALUE2(ItemUsage, "item usage", out.str());
    }

    RollVote needVote = ROLL_PASS;
    switch (usage)
    {
        case ITEM_USAGE_EQUIP:
        case ITEM_USAGE_REPLACE:
        case ITEM_USAGE_GUILD_TASK:
        case ITEM_USAGE_BAD_EQUIP:
            needVote = ROLL_NEED;
            break;
        case ITEM_USAGE_SKILL:
        case ITEM_USAGE_USE:
        case ITEM_USAGE_AH:
        case ITEM_USAGE_VENDOR:
            needVote = ROLL_GREED;
            break;
        /*
        case ITEM_USAGE_DISENCHANT:
            needVote = sPlayerbotAIConfig.LootRollDisenchant ? DISENCHANT : GREED;
            break;
        */
        default:
            break;
    }

    return StoreLootAction::IsLootAllowed(proto->ItemId, GET_PLAYERBOT_AI(bot)) ? needVote : ROLL_PASS;
}

bool MasterLootRollAction::isUseful() { return !IsRealPlayer(botAI->GetMaster()); }

bool MasterLootRollAction::Execute(Event event)
{
    Player* bot = QueryItemUsageAction::botAI->GetBot();

    /*
    WorldPacket p(event.getPacket());  // WorldPacket packet for CMSG_LOOT_ROLL, (8+4+1)
    ObjectGuid creatureGuid;
    uint32 mapId;
    uint32 itemSlot;
    uint32 itemId;
    uint32 randomSuffix;
    uint32 randomPropertyId;
    uint32 count;
    uint32 timeout;

    p.rpos(0);              // reset packet pointer
    p >> creatureGuid;      // creature guid what we're looting
    p >> mapId;             /// 3.3.3 mapid
    p >> itemSlot;          // the itemEntryId for the item that shall be rolled for
    p >> itemId;            // the itemEntryId for the item that shall be rolled for
    p >> randomSuffix;      // randomSuffix
    p >> randomPropertyId;  // item random property ID
    p >> count;             // items in stack
    p >> timeout;           // the countdown time to choose "need" or "greed"
    */

    auto p = event.getClientPacket<WorldPackets::Loot::LootRoll>();
    if (!p)
        return false;
    uint32 creatureGuid = p->lootedTarget;
    uint32 itemId = p->lootedTarget.GetCounter();
    uint32 itemSlot = p->itemSlot;

    ItemTemplate const* proto = sObjectMgr.GetItemTemplate(itemId);
    if (!proto)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Roll*> rolls = group->GetRolls();
    bool voted = false;
    for (Roll const* roll : rolls)
    {
        auto voteItr = roll->playerVote.find(bot->GetObjectGuid());
    }

    group->CountRollVote(bot, creatureGuid, itemSlot, CalculateRollVote(proto));

    return true;
}

bool CanBotUseToken(ItemTemplate const* proto, Player* bot)
{
    // Get the bitmask for the bot's class
    uint32 botClassMask = (1 << (bot->getClass() - 1));

    // Check if the bot's class is allowed to use the token
    if (proto->AllowableClass & botClassMask)
        return true; // Bot's class is eligible to use this token

    return false; // Bot's class cannot use this token
}

bool RollUniqueCheck(ItemTemplate const* proto, Player* bot)
{
    // Count the total number of the item (equipped + in bags)
    uint32 totalItemCount = bot->GetItemCount(proto->ItemId, true);

    // Count the number of the item in bags only
    uint32 bagItemCount = bot->GetItemCount(proto->ItemId, false);

    // Determine if the unique item is already equipped
    bool isEquipped = (totalItemCount > bagItemCount);
    if (isEquipped && proto->HasFlag(ITEM_FLAG_UNIQUE_EQUIPPED))
        return true;  // Unique Item is already equipped
    else if (proto->HasFlag(ITEM_FLAG_UNIQUE_EQUIPPED) && (bagItemCount > 1))
        return true; // Unique item already in bag, don't roll for it
    return false; // Item is not equipped or in bags, roll for it
}

bool RollAction::Execute(Event event)
{
    std::string link = event.getParam();

    if (link.empty())
    {
        bot->DoRandomRoll(0,100);
        return false;
    }
    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
        return false;
    uint32 itemId = *itemIds.begin();
    ItemTemplate const* proto = sObjectMgr.GetItemTemplate(itemId);
    if (!proto)
    {
        return false;
    }
    std::string itemUsageParam;
    itemUsageParam = std::to_string(itemId);

    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemUsageParam);
    bool doRoll = false;
    switch (proto->Class)
    {
        case ITEM_CLASS_JUNK:
        if (proto->SubClass == ITEM_SUBCLASS_JUNK && proto->Quality == ITEM_QUALITY_EPIC)
        {
            if (CanBotUseToken(proto, bot) && !RollUniqueCheck(proto, bot))
                doRoll = true;
        }
        break;
        case ITEM_CLASS_WEAPON:
        case ITEM_CLASS_ARMOR:
        if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_BAD_EQUIP)
            doRoll = true;
    }
    if (doRoll)
        bot->DoRandomRoll(0,100);
    return true;
}

bool LootStartRollAction::Execute(Event event)
{
    bool ret = false;
    auto packets = event.getAllPackets();
    for (auto itr = packets.begin(); itr != packets.end(); ++itr)
        ret |= ExecutePacket(itr->first);
    return ret;
}

bool LootStartRollAction::ExecutePacket(WorldPacket& packet)
{
    WorldPacket p(packet);  // WorldPacket packet for SMSG_LOOT_START_ROLL, (8 + 4 + 4 + 4 + 4 + 4 + 4 + 1)
    ObjectGuid itemGUID;
    // uint32 mapId;
    uint32 itemSlot;
    uint32 itemId;
    uint32 randomSuffix;
    uint32 randomPropertyId;
    // uint32 count;
    uint32 timeout;

    p.rpos(0);
    p >> itemGUID;
    // p >> mapId;
    p >> itemSlot;
    p >> itemId;
    p >> randomSuffix;
    p >> randomPropertyId;
    // p >> count;
    p >> timeout;

    ItemTemplate const* proto = sObjectMgr.GetItemTemplate(itemId);
    if (!proto)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    RollVote vote = ROLL_PASS;
    switch (proto->Class)
    {
        case ITEM_CLASS_JUNK:
            if (proto->SubClass == ITEM_SUBCLASS_JUNK && proto->Quality == ITEM_QUALITY_EPIC)
            {
                if (CanBotUseToken(proto, bot) && !RollUniqueCheck(proto, bot))
                    vote = ROLL_NEED;
            }
            break;
        case ITEM_CLASS_WEAPON:
        case ITEM_CLASS_ARMOR:
        {
            int32 randomProperty = 0;
            if (randomPropertyId)
                randomProperty = randomPropertyId;
            else if (randomSuffix)
                randomProperty = -((int)randomSuffix);

            std::string itemUsageParam;
            if (randomProperty != 0)
                itemUsageParam = std::to_string(itemId) + "," + std::to_string(randomProperty);
            else
                itemUsageParam = std::to_string(itemId);
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemUsageParam);

            if ((usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_BAD_EQUIP) && !RollUniqueCheck(proto, bot))
            {
                RollDependChecker checker(proto, bot, group, &itemGUID);
                if (checker.Check())
                    vote = ROLL_GREED;
                else
                    vote = ROLL_NEED;
                LOG_INFO("playerbots", "{} USE={} {}", bot->GetName(), usage, checker.verbose.str().c_str());
            }
            break;
        }
        default:
            break;
    }

    group->CountRollVote(bot, itemGUID, itemSlot, vote);
    return true;
}

RollDependChecker::RollDependChecker(ItemTemplate const* itemProto, Player* bot, Group* group, ObjectGuid const* itemGUID): itemProto(itemProto), depend(false), bot(bot), group(group)
{
    this->itemRoll = nullptr;
    if (itemGUID)
    {
        // find current roll of item to check
        std::vector<Roll*> rolls = group->GetRolls();
        for (const Roll* roll : rolls)
        {
            if (roll->lootedTargetGUID == *itemGUID)
            {
                this->itemRoll = roll;
                break;
            }
        }
    }
    verbose << itemProto->Name1 << " CLS=" << itemProto->Class << " SCLS=" << itemProto->SubClass;
}

void RollDependChecker::CheckClass(std::initializer_list<uint32> preferedClasses)
{
    if (depend)
        return;

    uint32 preferdMask = 0;
    for (uint32 x : preferedClasses) {
        preferdMask |= (1 << (x - 1));
    }

    // do not abdicate if self is prefered class
    if ((bot->getClassMask() & preferdMask) != 0)
        return;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member == bot)
            continue;

        if ((member->getClassMask() & preferdMask) != 0)
        {
            if (itemRoll)
            {
                auto voteItr = itemRoll->playerVote.find(member->GetObjectGuid());
                verbose << " " << member->GetName() << "(";
                if (voteItr != itemRoll->playerVote.end())
                {
                    verbose << uint32(voteItr->second);
                }
                verbose << ")";
                if (voteItr == itemRoll->playerVote.end() || voteItr->second == ROLL_NOT_EMITED_YET)
                {
                    dependPlayers.push_back(member->GetObjectGuid().GetCounter());
                    depend = true;
                    verbose << " D1";
                }
                if (voteItr->second == ROLL_NEED)
                {
                    dependPlayers.clear();
                    depend = true;
                    verbose << " D2";
                    break;
                }
            }
            else
            {
                verbose << " " << member->GetName();
                dependPlayers.clear();
                depend = true;
                verbose << " D3";
                break;
            }
        }
    }
}

bool RollDependChecker::Check()
{
    if (itemProto->Class == ITEM_CLASS_WEAPON)
    {
        switch (itemProto->SubClass) {
            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            case ITEM_SUBCLASS_WEAPON_GUN:
                CheckClass({ CLASS_HUNTER });
                break;
            case ITEM_SUBCLASS_WEAPON_STAFF:
                CheckClass( { CLASS_MAGE, CLASS_PRIEST, CLASS_WARLOCK, CLASS_DRUID });
                break;
            case ITEM_SUBCLASS_WEAPON_WAND:
                CheckClass( { CLASS_MAGE, CLASS_PRIEST, CLASS_WARLOCK });
                break;
            case ITEM_SUBCLASS_WEAPON_DAGGER:
                CheckClass( { CLASS_ROGUE, CLASS_DRUID, CLASS_HUNTER });
                break;
            case ITEM_SUBCLASS_WEAPON_FIST:
                CheckClass( { CLASS_ROGUE, CLASS_SHAMAN });
            case ITEM_SUBCLASS_WEAPON_THROWN:
                CheckClass( { CLASS_ROGUE, CLASS_HUNTER });
            case ITEM_SUBCLASS_WEAPON_AXE:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_SHAMAN, CLASS_ROGUE });
            case ITEM_SUBCLASS_WEAPON_AXE2:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
            case ITEM_SUBCLASS_WEAPON_MACE:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_SHAMAN, CLASS_DRUID, CLASS_ROGUE });
            case ITEM_SUBCLASS_WEAPON_MACE2:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_SHAMAN, CLASS_DRUID });
            case ITEM_SUBCLASS_WEAPON_POLEARM:
                CheckClass( { CLASS_HUNTER, CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_DRUID });
            case ITEM_SUBCLASS_WEAPON_SWORD:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_ROGUE });
            case ITEM_SUBCLASS_WEAPON_SWORD2:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            default:
                break;
        }
    }
    else if (itemProto->Class == ITEM_CLASS_ARMOR)
    {
        switch (itemProto->SubClass) {
            case ITEM_SUBCLASS_ARMOR_CLOTH:
                if (itemProto->InventoryType != INVTYPE_CLOAK)
                    CheckClass( { CLASS_MAGE, CLASS_PRIEST, CLASS_WARLOCK });
                break;
            case ITEM_SUBCLASS_ARMOR_LEATHER:
                CheckClass( { CLASS_HUNTER, CLASS_ROGUE, CLASS_DRUID });
                break;
            case ITEM_SUBCLASS_ARMOR_MAIL:
                CheckClass( { CLASS_HUNTER, CLASS_SHAMAN });
                break;
            case ITEM_SUBCLASS_ARMOR_PLATE:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            case ITEM_SUBCLASS_ARMOR_BUCKLER:
            case ITEM_SUBCLASS_ARMOR_SHIELD:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_SHAMAN });
                break;
            case ITEM_SUBCLASS_ARMOR_TOTEM:
                CheckClass( { CLASS_SHAMAN });
                break;
            default:
                break;
        }
    }

    if (depend)
        return true;

    uint32 maxStatType = 0;
    int32 maxStatValue = -1;
    for (auto const& stat : itemProto->ItemStat) {
        if (stat.ItemStatValue > maxStatValue)
        {
            maxStatValue = stat.ItemStatValue;
            maxStatType = stat.ItemStatType;
        }
    }
    verbose << " MAXSTAT=" << maxStatType;
    if (maxStatValue != 0)
    {
        switch (maxStatType) {
            case ITEM_MOD_MANA:
                CheckClass( { CLASS_PRIEST, CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN, CLASS_PALADIN, CLASS_DRUID });
                break;
            case ITEM_MOD_HEALTH:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            case ITEM_MOD_AGILITY:
                CheckClass( { CLASS_ROGUE, CLASS_HUNTER, CLASS_SHAMAN, CLASS_DRUID });
                break;
            case ITEM_MOD_STRENGTH:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            case ITEM_MOD_INTELLECT:
                CheckClass( { CLASS_PRIEST, CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN, CLASS_PALADIN, CLASS_DRUID });
                break;
            case ITEM_MOD_SPIRIT:
                CheckClass( { CLASS_PRIEST, CLASS_MAGE, CLASS_SHAMAN, CLASS_DRUID });
                break;
            case ITEM_MOD_STAMINA:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            /*
            case ITEM_MOD_DEFENSE_SKILL_RATING:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            case ITEM_MOD_DODGE_RATING:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_DRUID });
                break;
            case ITEM_MOD_PARRY_RATING:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            case ITEM_MOD_BLOCK_RATING:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN });
                break;
            case ITEM_MOD_HIT_MELEE_RATING:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_ROGUE, CLASS_SHAMAN, CLASS_DRUID });
                break;
            case ITEM_MOD_HIT_RANGED_RATING:
                CheckClass( { CLASS_HUNTER });
                break;
            case ITEM_MOD_HIT_SPELL_RATING:
                CheckClass( { CLASS_PRIEST, CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN, CLASS_DRUID });
                break;
            case ITEM_MOD_CRIT_MELEE_RATING:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_ROGUE, CLASS_SHAMAN, CLASS_DRUID });
                break;
            case ITEM_MOD_CRIT_RANGED_RATING:
                CheckClass( { CLASS_HUNTER });
                break;
            case ITEM_MOD_CRIT_SPELL_RATING:
                CheckClass( { CLASS_PRIEST, CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN, CLASS_DRUID });
                break;
            case ITEM_MOD_HIT_TAKEN_MELEE_RATING:
            case ITEM_MOD_HIT_TAKEN_RANGED_RATING:
            case ITEM_MOD_HIT_TAKEN_SPELL_RATING:
            case ITEM_MOD_CRIT_TAKEN_MELEE_RATING:
            case ITEM_MOD_CRIT_TAKEN_RANGED_RATING:
            case ITEM_MOD_CRIT_TAKEN_SPELL_RATING:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            case ITEM_MOD_HASTE_MELEE_RATING:
                CheckClass( { CLASS_WARRIOR, CLASS_ROGUE, CLASS_DEATH_KNIGHT, CLASS_SHAMAN, CLASS_DRUID });
                break;
            case ITEM_MOD_HASTE_RANGED_RATING:
                CheckClass( { CLASS_HUNTER });
                break;
            case ITEM_MOD_HASTE_SPELL_RATING:
                CheckClass( { CLASS_PRIEST, CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN });
                break;
            case ITEM_MOD_HIT_RATING:
            case ITEM_MOD_CRIT_RATING:
                break;
            case ITEM_MOD_HIT_TAKEN_RATING:
            case ITEM_MOD_CRIT_TAKEN_RATING:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            case ITEM_MOD_RESILIENCE_RATING:
            case ITEM_MOD_HASTE_RATING:
                CheckClass( { CLASS_ROGUE, CLASS_HUNTER, CLASS_PRIEST, CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN, CLASS_DRUID });
                break;
            case ITEM_MOD_EXPERTISE_RATING:
                CheckClass( { CLASS_ROGUE, CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            case ITEM_MOD_ATTACK_POWER:
                CheckClass( { CLASS_ROGUE, CLASS_HUNTER, CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_SHAMAN, CLASS_DRUID });
                break;
            case ITEM_MOD_RANGED_ATTACK_POWER:
                CheckClass( { CLASS_HUNTER });
                break;
            case ITEM_MOD_MANA_REGENERATION:
                CheckClass( { CLASS_PRIEST, CLASS_SHAMAN, CLASS_PALADIN, CLASS_DRUID });
                break;
            case ITEM_MOD_ARMOR_PENETRATION_RATING:
                CheckClass( { CLASS_ROGUE, CLASS_HUNTER, CLASS_WARRIOR, CLASS_DEATH_KNIGHT, CLASS_DRUID });
                break;
            case ITEM_MOD_SPELL_POWER:
                CheckClass( { CLASS_PRIEST, CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN, CLASS_PALADIN, CLASS_DRUID });
                break;
            case ITEM_MOD_HEALTH_REGEN:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT });
                break;
            case ITEM_MOD_SPELL_PENETRATION:
                CheckClass( { CLASS_PRIEST, CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN, CLASS_PALADIN, CLASS_DRUID });
                break;
            case ITEM_MOD_BLOCK_VALUE:
                CheckClass( { CLASS_WARRIOR, CLASS_PALADIN });
                break;
            */
            default:
                break;
        }
    }

    return depend;
}
