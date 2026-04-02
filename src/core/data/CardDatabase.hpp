#pragma once

#include "../../objects/cards/types/CardEffectTypes.hpp"
#include "../../objects/cards/types/CardTypes.hpp"
#include <string>
#include <unordered_map>
#include <vector>

struct StageData {
    int level;
    int health;
    int attack;
    std::vector<Ability> abilities;
    std::vector<EffectData> effects;
};

struct CreatureData {
    int id;
    int manaCost;
    Race race;
    Rarity rarity;
    std::string name;
    std::string description;
    std::vector<StageData> stages;
};

struct SpellData {
    int id;
    int manaCost;
    CardType type;
    Rarity rarity;
    std::string name;
    std::string description;
    std::vector<EffectData> effects;
};

class CardDatabase {
  private:
    std::unordered_map<int, CreatureData> creatureCards;
    std::unordered_map<int, SpellData> spellCards;

  public:
    CardDatabase() = default;
    ~CardDatabase() = default;
    bool LoadFromJson(const std::string &filepath);

    const CreatureData *GetCreature(const int &id) const;
    const SpellData *GetSpell(const int &id) const;
};