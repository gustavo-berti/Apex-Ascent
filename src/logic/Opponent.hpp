#pragma once
#include "../objects/cards/types/CardTypes.hpp"
#include "Entity.hpp"
#include <random>

// ── Opponent ──────────────────────────────────────────────────────
struct Opponent : public Entity {
    bool isGuardian = false;
    Race deckType = Race::NONE;
    int deckPart = 0;

    static int RandomInt(int minValue, int maxValue) {
        if (minValue > maxValue) std::swap(minValue, maxValue);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(minValue, maxValue);
        return dist(gen);
    }

    static int GetDeckSizeForPart(int part, bool guardian) {
        switch (part) {
        case 1:
            return RandomInt(14, 18);
        case 2:
            return RandomInt(22, 26);
        case 3:
            return RandomInt(30, 34);
        case 4:
            return RandomInt(38, 42);
        case 5:
            return RandomInt(46, 50);
        default:
            return 54;
        }
    }

    static std::vector<std::string> BuildDeck(bool guardian, Race type, int part) {
        std::vector<std::string> deck;

        switch (type) {
        case Race::PIXIE:
            deck = {"1",  "1",  "1",  "1",  "1",  "1",  "1",  "1",  "2",  "2",  "2",
                    "2",  "2",  "2",  "2",  "2",  "11", "11", "11", "11", "11", "11",
                    "11", "12", "12", "12", "12", "12", "12", "12", "21", "21", "21",
                    "21", "21", "22", "22", "22", "22", "22", "31", "31", "31", "31",
                    "32", "32", "32", "32", "41", "41", "41", "42", "42", "42"};
            break;
        case Race::AUTOMAT:
            deck = {"7",  "7",  "7",  "7",  "7",  "7",  "7",  "7",  "8",  "8",  "8",
                    "8",  "8",  "8",  "8",  "8",  "17", "17", "17", "17", "17", "17",
                    "17", "18", "18", "18", "18", "18", "18", "18", "27", "27", "27",
                    "27", "27", "28", "28", "28", "28", "28", "37", "37", "37", "37",
                    "38", "38", "38", "38", "47", "47", "47", "48", "48", "48"};
            break;
        case Race::DRAGON:
            deck = {"3",  "3",  "3",  "3",  "3",  "3",  "3",  "3",  "4",  "4",  "4",
                    "4",  "4",  "4",  "4",  "4",  "13", "13", "13", "13", "13", "13",
                    "13", "14", "14", "14", "14", "14", "14", "14", "23", "23", "23",
                    "23", "23", "24", "24", "24", "24", "24", "33", "33", "33", "33",
                    "34", "34", "34", "34", "43", "43", "43", "44", "44", "44"};
            break;
        case Race::HUMAN:
            deck = {"5",  "5",  "5",  "5",  "5",  "5",  "5",  "5",  "6",  "6",  "6",
                    "6",  "6",  "6",  "6",  "6",  "15", "15", "15", "15", "15", "15",
                    "15", "16", "16", "16", "16", "16", "16", "16", "25", "25", "25",
                    "25", "25", "26", "26", "26", "26", "26", "35", "35", "35", "35",
                    "36", "36", "36", "36", "45", "45", "45", "46", "46", "46"};
            break;
        case Race::DRYAD:
            deck = {"9",  "9",  "9",  "9",  "9",  "9",  "9",  "9",  "10", "10", "10",
                    "10", "10", "10", "10", "10", "19", "19", "19", "19", "19", "19",
                    "19", "20", "20", "20", "20", "20", "20", "20", "29", "29", "29",
                    "29", "29", "30", "30", "30", "30", "30", "39", "39", "39", "39",
                    "40", "40", "40", "40", "49", "49", "49", "50", "50", "50"};
            break;
        case Race::NONE:
            deck = {"1",  "1",  "1",  "1",  "1",  "1",  "1",  "1",  "21",
                    "21", "21", "21", "24", "24", "24", "28", "28", "38"};
        default:
            break;
        }

        const int total_cards_per_deck = GetDeckSizeForPart(part, guardian);
        const int limitedSize = std::min(total_cards_per_deck, static_cast<int>(deck.size()));
        deck.resize(limitedSize);
        return deck;
    }

    Opponent() { masterDeck = BuildDeck(false, Race::NONE, 0); }

    void SetGuardian(bool guardian) {
        isGuardian = guardian;
        maxHealth = 30;
        if (isGuardian) {
            maxHealth *= 2;
        }

        currentHealth = maxHealth;
        masterDeck = BuildDeck(isGuardian, deckType, deckPart);
    }

    void SetDeck(Race type, int part) {
        deckType = type;
        deckPart = part;
        masterDeck = BuildDeck(isGuardian, deckType, deckPart);
    }
};