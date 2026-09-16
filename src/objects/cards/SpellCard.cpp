#include "SpellCard.hpp"

SpellCard::SpellCard(const SpellData *data, int x, int y)
    : Card(data->id, data->name, data->manaCost, data->rarity, data->imagePath, x, y),
      dataRef(data) {}

SpellCard::~SpellCard() {}

void SpellCard::Initialize() {}

void SpellCard::Update(float dt) {}

void SpellCard::Render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 150, 50, 200, 255);
    Card::Render(renderer);
    // Additional rendering for effect description can be added here
}

void SpellCard::ActivateEffect() {
    // Implement the logic to activate the spell's effect based on spellType
}

bool SpellCard::isFast() const { return spellType == SpellType::FAST; }