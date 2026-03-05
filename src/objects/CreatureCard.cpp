#include "CreatureCard.hpp"

CreatureCard::CreatureCard(std::string name, int manaCost, Rarity rarity, std::string imagePath, int attack, 
    int health, std::string effectDescription, int xpPoints, int x, int y)
    : Card(name, manaCost, rarity, imagePath, x, y), attack(attack), health(health), effectDescription(effectDescription), 
    xpPoints(xpPoints) {
}

CreatureCard::~CreatureCard() {}

void CreatureCard::Initialize() {}

void CreatureCard::Update(float dt) {}

void CreatureCard::Render(SDL_Renderer* renderer) {
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