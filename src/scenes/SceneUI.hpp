#pragma once
#include "../core/GameWorld.hpp"
#include "../objects/ui/UIButton.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

class GameManager;

// Base das telas de menu: cuida do fundo, do titulo, do hover e do clique dos
// botoes. A tela concreta so monta `buttons` (cada um com o seu callback) e,
// se precisar de algo a mais na tela, sobrescreve RenderContent.
class SceneUI : public GameWorld {
  protected:
    static constexpr int kTitleY = 50;

    GameManager &gameManager;
    SDL_Texture *background = nullptr;
    TTF_Font *font = nullptr;      // rotulos e botoes
    TTF_Font *fontTitle = nullptr; // titulo da tela
    std::string title;
    std::vector<ui::UIButton> buttons;
    int hoveredIndex = -1;
    int screenWidth = 0;
    int screenHeight = 0;

    void LoadBackground(SDL_Renderer *renderer, const std::string &path);
    void RenderCenteredText(SDL_Renderer *renderer, TTF_Font *f, const std::string &text, int y,
                            SDL_Color color) const;

    // Desenhado entre o titulo e os botoes.
    virtual void RenderContent(SDL_Renderer *renderer) { (void)renderer; }

  public:
    explicit SceneUI(GameManager &manager);
    ~SceneUI() override;

    void Initialize(SDL_Renderer *renderer) override;
    void HandleInput(SDL_Event &event) override;
    void Update(float dt) override { (void)dt; }
    void Render(SDL_Renderer *renderer) override;
};
