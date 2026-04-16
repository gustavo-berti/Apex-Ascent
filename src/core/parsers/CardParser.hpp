#pragma once

#include "../data/CardDatabase.hpp"
#include "nlohmann/json.hpp"

CreatureData ParseCreature(const nlohmann::json &creature);
SpellData ParseSpell(const nlohmann::json &spell);
EffectData ParseEffect(const nlohmann::json &effect);