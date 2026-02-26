#include "Card.hpp"

Card::Card(std::string name, CardType type, int manaCost, int x, int y)
    : DynamicObject(x, y, 120, 180), name(name), type(type), manaCost(manaCost) {
}

Card::~Card() {}

void Card::Initialize() {}

void Card::Update(float dt) {}

void Card::Render(SDL_Renderer* renderer) {
    SDL_Rect rect = { x, y, width, height };

    if (type == CardType::CREATURE) {
        SDL_SetRenderDrawColor(renderer, 50, 100, 200, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 150, 50, 200, 255);
    }

    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
}