#pragma once
#include <algorithm>
#include <string>
#include <vector>

// ── Mana ──────────────────────────────────────────────────────────
struct ManaState {
    static constexpr int MAX_MANA = 10;

    int current = 1;
    int total = 1;

    void OnTurnStart(bool gainMana) {
        if (gainMana) {
            total = std::min(total + 1, MAX_MANA);
        }
        current = total;
    }

    bool CanAfford(int cost) const { return current >= cost; }

    bool Spend(int cost) {
        if (!CanAfford(cost)) return false;
        current -= cost;
        return true;
    }
};

// ── Entity ─────────────────────────────────────────────────────────
struct Entity {
    static constexpr int MAX_HAND_SIZE = 10;
    static constexpr int TURN_START_DRAW_COUNT = 1;

    int maxHealth = 30;
    int currentHealth = 30;

    ManaState mana;
    std::vector<std::string> masterDeck;

    Entity() = default;
    virtual ~Entity() = default;

    int GetMaxHandSize() const { return MAX_HAND_SIZE; }
    int GetTurnStartDrawCount() const { return TURN_START_DRAW_COUNT; }
    const std::vector<std::string> &GetMasterDeck() const { return masterDeck; }

    bool IsHandFull(int currentHandSize) const { return currentHandSize >= MAX_HAND_SIZE; }

    int GetAllowedDrawAmount(int currentHandSize, int requestedDraw) const {
        if (requestedDraw <= 0) return 0;
        const int freeSlots = std::max(0, MAX_HAND_SIZE - currentHandSize);
        return std::min(requestedDraw, freeSlots);
    }

    virtual bool TakeDamage(int amount) {
        currentHealth -= amount;
        if (currentHealth < 0) currentHealth = 0;
        return currentHealth <= 0;
    }

    bool IsDefeated() const { return currentHealth <= 0; }

    float GetHealthRatio() const {
        if (maxHealth <= 0) return 0.0f;
        return static_cast<float>(currentHealth) / static_cast<float>(maxHealth);
    }

    virtual void Reset() {
        currentHealth = maxHealth;
        mana = ManaState{};
    }
};