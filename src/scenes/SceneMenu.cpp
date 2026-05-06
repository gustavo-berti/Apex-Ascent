#include "SceneMenu.hpp"
#include "../core/GameManager.hpp"
#include "../scenes/SceneBattle.hpp"
#include <SDL2/SDL_ttf.h>
#include <iostream>

TTF_Font *font = nullptr;
TTF_Font *fontTitle = nullptr;

SceneMenu::SceneMenu(GameManager &gm) : gameManager(gm) {}

SceneMenu::~SceneMenu() {
    if (background) {
        SDL_DestroyTexture(background);
        background = nullptr;
    }
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (fontTitle) {
        TTF_CloseFont(fontTitle);
        fontTitle = nullptr;
    }
}

void SceneMenu::Initialize(SDL_Renderer *renderer) {
    SDL_Surface *surface = IMG_Load("assets/images/start_menu.png");
    if (!surface) {
        std::cerr << "Erro ao carregar fundo: " << IMG_GetError() << std::endl;
    } else {
        background = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    int w, h;
    SDL_RenderGetLogicalSize(renderer, &w, &h);
    if (w == 0 || h == 0) SDL_GetRendererOutputSize(renderer, &w, &h);

    int btnW = 200;
    int btnH = 50;
    int gap = 30;
    int margin = 40;
    int stackHeight = (btnH * 3) + (gap * 2);

    int startX = w - margin - btnW;
    int startY = h - margin - stackHeight;

    buttons = {
        {{startX, startY, btnW, btnH}, "Começar Jogo"},
        {{startX, startY + gap + btnH, btnW, btnH}, "Coleção"},
        {{startX, startY + (gap + btnH) * 2, btnW, btnH}, "Sair"},
    };

    TTF_Init();
    font = TTF_OpenFont("assets/fonts/frozen.ttf", 24);
    fontTitle = TTF_OpenFont("assets/fonts/frozen.ttf", 60);
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
        battle->Initialize(gameManager.GetRenderer());
        battle->StartBattle(&gameManager.GetPlayer(), &gameManager.GetOpponent(),
                            gameManager.GetRenderer());
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
    int w, h;
    SDL_RenderGetLogicalSize(renderer, &w, &h);
    if (w == 0 || h == 0) SDL_GetRendererOutputSize(renderer, &w, &h);

    if (background) {
        SDL_Rect dst = {0, 0, w, h};
        SDL_RenderCopy(renderer, background, nullptr, &dst);
    } else {
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_RenderClear(renderer);
    }

    if (fontTitle) {
        const std::string title = "Apex Ascent";
        int titleW, titleH;
        TTF_SizeUTF8(fontTitle, title.c_str(), &titleW, &titleH);
        int titleX = (w - titleW) / 2;
        int titleY = 50;
        SDL_Color white = {255, 255, 255, 255};
        RenderText(renderer, title, titleX, titleY, white, fontTitle);
    }

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
        RenderText(renderer, btn.label, textX, textY, white, font);
    }
}

void SceneMenu::RenderText(SDL_Renderer *renderer, const std::string &text, int x, int y,
                           SDL_Color color, TTF_Font *font) const {
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