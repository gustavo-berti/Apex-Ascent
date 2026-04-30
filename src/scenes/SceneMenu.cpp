#include "SceneMenu.hpp"
#include "../core/GameManager.hpp"
#include "../scenes/SceneBattle.hpp"
#include <SDL2/SDL_ttf.h>
#include <iostream>

TTF_Font *font = nullptr;

SceneMenu::SceneMenu(GameManager &gm) : gameManager(gm) {}

SceneMenu::~SceneMenu() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

void SceneMenu::Initialize() {
    TTF_Init();
    font = TTF_OpenFont("assets/fonts/frozen.ttf", 24);
    if (!font) std::cerr << "Erro ao carregar fonte: " << TTF_GetError() << std::endl;

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
    if (event.type == SDL_MOUSEMOTION) {
        hoveredIndex = -1;
        for (int i = 0; i < (int)buttons.size(); i++) {
            if (GameManager::IsPointInsideRect(event.motion.x, event.motion.y, buttons[i].rect)) {
                hoveredIndex = i;
                break;
            }
        }
    }

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

    for (int i = 0; i < (int)buttons.size(); i++)
        RenderButton(renderer, buttons[i], i == hoveredIndex);
}

void SceneMenu::RenderButton(SDL_Renderer *renderer, const MenuButton &btn, bool hovered) const {
    if (hovered)
        SDL_SetRenderDrawColor(renderer, 100, 100, 180, 255); // mais claro
    else
        SDL_SetRenderDrawColor(renderer, 60, 60, 100, 255);

    SDL_RenderFillRect(renderer, &btn.rect);

    SDL_SetRenderDrawColor(renderer, 180, 180, 255, 255);
    SDL_RenderDrawRect(renderer, &btn.rect);

    if (font) {
        int textW, textH;
        TTF_SizeUTF8(font, btn.label.c_str(), &textW, &textH);
        int textX = btn.rect.x + (btn.rect.w - textW) / 2;
        int textY = btn.rect.y + (btn.rect.h - textH) / 2;

        SDL_Color white = {255, 255, 255, 255};
        RenderText(renderer, btn.label, textX, textY, white);
    }
}

void SceneMenu::RenderText(SDL_Renderer *renderer, const std::string &text, int x, int y,
                           SDL_Color color) const {
    if (!font) return;

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}