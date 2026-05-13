#include "Card.hpp"
#include <iostream>

const std::map<Rarity, Color> Card::rarityColors = {
    {Rarity::COMMON, Color(100, 100, 100, 255)},  // Gray
    {Rarity::UNCOMMON, Color(50, 150, 50, 255)},  // Green
    {Rarity::RARE, Color(50, 50, 150, 255)},      // Blue
    {Rarity::EPIC, Color(150, 50, 150, 255)},     // Purple
    {Rarity::LEGENDARY, Color(200, 150, 50, 255)} // Orange
};

Color Card::GetRarityColor() const {
    auto it = rarityColors.find(rarity);
    if (it != rarityColors.end()) {
        return it->second;
    }

    std::cerr << "Rarity invalida em Card::GetRarityColor: " << static_cast<int>(rarity)
              << std::endl;
    return Color(100, 100, 100, 255);
}

Card::Card(std::string name, int manaCost, Rarity rarity, std::string imagePath, int x, int y)
    : DynamicObject(x, y, 100, 140), name(name), manaCost(manaCost), rarity(rarity),
      imagePath(imagePath) {}

Card::~Card() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void Card::Initialize() {}

void Card::Update(float dt) {}

void Card::Render(SDL_Renderer *renderer) {
    SDL_Rect rect = {x, y, width, height};

    if (texture) {
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    Color borderColor = GetRarityColor();
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b,
                           borderColor.a);

    const int borderThickness = 4;
    for (int i = 0; i < borderThickness; ++i) {
        SDL_Rect borderRect = {x + i, y + i, width - (i * 2), height - (i * 2)};
        SDL_RenderDrawRect(renderer, &borderRect);
    }
}

void Card::LoadTexture(SDL_Renderer *renderer) {
    if (imagePath.empty()) return;

    SDL_Surface *surface = IMG_Load(imagePath.c_str());
    if (!surface) {
        std::cerr << "Erro ao carregar imagem: " << imagePath << " — " << IMG_GetError()
                  << std::endl;
        return;
    }

    this->texture = SDL_CreateTextureFromSurface(renderer, surface);
    this->renderer = renderer;
    SDL_FreeSurface(surface);
}