#pragma once
#include "../core/GameWorld.hpp"
#include <SDL2/SDL.h>
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
    std::vector<MenuButton> buttons;
    int hoveredIndex = -1;

    bool IsButtonClicked(const MenuButton &btn, const SDL_Event &event) const;
    void RenderButton(SDL_Renderer *renderer, const MenuButton &btn, bool hovered) const;
    void RenderText(SDL_Renderer *renderer, const std::string &text, int x, int y,
                    SDL_Color color) const;

  public:
    SceneMenu(GameManager &manager);
    ~SceneMenu() override;

    void Initialize() override;
    void HandleInput(SDL_Event &event) override;
    void Update(float dt) override;
    void Render(SDL_Renderer *renderer) override;
};