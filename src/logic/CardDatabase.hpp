#pragma once

#include "../objects/cards/CardTypes.hpp"
#include <string>
#include <unordered_map>
#include <vector>

struct StageData {
    int level;
    int health;
    int attack;
    std::vector<Ability> abilities;
    std::vector<std::string> effects;
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
    int mana;
    CardType tipo;
    Rarity raridade;
    std::string nome;
    std::string descricao;
    std::vector<std::string> efeitos;
};

class CardDatabase {
  private:
    std::unordered_map<int, CreatureData> creatureCards;
    std::unordered_map<int, SpellData> spellCards;

    Race StringToRace(const std::string &racaStr);
    Rarity StringToRarity(const std::string &raridadeStr);
    CardType StringToSpellType(const std::string &tipoStr);
    Ability StringToAbility(const std::string &abilityStr);

  public:
    CardDatabase() = default;
    ~CardDatabase() = default;
    bool LoadFromJson(const std::string &filepath);

    const CreatureData *GetCreature(const int &id) const;
    const SpellData *GetSpell(const int &id) const;
};