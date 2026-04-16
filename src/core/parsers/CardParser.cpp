#include "CardParser.hpp"
#include "../enums/EnumConverter.hpp"

using json = nlohmann::json;

EffectData ParseEffect(const json &effect) {
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
    }

    return data;
}

CreatureData ParseCreature(const json &creature) {
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

            EffectData effectData = ParseEffect(effect);
            stageData.effects.push_back(effectData);
        }

        data.stages.push_back(stageData);
    }

    return data;
}

SpellData ParseSpell(const json &spell) {
    SpellData data;

    data.id = spell["id"];
    data.name = spell["display_name"];
    data.type = StringToCardType(spell["type"]);
    data.rarity = StringToRarity(spell["rarity"]);
    data.manaCost = spell["mana"];
    data.description = spell["description"];

    for (const auto &effect : spell["effects"]) {
        if (!effect.is_object()) {
            continue;
        }

        EffectData effectData = ParseEffect(effect);
        data.effects.push_back(effectData);
    }

    return data;
}