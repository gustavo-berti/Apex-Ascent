#include "ScenePause.hpp"
#include "../core/GameManager.hpp"
#include "SceneMenu.hpp"
#include <iostream>

ScenePause::ScenePause(GameManager &manager, std::function<void()> resumeCallback)
    : SceneUI(manager), onResume(std::move(resumeCallback)) {}

void ScenePause::Initialize(SDL_Renderer *renderer) {
    SceneUI::Initialize(renderer);
    // Sem LoadBackground: o fundo desta tela e a propria batalha, escurecida.
    title = "Pausa";
    ShowMainScreen();
}

// ── Tela principal ────────────────────────────────────────────────

void ScenePause::ShowMainScreen() {
    confirmingExit = false;
    hoveredIndex = -1;
    buttons.clear();

    const int btnW = 320, btnH = 60, gap = 25;
    const int stackHeight = btnH * 3 + gap * 2;
    const int x = (screenWidth - btnW) / 2;
    const int y = (screenHeight - stackHeight) / 2;

    buttons.push_back({{x, y, btnW, btnH},
                       "Continuar",
                       [this] {
                           // Devolve o controle para a batalha, que destroi esta
                           // cena: nada pode tocar em `this` depois daqui.
                           if (onResume) onResume();
                       },
                       ui::styles::kPrimary});

    buttons.push_back({{x, y + gap + btnH, btnW, btnH}, "Opções",
                       [] { std::cout << "[PAUSA] Opções clicado" << std::endl; }});

    buttons.push_back({{x, y + (gap + btnH) * 2, btnW, btnH},
                       "Voltar ao menu",
                       [this] { ShowExitConfirmation(); },
                       ui::styles::kSecondary});
}

// ── Confirmação de saída ──────────────────────────────────────────

void ScenePause::ShowExitConfirmation() {
    confirmingExit = true;
    hoveredIndex = -1;
    buttons.clear();

    const int btnW = 260, btnH = 60, gap = 40;
    const int rowW = btnW * 2 + gap;
    const int x = (screenWidth - rowW) / 2;
    const int y = screenHeight / 2;

    buttons.push_back({{x, y, btnW, btnH}, "Sim, sair", [this] { ReturnToMenu(); },
                       ui::styles::kPrimary});
    buttons.push_back({{x + btnW + gap, y, btnW, btnH}, "Cancelar", [this] { ShowMainScreen(); },
                       ui::styles::kSecondary});
}

void ScenePause::ReturnToMenu() {
    std::cout << "[PAUSA] Abandonando a partida e voltando ao menu." << std::endl;

    SceneMenu *menu = new SceneMenu(gameManager);
    menu->Initialize(gameManager.GetRenderer());
    gameManager.ChangeMusic("assets/audio/music/menu_theme.mp3");
    gameManager.ChangeScene(menu);
}

// ── Render ────────────────────────────────────────────────────────

void ScenePause::RenderBackground(SDL_Renderer *renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 190);
    SDL_Rect full = {0, 0, screenWidth, screenHeight};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void ScenePause::RenderContent(SDL_Renderer *renderer) {
    if (!confirmingExit) return;

    const SDL_Color warning = {255, 220, 80, 255};
    RenderCenteredText(renderer, font, "Voltar ao menu?", buttons[0].rect.y - 110, warning);
    RenderCenteredText(renderer, font, "O progresso desta partida será perdido.",
                       buttons[0].rect.y - 60, {255, 255, 255, 255});
}
