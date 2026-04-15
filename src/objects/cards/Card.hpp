#pragma once
#include "../base/DynamicObject.hpp"
#include "./types/CardTypes.hpp"
#include <string>
#include <vector>

class Card : public DynamicObject {
  private:
    std::string name;
    int manaCost;
    std::string description;
    Rarity rarity;
    std::string imagePath;

  public:
    Card(std::string name, int manaCost, Rarity rarity, std::string imagePath, int x, int y);
    virtual ~Card();

    virtual void Initialize() override;
    virtual void Update(float dt) override;
    virtual void Render(SDL_Renderer *renderer) override;
    virtual void ActivateEffect() {};

    void SetPosition(int newX, int newY) {
        x = newX;
        y = newY;
    }

    const std::string &GetName() const { return name; }
};