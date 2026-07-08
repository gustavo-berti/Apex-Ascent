#include "CardParser.hpp"
#include "../enums/EnumConverter.hpp"

using json = nlohmann::json;

EffectData CardParser::ParseEffect(const json &effect) {
    EffectData data;

    data.trigger = StringToEffectTrigger(effect.value("trigger", std::string("ON_ENTER")));
    data.action = StringToEffectAction(effect.value("action", std::string("BUFF_SELF")));
    data.target = StringToEffectTarget(effect.value("target", std::string("SELF")));
    data.condition = StringToEffectCondition(effect.value("condition", std::string("NONE")));

    if (effect.contains("params") && effect["params"].is_object()) {
        const auto &params = effect["params"];
        data.params.amount = params.value("amount", 0);
        data.params.attack_bonus = params.value("attack_bonus", 0);
        data.params.health_bonus = params.value("health_bonus", 0);
        data.params.spawn_card_id = params.value("spawn_card_id", -1);
        data.params.retreat_on_trigger = params.value("retreat_on_trigger", false);
        data.params.max_activations = params.value("max_activations", -1);
        data.params.cost_modifier = params.value("cost_modifier", 0);
        data.params.count_threshold = params.value("count_threshold", 0);
        data.params.affect_all_enemies = params.value("affect_all_enemies", false);
        data.params.draw_count = params.value("draw_count", 1);
        data.params.race_filter = StringToRace(params.value("race_filter", std::string("NONE")));
        data.params.card_type_filter =
            StringToCardType(params.value("card_type_filter", std::string("CREATURE")));
        data.params.ability_to_apply =
            StringToAbility(params.value("ability_to_apply", std::string("NONE")));
        data.params.scaling = StringToDynamicScaling(params.value("scaling", std::string("NONE")));
    }

    return data;
}

GenericData CardParser::ParseGeneric(const json &card) {
    GenericData data;

    data.id = card.value("id", -1);
    data.name = card.value("display_name", std::string("Unnamed Card"));
    data.description = card.value("description", std::string(""));
    data.rarity = StringToRarity(card.value("rarity", std::string("COMMON")));
    data.manaCost = card.value("mana", 0);
    data.imagePath = card.value("image_path", "");

    return data;
}

CreatureData CardParser::ParseCreature(const json &creature) {
    CreatureData data;

    GenericData generic = ParseGeneric(creature);
    static_cast<GenericData &>(data) = generic;

    data.race = StringToRace(creature.value("race", std::string("NONE")));

    if (!creature.contains("stages") || !creature["stages"].is_array()) {
        return data;
    }

    for (const auto &stage : creature["stages"]) {
        StageData stageData;
        stageData.level = stage.value("level", 0);
        stageData.health = stage.value("health", 0);
        stageData.attack = stage.value("attack", 0);

        if (stage.contains("abilities") && stage["abilities"].is_array()) {
            for (const auto &ability : stage["abilities"]) {
                stageData.abilities.push_back(StringToAbility(ability));
            }
        }

        if (stage.contains("effects") && stage["effects"].is_array()) {
            for (const auto &effect : stage["effects"]) {
                if (!effect.is_object()) {
                    continue;
                }
                stageData.effects.push_back(ParseEffect(effect));
            }
        }

        data.stages.push_back(stageData);
    }

    return data;
}

SpellData CardParser::ParseSpell(const json &spell) {
    SpellData data;

    GenericData generic = ParseGeneric(spell);
    static_cast<GenericData &>(data) = generic;

    data.type = StringToCardType(spell.value("type", std::string("SPELL")));

    if (!spell.contains("effects") || !spell["effects"].is_array()) {
        return data;
    }

    for (const auto &effect : spell["effects"]) {
        if (!effect.is_object()) {
            continue;
        }
        data.effects.push_back(ParseEffect(effect));
    }

    return data;
}