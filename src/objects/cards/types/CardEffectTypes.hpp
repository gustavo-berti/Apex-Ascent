#pragma once
#include "CardTypes.hpp"
#include <string>

enum class EffectTrigger {
    ON_ENTER,
    ON_ATTACK,
    ON_ATTACK_BLOCKED,
    ON_KILL,
    ON_DEATH,
    ON_EVOLUTION,
    ON_SPELL_CAST,
    ON_OPPONENT_PLAY,
    ON_RECEIVE_HEAL,
    ON_ROUND_END,
    PASSIVE,
    PASSIVE_COND,
};

enum class EffectAction {
    HEAL_TARGET,
    BUFF_SELF,
    BUFF_SELF_PER_COUNT,
    BUFF_ALLIES,
    APPLY_ABILITY_ALLIES,
    APPLY_STATUS_TO_ENEMY,
    SPAWN_CARD,
    RETREAT_SELF,
    DRAW_CARDS,
    MODIFY_SPELL_COST,
    NEGATE_DEATH,
    COPY_STATS,
    DEAL_BONUS_DAMAGE,
    DOUBLE_SPELL_EFFECTS,
    EVOLVE_SELF,
};

enum class EffectTarget {
    SELF,
    ALLY_CHOOSE,
    ALLY_RANDOM,
    ALLY_ALL,
    ALLY_RACE,
    ENEMY_CHOOSE,
    ENEMY_RANDOM,
    ENEMY_ALL,
    ENEMY_RACE,
    ENEMY_LEVEL_HIGHER_THAN,
    ENEMY_LEVEL_LOWER_THAN,
    ANY_CHOOSE,
    ALL,
};

enum class EffectCondition {
    NONE,
    COUNT_ALLIES_RACE_GTE,
    COUNT_DECAYED_GTE,
    TOTAL_ATTACK_GTE,
    TIMES_ACTIVATED_LT,
};

struct EffectParams {
    int amount = 0;
    int attack_bonus = 0;
    int health_bonus = 0;
    int spawn_card_id = -1;
    bool retreat_on_trigger = false;
    int max_activations = -1;
    int cost_modifier = 0;
    int count_threshold = 0;
    bool affect_all_enemies = false;
    int draw_count = 1;

    Race race_filter = Race::NONE;
    CardType card_type_filter = CardType::CREATURE;
    Ability ability_to_apply = Ability::NONE;
};

struct EffectData {
    EffectTrigger trigger;
    EffectAction action;
    EffectTarget target = EffectTarget::SELF;
    EffectCondition condition = EffectCondition::NONE;
    EffectParams params;

    int activations_used = 0;
};