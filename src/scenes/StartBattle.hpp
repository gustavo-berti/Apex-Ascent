#pragma once
#include "../core/GameWorld.hpp"
#include "../objects/cards/Card.hpp"
#include <string>
#include <vector>

class SceneBattle : public GameWorld {
  private:
    SDL_Rect playerHandZone;

    std::vector<Card *> drawPile;
    std::vector<Card *> hand;
    std::vector<Card *> discardPile;

  public:
    SceneBattle();
    ~SceneBattle() override = default;

    void initialize() override;
    void StartBattle(const std::vector<std::string> &masterDeck);
    void DrawCards(int amount);
};