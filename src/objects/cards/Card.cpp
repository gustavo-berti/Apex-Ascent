#include "Card.hpp"

Card::Card(std::string name, int manaCost, Rarity rarity, std::string imagePath, int x, int y)
    : DynamicObject(x, y, 100, 140), name(name), manaCost(manaCost), rarity(rarity),
      imagePath(imagePath) {}

Card::~Card() {}

void Card::Initialize() {}

void Card::Update(float dt) {}

void Card::Render(SDL_Renderer *renderer) {
    SDL_Rect rect = {x, y, width, height};

    if (rarity == Rarity::COMMON) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Gray
    } else if (rarity == Rarity::UNCOMMON) {
        SDL_SetRenderDrawColor(renderer, 50, 150, 50, 255); // Green
    } else if (rarity == Rarity::RARE) {
        SDL_SetRenderDrawColor(renderer, 50, 50, 150, 255); // Blue
    } else if (rarity == Rarity::EPIC) {
        SDL_SetRenderDrawColor(renderer, 150, 50, 150, 255); // Purple
    } else if (rarity == Rarity::LEGENDARY) {
        SDL_SetRenderDrawColor(renderer, 200, 150, 50, 255); // Orange
    }
    SDL_RenderFillRect(renderer, &rect);

    int padding = 4;
    SDL_Rect innerRect = {x + padding, y + padding, width - padding * 2, height - padding * 2};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &innerRect);
}