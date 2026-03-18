#include "CardDatabase.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Race CardDatabase::StringToRace(const std::string &raceStr) {
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

Rarity CardDatabase::StringToRarity(const std::string &rarityType) {
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

CardType CardDatabase::StringToSpellType(const std::string &cardType) {
    if (cardType == "CREATURE")
        return CardType::CREATURE;
    else if (cardType == "SPELL")
        return CardType::SPELL;
    return CardType::SPELL;
}

Ability CardDatabase::StringToAbility(const std::string &abilityStr) {
    if (abilityStr == "SHIELD")
        return Ability::SHIELD;
    else if (abilityStr == "TERRIFY")
        return Ability::TERRIFY;
    else if (abilityStr == "QUICK_ATTACK")
        return Ability::QUICK_ATTACK;
    else if (abilityStr == "DECAY")
        return Ability::DECAY;
    return Ability::NONE;
}

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
            for (const auto &effect : stage["effects"])
                stageData.effects.push_back(effect);
            data.stages.push_back(stageData);
        }

        creatureCards[data.id] = data;
        std::cout << "Base de dados carregada! Criaturas na memoria: " << creatureCards.size()
                  << std::endl;
    }

    for (const auto &spell : j["spells"]) {
        SpellData data;
        data.id = spell["id"];
        data.nome = spell["display_name"];
        data.tipo = StringToSpellType(spell["type"]);
        data.raridade = StringToRarity(spell["rarity"]);
        data.mana = spell["mana"];
        data.descricao = spell["description"];
        for (const auto &efeito : spell["effects"])
            data.efeitos.push_back(efeito);

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