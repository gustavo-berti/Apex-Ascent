#pragma once
#include "../logic/CardDatabase.hpp"
#include "./Card.hpp"

class CreatureCard : public Card {
  private:
    const CreatureData *dataRef;

    int attack;
    int health;
    int xpPoints;
    int stage = 1;
    std::vector<Ability> abilities;

  public:
    CreatureCard(const CreatureData *data, int x, int y);
    virtual ~CreatureCard();

    virtual void Initialize() override;
    virtual void Update(float dt) override;
    virtual void Render(SDL_Renderer *renderer) override;
    void GainXP();
    void LevelUp();
    int GetAttack() const { return attack; }
    int GetHealth() const { return health; }

    bool hasAbility(Ability ability) const {
        for (const auto &a : abilities) {
            if (a == ability) {
                return true;
            }
        }
        return false;
    }
};