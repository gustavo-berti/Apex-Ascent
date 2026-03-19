#pragma once
#include <vector>
#include <string>
struct Player {
    int maxHealth = 30;
    int currentHealth = 30;
    int gold = 0;
    
    std::vector<std::string> masterDeck;

    Player() {
        masterDeck = {
            "1", "1", "1", "1", "1", "1", "1", "1", "1", "1"
        };
    }
    
    // void AddCardToDeck(const std::string& cardId) { masterDeck.push_back(cardId); }
    // void RemoveCardFromDeck(const std::string& cardId) { ... }
};