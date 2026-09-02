#include "SceneUI.hpp"
#include "../objects/ui/UIRenderUtils.hpp"
#include <SDL2/SDL_image.h>
#include <iostream>

SceneUI::SceneUI(GameManager &manager) : gameManager(manager) {}

SceneUI::~SceneUI() {
    if (background) {
        SDL_DestroyTexture(background);
        background = nullptr;
    }
    // As fontes vem do cache do UIRenderUtils e sobrevivem a cena.
}

void SceneUI::Initialize(SDL_Renderer *renderer) {
    SDL_RenderGetLogicalSize(renderer, &screenWidth, &screenHeight);
    if (screenWidth == 0 || screenHeight == 0)
        SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

    font = ui::UIRenderUtils::LoadFont("assets/fonts/frozen.ttf", 24);
    fontTitle = ui::UIRenderUtils::LoadFont("assets/fonts/frozen.ttf", 60);
}

void SceneUI::LoadBackground(SDL_Renderer *renderer, const std::string &path) {
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Erro ao carregar fundo: " << IMG_GetError() << std::endl;
        return;
    }

    if (background) SDL_DestroyTexture(background);
    background = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void SceneUI::RenderCenteredText(SDL_Renderer *renderer, TTF_Font *f, const std::string &text, int y,
                                 SDL_Color color) const {
    if (!f || text.empty()) return;

    int textW = 0;
    int textH = 0;
    TTF_SizeUTF8(f, text.c_str(), &textW, &textH);
    ui::UIRenderUtils::RenderText(renderer, text, (screenWidth - textW) / 2, y, color, f);
}

void SceneUI::HandleInput(SDL_Event &event) {
    if (event.type == SDL_MOUSEMOTION) {
        hoveredIndex = ui::FindButtonAt(buttons, event.motion.x, event.motion.y);
        return;
    }

    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) return;

    ui::DispatchClick(buttons, event.button.x, event.button.y);
    // O callback pode ter trocado de cena: nada pode tocar em `this` aqui.
}

void SceneUI::Render(SDL_Renderer *renderer) {
    if (background) {
        SDL_Rect dst = {0, 0, screenWidth, screenHeight};
        SDL_RenderCopy(renderer, background, nullptr, &dst);
    } else {
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_RenderClear(renderer);
    }

    RenderCenteredText(renderer, fontTitle, title, kTitleY, {255, 255, 255, 255});
    RenderContent(renderer);
    ui::RenderButtons(renderer, buttons, font, hoveredIndex);
}
