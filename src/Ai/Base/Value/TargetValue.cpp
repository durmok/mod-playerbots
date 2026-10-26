/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TargetValue.h"
// #include "CombatManager.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
// #include "ScriptedCreature.h"
#include "Strategy.h"
#include "ThreatManager.h"

GuidSet GatherStrategyTargetExclusions(PlayerbotAI* botAI, TargetValueExclusionType type)
{
    GuidSet exclusions;
    if (!botAI || type == TargetValueExclusionType::None || !botAI->HasTargetExclusions())
        return exclusions;

    for (auto const& strategyName : botAI->GetStrategies(BOT_STATE_COMBAT))
    {
        Strategy* strategy = botAI->GetStrategy(strategyName, BOT_STATE_COMBAT);
        if (!strategy)
            continue;

        strategy->AppendTargetExclusions(exclusions, type);
    }

    return exclusions;
}

Unit* FindTargetStrategy::GetResult() { return result; }

TargetValueExclusionType FindTargetStrategy::GetExclusionType() { return TargetValueExclusionType::None; }

Unit* TargetValue::FindTarget(FindTargetStrategy* strategy)
{
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    GuidSet const dynamicExclusions = GatherStrategyTargetExclusions(botAI, strategy->GetExclusionType());
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || dynamicExclusions.find(guid) != dynamicExclusions.end())
            continue;

        ThreatManager& threatMgr = unit->GetThreatMgr();
        strategy->CheckAttacker(unit, &threatMgr);
    }

    return strategy->GetResult();
}

bool FindNonCcTargetStrategy::IsCcTarget(Unit* attacker)
{
    if (Group* group = botAI->GetBot()->GetGroup())
    {
        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* member = ObjectAccessor::FindPlayer(itr->guid);
            if (!member || !member->IsAlive())
                continue;

            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member))
            {
                if (botAI->GetAiObjectContext()->GetValue<Unit*>("rti cc target")->Get() == attacker)
                    return true;

                std::string const rti = botAI->GetAiObjectContext()->GetValue<std::string>("rti cc")->Get();
                int32 index = RtiTargetValue::GetRtiIndex(rti);
                if (index != -1)
                {
                    if (ObjectGuid guid = group->GetTargetIcon(index))
                        if (attacker->GetObjectGuid() == guid)
                            return true;
                }
            }
        }

        if (ObjectGuid guid = group->GetTargetIcon(4))
            if (attacker->GetObjectGuid() == guid)
                return true;
    }

    return false;
}

void FindTargetStrategy::GetPlayerCount(Unit* creature, uint32* tankCount, uint32* dpsCount)
{
    Player* bot = botAI->GetBot();
    if (tankCountCache.find(creature) != tankCountCache.end())
    {
        *tankCount = tankCountCache[creature];
        *dpsCount = dpsCountCache[creature];
        return;
    }

    *tankCount = 0;
    *dpsCount = 0;

    Unit::AttackerSet attackers(creature->GetAttackers());
    for (Unit* attacker : attackers)
    {
        if (!attacker || !attacker->IsAlive() || attacker == bot)
            continue;

        Player* player = attacker->ToPlayer();
        if (!player)
            continue;

        if (PlayerbotAI::IsTank(player))
            ++(*tankCount);
        else
            ++(*dpsCount);
    }

    tankCountCache[creature] = *tankCount;
    dpsCountCache[creature] = *dpsCount;
}

bool FindTargetStrategy::IsHighPriority(Unit* attacker)
{
    if (Group* group = botAI->GetBot()->GetGroup())
    {
        ObjectGuid guid = group->GetTargetIcon(7);
        if (guid && attacker->GetObjectGuid() == guid)
        {
            return true;
        }
    }
    GuidVector prioritizedTargets = botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Get();
    for (ObjectGuid targetGuid : prioritizedTargets)
    {
        if (targetGuid && attacker->GetObjectGuid() == targetGuid)
        {
            return true;
        }
    }
    return false;
}

WorldPosition LastLongMoveValue::Calculate()
{
    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");
    if (lastMove.lastPath.empty())
        return WorldPosition();

    return lastMove.lastPath.getBack();
}

WorldPosition HomeBindValue::Calculate()
{
    auto home = bot->GetHomeBind();
    return WorldPosition(home.mapId, home.x, home.y, home.z, 0.f);
}

Unit* FindTargetValue::Calculate()
{
    if (qualifier == "")
    {
        return nullptr;
    }
    Group* group = bot->GetGroup();
    if (!group)
    {
        return nullptr;
    }
    // for (auto const& ref : bot->GetThreatMgr().GetThreatenedByMeList())
    HostileReference* ref = bot->GetHostileRefManager().getFirst();
    while (ref)
    {
        Unit* unit = ref->getSourceUnit();
        ref = ref->next();
        if (!unit)
            continue;

        std::wstring wnamepart;
        Utf8toWStr(unit->GetName(), wnamepart);
        wstrToLower(wnamepart);
        if (!qualifier.empty() && qualifier.length() == wnamepart.length() && Utf8FitTo(qualifier, wnamepart))
            return unit;
    }

    return nullptr;
}

void FindBossTargetStrategy::CheckAttacker(Unit* attacker, ThreatManager* /*threatManager*/)
{
    /*
    attacker->ToCreature()->GetAIName();
    UnitAI* unitAI = attacker->GetAI();
    BossAI* bossAI = dynamic_cast<BossAI*>(unitAI);
    if (bossAI)
    {
        result = attacker;
    }
    */
    if (attacker->IsCreature() && attacker->ToCreature()->IsWorldBoss())
        result = attacker;
}

Unit* BossTargetValue::Calculate()
{
    FindBossTargetStrategy strategy(botAI);
    return FindTarget(&strategy);
}
