#include "./EnumConverter.hpp"
#include <unordered_map>

EffectTrigger StringToEffectTrigger(const std::string &trigger) {
    static const std::unordered_map<std::string, EffectTrigger> map = {
        {"ON_ENTER", EffectTrigger::ON_ENTER},
        {"ON_ATTACK", EffectTrigger::ON_ATTACK},
        {"ON_ATTACK_BLOCKED", EffectTrigger::ON_ATTACK_BLOCKED},
        {"ON_KILL", EffectTrigger::ON_KILL},
        {"ON_DEATH", EffectTrigger::ON_DEATH},
        {"ON_EVOLUTION", EffectTrigger::ON_EVOLUTION},
        {"ON_SPELL_CAST", EffectTrigger::ON_SPELL_CAST},
        {"ON_OPPONENT_PLAY", EffectTrigger::ON_OPPONENT_PLAY},
        {"ON_RECEIVE_HEAL", EffectTrigger::ON_RECEIVE_HEAL},
        {"ON_ROUND_END", EffectTrigger::ON_ROUND_END},
        {"PASSIVE", EffectTrigger::PASSIVE},
        {"PASSIVE_COND", EffectTrigger::PASSIVE_COND}};

    auto it = map.find(trigger);
    return it != map.end() ? it->second : EffectTrigger::ON_ENTER;
}

EffectAction StringToEffectAction(const std::string &action) {
    static const std::unordered_map<std::string, EffectAction> map = {
        {"HEAL_TARGET", EffectAction::HEAL_TARGET},
        {"BUFF_SELF", EffectAction::BUFF_SELF},
        {"BUFF_SELF_PER_COUNT", EffectAction::BUFF_SELF_PER_COUNT},
        {"BUFF_ALLIES", EffectAction::BUFF_ALLIES},
        {"APPLY_ABILITY_ALLIES", EffectAction::APPLY_ABILITY_ALLIES},
        {"APPLY_STATUS_TO_ENEMY", EffectAction::APPLY_STATUS_TO_ENEMY},
        {"SPAWN_CARD", EffectAction::SPAWN_CARD},
        {"RETREAT_SELF", EffectAction::RETREAT_SELF},
        {"DRAW_CARDS", EffectAction::DRAW_CARDS},
        {"MODIFY_SPELL_COST", EffectAction::MODIFY_SPELL_COST},
        {"NEGATE_DEATH", EffectAction::NEGATE_DEATH},
        {"COPY_STATS", EffectAction::COPY_STATS},
        {"DEAL_BONUS_DAMAGE", EffectAction::DEAL_BONUS_DAMAGE},
        {"DOUBLE_SPELL_EFFECTS", EffectAction::DOUBLE_SPELL_EFFECTS},
        {"EVOLVE_SELF", EffectAction::EVOLVE_SELF}};

    auto it = map.find(action);
    return it != map.end() ? it->second : EffectAction::BUFF_SELF;
}

EffectTarget StringToEffectTarget(const std::string &target) {
    static const std::unordered_map<std::string, EffectTarget> map = {
        {"SELF", EffectTarget::SELF},
        {"ALLY_CHOOSE", EffectTarget::ALLY_CHOOSE},
        {"ALLY_RANDOM", EffectTarget::ALLY_RANDOM},
        {"ALLY_ALL", EffectTarget::ALLY_ALL},
        {"ALLY_RACE", EffectTarget::ALLY_RACE},
        {"ENEMY_CHOOSE", EffectTarget::ENEMY_CHOOSE},
        {"ENEMY_RANDOM", EffectTarget::ENEMY_RANDOM},
        {"ENEMY_ALL", EffectTarget::ENEMY_ALL},
        {"ENEMY_RACE", EffectTarget::ENEMY_RACE},
        {"ENEMY_LEVEL_HIGHER_THAN", EffectTarget::ENEMY_LEVEL_HIGHER_THAN},
        {"ENEMY_LEVEL_LOWER_THAN", EffectTarget::ENEMY_LEVEL_LOWER_THAN},
        {"ANY_CHOOSE", EffectTarget::ANY_CHOOSE},
        {"ALL", EffectTarget::ALL}};

    auto it = map.find(target);
    return it != map.end() ? it->second : EffectTarget::SELF;
}

EffectCondition StringToEffectCondition(const std::string &condition) {
    static const std::unordered_map<std::string, EffectCondition> map = {
        {"NONE", EffectCondition::NONE},
        {"COUNT_ALLIES_RACE_GTE", EffectCondition::COUNT_ALLIES_RACE_GTE},
        {"COUNT_DECAYED_GTE", EffectCondition::COUNT_DECAYED_GTE},
        {"TOTAL_ATTACK_GTE", EffectCondition::TOTAL_ATTACK_GTE},
        {"TIMES_ACTIVATED_LT", EffectCondition::TIMES_ACTIVATED_LT}};

    auto it = map.find(condition);
    return it != map.end() ? it->second : EffectCondition::NONE;
}

Race StringToRace(const std::string &raceStr) {
    static const std::unordered_map<std::string, Race> map = {{"HUMAN", Race::HUMAN},
                                                              {"AUTOMAT", Race::AUTOMAT},
                                                              {"PIXIE", Race::PIXIE},
                                                              {"DRAGON", Race::DRAGON},
                                                              {"DRYAD", Race::DRYAD}};

    auto it = map.find(raceStr);
    return it != map.end() ? it->second : Race::NONE;
}

Rarity StringToRarity(const std::string &rarityType) {
    static const std::unordered_map<std::string, Rarity> map = {{"COMMON", Rarity::COMMON},
                                                                {"UNCOMMON", Rarity::UNCOMMON},
                                                                {"RARE", Rarity::RARE},
                                                                {"EPIC", Rarity::EPIC},
                                                                {"LEGENDARY", Rarity::LEGENDARY}};

    auto it = map.find(rarityType);
    return it != map.end() ? it->second : Rarity::COMMON;
}

CardType StringToCardType(const std::string &cardType) {
    static const std::unordered_map<std::string, CardType> map = {{"CREATURE", CardType::CREATURE},
                                                                  {"SPELL", CardType::SPELL}};

    auto it = map.find(cardType);
    return it != map.end() ? it->second : CardType::SPELL;
}

SpellSpeed StringToSpellSpeed(const std::string &spellType) {
    static const std::unordered_map<std::string, SpellSpeed> map = {{"FAST", SpellSpeed::FAST},
                                                                    {"SLOW", SpellSpeed::SLOW}};

    auto it = map.find(spellType);
    return it != map.end() ? it->second : SpellSpeed::SLOW;
}

Ability StringToAbility(const std::string &abilityStr) {
    static const std::unordered_map<std::string, Ability> map = {
        {"SHIELD", Ability::SHIELD},
        {"TERRIFY", Ability::TERRIFY},
        {"QUICK_ATTACK", Ability::QUICK_ATTACK},
        {"ROT", Ability::ROT},
        {"PIERCING", Ability::PIERCING}};

    auto it = map.find(abilityStr);
    return it != map.end() ? it->second : Ability::NONE;
}

DynamicScaling StringToDynamicScaling(const std::string &scalingStr) {
    static const std::unordered_map<std::string, DynamicScaling> map = {
        {"NONE", DynamicScaling::NONE},
        {"KILLS_COUNT", DynamicScaling::KILLS_COUNT},
        {"DECAYED_COUNT", DynamicScaling::DECAYED_COUNT},
        {"MISSING_HEALTH", DynamicScaling::MISSING_HEALTH}};

    auto it = map.find(scalingStr);
    return it != map.end() ? it->second : DynamicScaling::NONE;
}