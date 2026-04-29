#include "CardDatabase.hpp"
#include "../enums/EnumConverter.hpp"
#include "../parsers/CardParser.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json; // namespace

bool CardDatabase::LoadFromJson(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open card data file: " << filepath << std::endl;
        return false;
    }

    json j;
    file >> j;

    for (const auto &creature : j["creatures"]) {
        CreatureData data = CardParser::ParseCreature(creature);
        creatureCards[data.id] = data;
    }

    for (const auto &spell : j["spells"]) {
        SpellData data = CardParser::ParseSpell(spell);
        spellCards[data.id] = data;
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