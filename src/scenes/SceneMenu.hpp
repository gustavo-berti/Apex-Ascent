#pragma once
#include "../core/GameWorld.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

class GameManager;

struct MenuButton {
    SDL_Rect rect;
    std::string label;
};

class SceneMenu : public GameWorld {
  private:
    GameManager &gameManager;
    SDL_Texture *background = nullptr;
    TTF_Font *font = nullptr;
    TTF_Font *fontTitle = nullptr;
    std::vector<MenuButton> buttons;
    int hoveredIndex = -1;

    bool IsButtonClicked(const MenuButton &btn, const SDL_Event &event) const;
    void RenderButton(SDL_Renderer *renderer, const MenuButton &btn, bool hovered) const;
    void RenderText(SDL_Renderer *renderer, const std::string &text, int x, int y, SDL_Color color,
                    TTF_Font *font) const;

  public:
    SceneMenu(GameManager &manager);
    ~SceneMenu() override;

    void Initialize(SDL_Renderer *renderer) override;
    void HandleInput(SDL_Event &event) override;
    void Update(float dt) override;
    void Render(SDL_Renderer *renderer) override;
};