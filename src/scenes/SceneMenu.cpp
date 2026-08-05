#include "SceneMenu.hpp"
#include "../core/GameManager.hpp"
#include "../objects/ui/UIRenderUtils.hpp"
#include "../scenes/SceneBattle.hpp"
#include <SDL2/SDL_ttf.h>
#include <iostream>

SceneMenu::SceneMenu(GameManager &gm) : gameManager(gm) {}

SceneMenu::~SceneMenu() {
    if (background) {
        SDL_DestroyTexture(background);
        background = nullptr;
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

    font = ui::UIRenderUtils::LoadFont("assets/fonts/frozen.ttf", 24);
    fontTitle = ui::UIRenderUtils::LoadFont("assets/fonts/frozen.ttf", 60);
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
        gameManager.ChangeMusic("assets/audio/music/battle_theme.mp3");
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
        ui::UIRenderUtils::RenderText(renderer, title, titleX, titleY, white, fontTitle);
    }

    for (int i = 0; i < (int)buttons.size(); i++) {
        ui::UIRenderUtils::RenderButton(renderer, buttons[i].rect, buttons[i].label, font,
                                        i == hoveredIndex);
    }
}