#pragma once
#include "../core/GameWorld.hpp"
#include "../core/data/CardDatabase.hpp"
#include "../objects/cards/Card.hpp"
#include <SDL2/SDL_ttf.h>
#include <vector>

class GameManager;

class SceneCollection : public GameWorld {
  private:
    GameManager &gameManager;
    CardDatabase cardDatabase;
    std::vector<Card *> allCards;
    SDL_Renderer *renderer = nullptr;

    TTF_Font *font = nullptr;
    TTF_Font *fontTitle = nullptr;

    SDL_Rect btnBack{};
    int scrollOffset = 0;
    int maxScroll = 0;

    static constexpr int cardWidth = 100;
    static constexpr int cardHeight = 140;
    static constexpr int gapX = 20;
    static constexpr int gapY = 30;
    static constexpr int cols = 12;
    static constexpr int marginTop = 130;
    static constexpr int marginLeft = 40;
    static constexpr int screenW = 1600;
    static constexpr int screenH = 900;

    void BuildCollection();
    void ArrangeGrid();

  public:
    explicit SceneCollection(GameManager &gm);
    ~SceneCollection() override;

    void Initialize(SDL_Renderer *renderer) override;
    void HandleInput(SDL_Event &event) override;
    void Update(float dt) override;
    void Render(SDL_Renderer *renderer) override;
};