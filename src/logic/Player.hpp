#pragma once
#include <string>
#include <vector>
struct Player {
    int maxHealth = 30;
    int currentHealth = 30;
    int gold = 0;

    std::vector<std::string> masterDeck;

    Player() { masterDeck = {"23", "1", "38", "28", "24", "21", "1", "28", "1", "38"}; }

    // void AddCardToDeck(const std::string& cardId) { masterDeck.push_back(cardId); }
    // void RemoveCardFromDeck(const std::string& cardId) { ... }
};