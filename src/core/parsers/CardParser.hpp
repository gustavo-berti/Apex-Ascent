#pragma once

#include "../data/CardDatabase.hpp"
#include "nlohmann/json.hpp"

class CardParser {
  public:
    static CreatureData ParseCreature(const nlohmann::json &creature);
    static SpellData ParseSpell(const nlohmann::json &spell);
    static EffectData ParseEffect(const nlohmann::json &effect);
    static GenericData ParseGeneric(const nlohmann::json &card);
};