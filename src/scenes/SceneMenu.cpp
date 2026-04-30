#include "SceneMenu.hpp"
#include "../core/GameManager.hpp"
#include "../scenes/SceneBattle.hpp"
#include <iostream>

SceneMenu::SceneMenu(GameManager &gm) : gameManager(gm) {}

void SceneMenu::Initialize() {
    int centerX = 1280 / 2;

    buttons = {
        {{centerX - 100, 250, 200, 50}, "Começar Jogo"},
        {{centerX - 100, 330, 200, 50}, "Coleção"},
        {{centerX - 100, 410, 200, 50}, "Sair"},
    };
}

bool SceneMenu::IsButtonClicked(const MenuButton &btn, const SDL_Event &event) const {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) return false;
    return GameManager::IsPointInsideRect(event.button.x, event.button.y, btn.rect);
}

void SceneMenu::HandleInput(SDL_Event &event) {
    if (IsButtonClicked(buttons[0], event)) {
        SceneBattle *battle = new SceneBattle();
        battle->Initialize();
        battle->StartBattle(&gameManager.GetPlayer(), gameManager.GetRenderer());
        gameManager.ChangeScene(battle);
    }
    if (IsButtonClicked(buttons[1], event)) {
        std::cout << "Coleção clicado" << std::endl;
        // GameManager.ChangeScene(new SceneCollection());
    }
    if (IsButtonClicked(buttons[2], event)) {
        std::cout << "Sair clicado" << std::endl;
        SDL_Event quit;
        quit.type = SDL_QUIT;
        SDL_PushEvent(&quit);
    }
}

void SceneMenu::Update(float dt) {}

void SceneMenu::Render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_RenderClear(renderer);

    for (const auto &btn : buttons)
        RenderButton(renderer, btn);
}

void SceneMenu::RenderButton(SDL_Renderer *renderer, const MenuButton &btn) const {
    SDL_SetRenderDrawColor(renderer, 60, 60, 100, 255);
    SDL_RenderFillRect(renderer, &btn.rect);

    SDL_SetRenderDrawColor(renderer, 180, 180, 255, 255);
    SDL_RenderDrawRect(renderer, &btn.rect);
}