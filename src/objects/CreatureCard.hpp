#pragma once
#include "./Card.hpp"

class CreatureCard : public Card {
  private:
    int attack;
    int health;
    std::string effectDescription;
    int xpPoints;
    int stage = 1;

  public:
    CreatureCard(std::string name, int manaCost, Rarity rarity, std::string imagePath, int attack,
                 int health, std::string effectDescription, int xpPoints, int x, int y);
    virtual ~CreatureCard();

    virtual void Initialize() override;
    virtual void Update(float dt) override;
    virtual void Render(SDL_Renderer *renderer) override;
    void GainXP();
    void LevelUp();
};