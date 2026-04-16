#pragma once

#include <string>

enum class Race { HUMAN, AUTOMAT, PIXIE, DRAGON, DRYAD, NONE };

enum class Rarity { COMMON, UNCOMMON, RARE, EPIC, LEGENDARY };

enum class CardType { CREATURE, SPELL };

enum class SpellSpeed { FAST, SLOW };

enum class Ability { SHIELD, TERRIFY, QUICK_ATTACK, ROT, PIERCING, NONE };

inline std::string translateRace(Race r) {
    switch (r) {
    case Race::HUMAN:
        return "Humano";
    case Race::AUTOMAT:
        return "Autômato";
    case Race::PIXIE:
        return "Fada";
    case Race::DRAGON:
        return "Dragão";
    case Race::DRYAD:
        return "Driade";
    case Race::NONE:
        return "Nenhuma";
    }
}

inline std::string translateRarity(Rarity r) {
    switch (r) {
    case Rarity::COMMON:
        return "Comum";
    case Rarity::UNCOMMON:
        return "Incomum";
    case Rarity::RARE:
        return "Rara";
    case Rarity::EPIC:
        return "Épica";
    case Rarity::LEGENDARY:
        return "Lendária";
    }
}

inline std::string translateCardType(CardType ct) {
    switch (ct) {
    case CardType::CREATURE:
        return "Criatura";
    case CardType::SPELL:
        return "Feitiço";
    }
}

inline std::string translateSpellSpeed(SpellSpeed ss) {
    switch (ss) {
    case SpellSpeed::FAST:
        return "Rápida";
    case SpellSpeed::SLOW:
        return "Lenta";
    }
}

inline std::string translateAbility(Ability a) {
    switch (a) {
    case Ability::SHIELD:
        return "Escudo";
    case Ability::TERRIFY:
        return "Aterrorizar";
    case Ability::QUICK_ATTACK:
        return "Ataque Rápido";
    case Ability::ROT:
        return "Apodrecer";
    case Ability::PIERCING:
        return "Perfurar";
    case Ability::NONE:
        return "Nenhuma";
    }
}