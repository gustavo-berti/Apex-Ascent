#pragma once
#include "../logic/Player.hpp"
#include <string>
#include <vector>

// ── Opponent ──────────────────────────────────────────────────────
// Representa o inimigo em uma batalha.
// Por enquanto o deck é idêntico ao do jogador.
// A flag isGuardian é definida antes de StartBattle pelo sistema de mapa.
//
struct Opponent {
    static constexpr int BASE_HP      = 30;
    static constexpr int GUARDIAN_HP  = 60; // guardião tem o dobro

    int maxHealth     = BASE_HP;
    int currentHealth = BASE_HP;

    bool isGuardian = false;

    ManaState mana; // simulado igualmente ao jogador

    std::vector<std::string> masterDeck;

    // Construtor padrão: não-guardião com deck igual ao do Player
    Opponent() {
        masterDeck = {"1", "38", "28", "24", "21", "1", "28", "1", "38"};
    }

    // Define se é guardião ANTES de iniciar a batalha.
    // Ajusta o HP máximo e atual de acordo.
    void SetGuardian(bool guardian) {
        isGuardian  = guardian;
        maxHealth   = guardian ? GUARDIAN_HP : BASE_HP;
        currentHealth = maxHealth;
    }

    // Recebe dano. Retorna true se morreu.
    bool TakeDamage(int amount) {
        currentHealth -= amount;
        if (currentHealth < 0) currentHealth = 0;
        return currentHealth <= 0;
    }

    bool IsDefeated() const { return currentHealth <= 0; }

    // HP normalizado para barra [0.0, 1.0]
    float GetHealthRatio() const {
        if (maxHealth <= 0) return 0.0f;
        return static_cast<float>(currentHealth) / static_cast<float>(maxHealth);
    }

    // Reset entre combates
    void Reset() {
        currentHealth = maxHealth;
        mana          = ManaState{};
    }

    const std::vector<std::string>& GetMasterDeck() const { return masterDeck; }
};