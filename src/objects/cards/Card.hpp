#pragma once
#include "../base/DynamicObject.hpp"
#include "./types/CardTypes.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <map>
#include <string>
#include <vector>

struct Color {
    uint8_t r, g, b, a;
    Color(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
};

class Card : public DynamicObject {
  private:
    std::string name;
    int manaCost;
    std::string description;
    Rarity rarity;
    std::string imagePath;

    SDL_Texture *texture = nullptr;
    SDL_Renderer *renderer = nullptr;

    static const std::map<Rarity, Color> rarityColors;

    Color GetRarityColor() const;

  public:
    Card(std::string name, int manaCost, Rarity rarity, std::string imagePath, int x, int y);
    virtual ~Card();

    virtual void Initialize() override;
    virtual void Update(float dt) override;
    virtual void Render(SDL_Renderer *renderer) override;
    virtual void ActivateEffect() {}

    void LoadTexture(SDL_Renderer *renderer);
    void SetPosition(int newX, int newY) {
        x = newX;
        y = newY;
    }

    const std::string &GetName() const { return name; }
    int GetManaCost() const { return manaCost; }
};