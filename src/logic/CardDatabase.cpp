#include "CardDatabase.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace {
EffectTrigger StringToEffectTrigger(const std::string &trigger) {
    if (trigger == "ON_ENTER") return EffectTrigger::ON_ENTER;
    if (trigger == "ON_ATTACK") return EffectTrigger::ON_ATTACK;
    if (trigger == "ON_ATTACK_BLOCKED") return EffectTrigger::ON_ATTACK_BLOCKED;
    if (trigger == "ON_KILL") return EffectTrigger::ON_KILL;
    if (trigger == "ON_DEATH") return EffectTrigger::ON_DEATH;
    if (trigger == "ON_EVOLUTION") return EffectTrigger::ON_EVOLUTION;
    if (trigger == "ON_SPELL_CAST") return EffectTrigger::ON_SPELL_CAST;
    if (trigger == "ON_OPPONENT_PLAY") return EffectTrigger::ON_OPPONENT_PLAY;
    if (trigger == "ON_RECEIVE_HEAL") return EffectTrigger::ON_RECEIVE_HEAL;
    if (trigger == "ON_ROUND_END") return EffectTrigger::ON_ROUND_END;
    if (trigger == "PASSIVE") return EffectTrigger::PASSIVE;
    if (trigger == "PASSIVE_COND") return EffectTrigger::PASSIVE_COND;
    return EffectTrigger::ON_ENTER;
}

EffectAction StringToEffectAction(const std::string &action) {
    if (action == "HEAL_TARGET") return EffectAction::HEAL_TARGET;
    if (action == "BUFF_SELF") return EffectAction::BUFF_SELF;
    if (action == "BUFF_SELF_PER_COUNT") return EffectAction::BUFF_SELF_PER_COUNT;
    if (action == "BUFF_ALLIES") return EffectAction::BUFF_ALLIES;
    if (action == "APPLY_ABILITY_ALLIES") return EffectAction::APPLY_ABILITY_ALLIES;
    if (action == "APPLY_STATUS_TO_ENEMY") return EffectAction::APPLY_STATUS_TO_ENEMY;
    if (action == "SPAWN_CARD") return EffectAction::SPAWN_CARD;
    if (action == "RETREAT_SELF") return EffectAction::RETREAT_SELF;
    if (action == "DRAW_CARDS") return EffectAction::DRAW_CARDS;
    if (action == "MODIFY_SPELL_COST") return EffectAction::MODIFY_SPELL_COST;
    if (action == "NEGATE_DEATH") return EffectAction::NEGATE_DEATH;
    if (action == "COPY_STATS") return EffectAction::COPY_STATS;
    if (action == "DEAL_BONUS_DAMAGE") return EffectAction::DEAL_BONUS_DAMAGE;
    if (action == "DOUBLE_SPELL_EFFECTS") return EffectAction::DOUBLE_SPELL_EFFECTS;
    if (action == "EVOLVE_SELF") return EffectAction::EVOLVE_SELF;
    return EffectAction::BUFF_SELF;
}

EffectTarget StringToEffectTarget(const std::string &target) {
    if (target == "SELF") return EffectTarget::SELF;
    if (target == "ALLY_CHOOSE") return EffectTarget::ALLY_CHOOSE;
    if (target == "ALLY_RANDOM") return EffectTarget::ALLY_RANDOM;
    if (target == "ALLY_ALL") return EffectTarget::ALLY_ALL;
    if (target == "ALLY_RACE") return EffectTarget::ALLY_RACE;
    if (target == "ENEMY_CHOOSE") return EffectTarget::ENEMY_CHOOSE;
    if (target == "ENEMY_RANDOM") return EffectTarget::ENEMY_RANDOM;
    if (target == "ENEMY_ALL") return EffectTarget::ENEMY_ALL;
    if (target == "ENEMY_RACE") return EffectTarget::ENEMY_RACE;
    if (target == "ENEMY_LEVEL_HIGHER_THAN") return EffectTarget::ENEMY_LEVEL_HIGHER_THAN;
    if (target == "ENEMY_LEVEL_LOWER_THAN") return EffectTarget::ENEMY_LEVEL_LOWER_THAN;
    if (target == "ANY_CHOOSE") return EffectTarget::ANY_CHOOSE;
    if (target == "ALL") return EffectTarget::ALL;
    return EffectTarget::SELF;
}

EffectCondition StringToEffectCondition(const std::string &condition) {
    if (condition == "NONE") return EffectCondition::NONE;
    if (condition == "COUNT_ALLIES_RACE_GTE") return EffectCondition::COUNT_ALLIES_RACE_GTE;
    if (condition == "COUNT_DECAYED_GTE") return EffectCondition::COUNT_DECAYED_GTE;
    if (condition == "TOTAL_ATTACK_GTE") return EffectCondition::TOTAL_ATTACK_GTE;
    if (condition == "TIMES_ACTIVATED_LT") return EffectCondition::TIMES_ACTIVATED_LT;
    return EffectCondition::NONE;
}

