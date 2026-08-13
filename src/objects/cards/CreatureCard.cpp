#include "CreatureCard.hpp"

CreatureCard::CreatureCard(const CreatureData *data, int x, int y)
    : Card(data->name, data->manaCost, data->rarity, data->imagePath, x, y), dataRef(data) {
    this->stage = 1;
    this->xpPoints = 0;

    this->attack = 0;
    this->health = 0;

    for (const StageData &stageData : dataRef->stages) {
        if (stageData.level == this->stage) {
            this->attack = stageData.attack;
            this->health = stageData.health;
            this->abilities = stageData.abilities;
            return;
        }
    }

    if (!dataRef->stages.empty()) {
        const StageData &fallback = dataRef->stages.front();
        this->attack = fallback.attack;
        this->health = fallback.health;
    }
}

CreatureCard::~CreatureCard() {}

void CreatureCard::Initialize() {}

void CreatureCard::Update(float dt) {}

void CreatureCard::Render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 100, 200, 255);
    Card::Render(renderer);

    const int badgeSize = 22;
    const int margin = 4;

    RenderBadge(renderer, attack, GetX() + margin, GetY() + GetHeight() - badgeSize - margin,
                Color(180, 40, 40, 255));
    RenderBadge(renderer, health, GetX() + GetWidth() - badgeSize - margin,
                GetY() + GetHeight() - badgeSize - margin, Color(40, 150, 70, 255));
}

void CreatureCard::GainXP() {
    // Implement logic to gain XP, e.g., after winning a battle
}

void CreatureCard::LevelUp() {
    // Implement logic to level up the creature, e.g., increase attack and health based on stage
}