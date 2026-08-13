#include "Card.hpp"
#include "../ui/UIRenderUtils.hpp"
#include <SDL2/SDL_ttf.h>
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
    SDL_Rect rect = {GetX(), GetY(), width, height};

    if (texture) {
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    Color borderColor = GetRarityColor();
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    const int borderThickness = 4;
    for (int i = 0; i < borderThickness; ++i) {
        SDL_Rect borderRect = {x + i, y + i, width - (i * 2), height - (i * 2)};
        SDL_RenderDrawRect(renderer, &borderRect);
    }

    RenderBadge(renderer, manaCost, GetX() + 4, GetY() + 4, Color(20, 60, 170, 255));
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

void Card::RenderBadge(SDL_Renderer *renderer, int value, int badgeX, int badgeY,
                       Color bgColor) const {
    static TTF_Font *badgeFont = ui::UIRenderUtils::LoadFont("./assets/fonts/arial.ttf", 20);
    if (!badgeFont) return;

    constexpr int badgeSize = 22;
    SDL_Rect badge = {badgeX, badgeY, badgeSize, badgeSize};

    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &badge);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &badge);

    std::string text = std::to_string(value);
    int textW = 0, textH = 0;
    TTF_SizeUTF8(badgeFont, text.c_str(), &textW, &textH);

    const int textX = badge.x + (badge.w - textW) / 2;
    const int textY = badge.y + (badge.h - textH) / 2;
    ui::UIRenderUtils::RenderText(renderer, text, textX, textY, {255, 255, 255, 255}, badgeFont);
}