Race StringToRace(const std::string &raceStr) {
    if (raceStr == "HUMAN")
        return Race::HUMAN;
    else if (raceStr == "AUTOMAT")
        return Race::AUTOMAT;
    else if (raceStr == "PIXIE")
        return Race::PIXIE;
    else if (raceStr == "DRAGON")
        return Race::DRAGON;
    else if (raceStr == "DRYAD")
        return Race::DRYAD;
    return Race::NONE;
}

Rarity StringToRarity(const std::string &rarityType) {
    if (rarityType == "COMMON")
        return Rarity::COMMON;
    else if (rarityType == "UNCOMMON")
        return Rarity::UNCOMMON;
    else if (rarityType == "RARE")
        return Rarity::RARE;
    else if (rarityType == "EPIC")
        return Rarity::EPIC;
    else if (rarityType == "LEGENDARY")
        return Rarity::LEGENDARY;
    return Rarity::COMMON;
}

CardType StringToSpellType(const std::string &cardType) {
    if (cardType == "CREATURE")
        return CardType::CREATURE;
    else if (cardType == "SPELL")
        return CardType::SPELL;
    return CardType::SPELL;
}

Ability StringToAbility(const std::string &abilityStr) {
    if (abilityStr == "SHIELD")
        return Ability::SHIELD;
    else if (abilityStr == "TERRIFY")
        return Ability::TERRIFY;
    else if (abilityStr == "QUICK_ATTACK")
        return Ability::QUICK_ATTACK;
    else if (abilityStr == "ROT")
        return Ability::ROT;
    return Ability::NONE;
}
} // namespace

bool CardDatabase::LoadFromJson(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open card data file: " << filepath << std::endl;
        return false;
    }

    json j;
    file >> j;

    for (const auto &creature : j["creatures"]) {
        CreatureData data;
        data.id = creature["id"];
        data.name = creature["display_name"];
        data.description = creature["description"];
        data.race = StringToRace(creature["race"]);
        data.rarity = StringToRarity(creature["rarity"]);
        data.manaCost = creature["mana"];

        for (const auto &stage : creature["stages"]) {
            StageData stageData;
            stageData.level = stage["level"];
            stageData.health = stage["health"];
            stageData.attack = stage["attack"];
            for (const auto &ability : stage["abilities"])
                stageData.abilities.push_back(StringToAbility(ability));
            for (const auto &effect : stage["effects"]) {
                if (!effect.is_object()) {
                    continue;
                }

                EffectData effectData;
                effectData.trigger =
                    StringToEffectTrigger(effect.value("trigger", std::string("ON_ENTER")));
                effectData.action =
                    StringToEffectAction(effect.value("action", std::string("BUFF_SELF")));
                effectData.target =
                    StringToEffectTarget(effect.value("target", std::string("SELF")));
                effectData.condition =
                    StringToEffectCondition(effect.value("condition", std::string("NONE")));

                if (effect.contains("params") && effect["params"].is_object()) {
                    const auto &params = effect["params"];
                    effectData.params.amount = params.value("amount", 0);
                    effectData.params.attack_bonus = params.value("attack_bonus", 0);
                    effectData.params.health_bonus = params.value("health_bonus", 0);
                    effectData.params.spawn_card_id = params.value("spawn_card_id", -1);
                    effectData.params.retreat_on_trigger =
                        params.value("retreat_on_trigger", false);
                    effectData.params.max_activations = params.value("max_activations", -1);
                    effectData.params.cost_modifier = params.value("cost_modifier", 0);
                    effectData.params.count_threshold = params.value("count_threshold", 0);
                    effectData.params.affect_all_enemies =
                        params.value("affect_all_enemies", false);
                    effectData.params.draw_count = params.value("draw_count", 1);
                }

                stageData.effects.push_back(effectData);
            }
            data.stages.push_back(stageData);
        }

        creatureCards[data.id] = data;
        std::cout << "Base de dados carregada! Criaturas na memoria: " << creatureCards.size()
                  << std::endl;
    }

    for (const auto &spell : j["spells"]) {
        SpellData data;
        data.id = spell["id"];
        data.name = spell["display_name"];
        data.type = StringToSpellType(spell["type"]);
        data.rarity = StringToRarity(spell["rarity"]);
        data.manaCost = spell["mana"];
        data.description = spell["description"];
        for (const auto &effect : spell["effects"])
            data.effects.push_back(effect);

        spellCards[data.id] = data;
        std::cout << "Base de dados carregada! Feitiços na memoria: " << spellCards.size()
                  << std::endl;
    }

    return true;
}

const CreatureData *CardDatabase::GetCreature(const int &id) const {
    auto it = creatureCards.find(id);
    if (it != creatureCards.end()) return &it->second;
    return nullptr;
}

const SpellData *CardDatabase::GetSpell(const int &id) const {
    auto it = spellCards.find(id);
    if (it != spellCards.end()) return &it->second;
    return nullptr;
}