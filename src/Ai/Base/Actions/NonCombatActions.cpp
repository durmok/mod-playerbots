/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NonCombatActions.h"
#include "Event.h"
#include "Playerbots.h"

namespace
{
constexpr uint32 BG_WS_SPELL_WARSONG_FLAG = 23333;
constexpr uint32 BG_WS_SPELL_SILVERWING_FLAG = 23335;
constexpr uint32 BG_EY_NETHERSTORM_FLAG_SPELL = 34976;

bool IsDisallowedShapeshiftForm(Player* bot)
{
    if (bot->getClass() == CLASS_DRUID)
    {
        ShapeshiftForm form = bot->GetShapeshiftForm();
        return form == FORM_TRAVEL || form == FORM_AQUA ||
            //    form == FORM_FLIGHT || form == FORM_FLIGHT_EPIC ||
               form == FORM_BEAR || form == FORM_DIREBEAR ||
               form == FORM_CAT;
    }
    else if (bot->getClass() == CLASS_PRIEST)
    {
        return bot->GetShapeshiftForm() == FORM_SPIRITOFREDEMPTION;
    }

    return false;
}
}

bool DrinkAction::Execute(Event event)
{
    if (botAI->HasCheat(BotCheatMask::food))
    {
        // if (bot->IsNonMeleeSpellCast(true))
        //     return false;

        bot->ClearUnitState(UNIT_STATE_CHASE);
        bot->ClearUnitState(UNIT_STATE_FOLLOW);

        if (bot->isMoving())
        {
            bot->StopMoving();
            // botAI->SetNextCheckDelay(sPlayerbotAIConfig->GlobalCoolDown);
            // return false;
        }
        bot->SetStandState(UNIT_STAND_STATE_SIT);
        bot->CastStop();

        // float hp = bot->GetHealthPercent();
        float mp = bot->GetPowerPct(POWER_MANA);
        float p = mp;
        float delay;

        if (!bot->InBattleground())
            delay = 18.0f * IN_MILLISECONDS * (100 - p) / 100.0f;
        else
            delay = 12.0f * IN_MILLISECONDS * (100 - p) / 100.0f;

        SetDuration((uint32)delay);

        bot->AddAura(25990);
        return true;
        // return botAI->CastSpell(24707, bot);
    }

    return UseItemAction::Execute(event);
}

bool DrinkAction::isUseful()
{
    return UseItemAction::isUseful() && AI_VALUE2(bool, "has mana", "self target") &&
           AI_VALUE2(uint8, "mana", "self target") < 85;
}

bool DrinkAction::isPossible()
{
    if (bot->IsInCombat() || bot->IsMounted() || IsDisallowedShapeshiftForm(bot))
        return false;

    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible();
}

bool EatAction::Execute(Event event)
{
    if (botAI->HasCheat(BotCheatMask::food))
    {
        // if (bot->IsNonMeleeSpellCast(true))
        //     return false;

        bot->ClearUnitState(UNIT_STATE_CHASE);
        bot->ClearUnitState(UNIT_STATE_FOLLOW);

        if (bot->isMoving())
        {
            bot->StopMoving();
            // botAI->SetNextCheckDelay(sPlayerbotAIConfig.GlobalCoolDown);
            // return false;
        }

        bot->SetStandState(UNIT_STAND_STATE_SIT);
        bot->CastStop();

        float hp = bot->GetHealthPct();
        // float mp = bot->HasMana() ? bot->GetPowerPercent() : 0.f;
        float p = hp;
        float delay;

        if (!bot->InBattleground())
            delay = 18.0f * IN_MILLISECONDS * (100 - p) / 100.0f;
        else
            delay = 12.0f * IN_MILLISECONDS * (100 - p) / 100.0f;

        SetDuration((uint32)delay);

        bot->AddAura(25990);
        return true;
    }

    return UseItemAction::Execute(event);
}

bool EatAction::isUseful()
{
    return UseItemAction::isUseful() &&
           AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig.LowHealth;
}

bool EatAction::isPossible()
{
    if (bot->IsInCombat() || bot->IsMounted() || IsDisallowedShapeshiftForm(bot))
        return false;

    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible();
}

uint32 selectFoodForLevel(uint32 level) {
    if (level < 12)
        return 433;
    else if (level < 22)
        return 434;
    else if (level < 32)
        return 435;
    else if (level < 42)
        return 1127;
    else if (level < 52)
        return 1129;
    else if (level < 60)
        return 1131;
    else if (level < 65)
        return 29073;
    else if (level < 70)
        return 22018;
    else if (level < 75)
        return 43015;
    else if (level < 80)
        return 61251;
    else
        return 61253;
}

uint32 selectDrinkForLevel(uint32 level) {
    if (level < 10)
        return 430;
    else if (level < 20)
        return 431;
    else if (level < 30)
        return 432;
    else if (level < 40)
        return 1133;
    else if (level < 50)
        return 1135;
    else if (level < 60)
        return 1137;
    else if (level < 65)
        return 22734;
    else if (level < 70)
        return 27089;
    else if (level < 75)
        return 42917;
    else if (level < 80)
        return 61255;
    else
        return 61257;
}

bool EatDrinkAction::Execute(Event /*event*/)
{
    bot->ClearUnitState(UNIT_STATE_CHASE);
    bot->ClearUnitState(UNIT_STATE_FOLLOW);

    if (bot->isMoving())
        bot->StopMoving();
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE)
        bot->GetMotionMaster()->Clear(false);

    bot->SetStandState(UNIT_STAND_STATE_SIT);

    if (AI_VALUE2(uint8, "health", GetTargetName()) < 100)
        bot->AddAura(selectFoodForLevel(bot->GetLevel()));
    if (AI_VALUE2(bool, "has mana", GetTargetName()) &&
        AI_VALUE2(uint8, "mana", GetTargetName()) < 100)
        bot->AddAura(selectDrinkForLevel(bot->GetLevel()));
    return true;
}

bool EatDrinkAction::isUseful() { 
    return true;
}

bool EatDrinkAction::isPossible()
{
    if (bot->IsInCombat() || bot->IsMounted() || IsDisallowedShapeshiftForm(bot) || bot->isMoving())
        return false;

    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
    {
        return false;
    }

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    return true;
}

