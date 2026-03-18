#include "SpellCard.hpp"

SpellCard::SpellCard(std::string name, int manaCost, Rarity rarity, std::string imagePath, std::string effectDescription, SpellType spellType, int x, int y)
    : Card(name, manaCost, rarity, imagePath, x, y), effectDescription(effectDescription), spellType(spellType) {
}

SpellCard::~SpellCard() {}

void SpellCard::Initialize() {}

void SpellCard::Update(float dt) {}

void SpellCard::Render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 150, 50, 200, 255);
    Card::Render(renderer); 
    // Additional rendering for effect description can be added here
}

void SpellCard::ActivateEffect() {
    // Implement the logic to activate the spell's effect based on spellType
}

bool SpellCard::isFast() const {
    return spellType == SpellType::FAST;
}