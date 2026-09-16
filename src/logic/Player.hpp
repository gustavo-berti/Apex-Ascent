#pragma once
#include "Entity.hpp"

// ── Player ────────────────────────────────────────────────────────
struct Player : public Entity {
    int gold = 0;

    // Escolhido no menu, junto com a raça e o nivel do oponente. E o nome que
    // aparece no placar quando a partida acaba.
    static constexpr const char *kDefaultName = "Jogador";
    std::string name = kDefaultName;

    Player() { masterDeck = {"1", "38", "28", "24", "21", "1", "28", "1", "38", "23", "23", "23", "23", "50", "50", "50"}; }
};