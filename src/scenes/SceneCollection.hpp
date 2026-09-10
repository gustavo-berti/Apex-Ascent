#pragma once
#include "../core/GameWorld.hpp"
#include "../core/data/CardDatabase.hpp"
#include "../objects/cards/Card.hpp"
#include <SDL2/SDL_ttf.h>
#include <string>
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
    TTF_Font *fontPanel = nullptr;

    SDL_Rect btnBack{};
    SDL_Rect btnClear{};
    int scrollOffset = 0;
    int maxScroll = 0;

    static constexpr int cardWidth = 100;
    static constexpr int cardHeight = 140;
    static constexpr int gapX = 20;
    static constexpr int gapY = 30;
    static constexpr int cols = 10; // deixa espaco pro painel do deck a direita
    static constexpr int marginTop = 130;
    static constexpr int marginLeft = 40;
    static constexpr int screenW = 1600;
    static constexpr int screenH = 900;

    // Regras do deck (espelham DeckBuilder::kDeckSize / kMaxCopies).
    static constexpr int kDeckTarget = 30;
    static constexpr int kMaxCopies = 3;

    static constexpr int panelX = marginLeft + cols * cardWidth + (cols - 1) * gapX + 40;
    static constexpr int panelW = screenW - panelX - marginLeft;

    void BuildCollection();
    void ArrangeGrid();

    // ── Selecao de deck (persiste em GameManager durante a sessao) ─────
    int GetSelectedCount(int cardId) const;
    int GetTotalSelected() const;
    void AddCopy(int cardId);
    void RemoveCopy(int cardId);
    Card *FindCardAtPoint(int x, int y) const;

    void RenderSelectionBadge(SDL_Renderer *renderer, const Card *card, int count) const;
    void RenderDeckPanel(SDL_Renderer *renderer) const;

  public:
    explicit SceneCollection(GameManager &gm);
    ~SceneCollection() override;

    void Initialize(SDL_Renderer *renderer) override;
    void HandleInput(SDL_Event &event) override;
    void Update(float dt) override;
    void Render(SDL_Renderer *renderer) override;
};
