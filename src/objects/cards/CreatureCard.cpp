#include "CreatureCard.hpp"

CreatureCard::CreatureCard(const CreatureData *data, int x, int y)
    : Card(data->name, data->manaCost, data->rarity, "", x, y), dataRef(data) {
    this->stage = 1;
    this->xpPoints = 0;

    if (!dataRef->stages.empty()) {
        const StageData &statusIniciais = dataRef->stages[0];

        this->attack = statusIniciais.attack;
        this->health = statusIniciais.health;
    } else {
        this->attack = 0;
        this->health = 0;
    }
}

CreatureCard::~CreatureCard() {}

void CreatureCard::Initialize() {}

void CreatureCard::Update(float dt) {}

void CreatureCard::Render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 100, 200, 255);
    Card::Render(renderer);
    // Additional rendering for attack, health, and effect description can be added here
}

void CreatureCard::GainXP() {
    // Implement logic to gain XP, e.g., after winning a battle
}

void CreatureCard::LevelUp() {
    // Implement logic to level up the creature, e.g., increase attack and health based on stage
}