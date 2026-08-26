#pragma once
#include "Entity.hpp"

// ── Player ────────────────────────────────────────────────────────
struct Player : public Entity {
    int gold = 0;

    Player() { masterDeck = {"1", "38", "28", "24", "21", "1", "28", "1", "38", "23", "23", "23", "23", "50", "50", "50"}; }
};