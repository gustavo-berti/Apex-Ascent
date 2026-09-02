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

    BuildOpponentSetupButtons(w, h);

    font = ui::UIRenderUtils::LoadFont("assets/fonts/frozen.ttf", 24);
    fontTitle = ui::UIRenderUtils::LoadFont("assets/fonts/frozen.ttf", 60);
}

// ── Botões da escolha do oponente ─────────────────────────────────
// Duas fileiras centralizadas (raça e nível) e as ações embaixo.

void SceneMenu::BuildOpponentSetupButtons(int w, int h) {
    setupButtons.clear();

    const int raceW = 210, raceH = 50, raceGap = 20;
    const int raceRowW = kRaceCount * raceW + (kRaceCount - 1) * raceGap;
    const int raceX = (w - raceRowW) / 2;
    const int raceY = h / 2 - 150;

    for (int i = 0; i < kRaceCount; ++i)
        setupButtons.push_back(
            {{raceX + i * (raceW + raceGap), raceY, raceW, raceH}, translateRace(kRaces[i])});

    const int lvlW = 100, lvlH = 50, lvlGap = 20;
    const int lvlRowW = kLevelCount * lvlW + (kLevelCount - 1) * lvlGap;
    const int lvlX = (w - lvlRowW) / 2;
    const int lvlY = h / 2;

    for (int i = 0; i < kLevelCount; ++i)
        setupButtons.push_back(
            {{lvlX + i * (lvlW + lvlGap), lvlY, lvlW, lvlH}, std::to_string(i + 1)});

    const int actW = 200, actH = 50, actGap = 40;
    const int actY = h / 2 + 150;
    setupButtons.push_back({{w / 2 - actW - actGap / 2, actY, actW, actH}, "Lutar"});
    setupButtons.push_back({{w / 2 + actGap / 2, actY, actW, actH}, "Voltar"});
}

bool SceneMenu::IsButtonClicked(const MenuButton &btn, const SDL_Event &event) const {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) return false;
    return GameManager::IsPointInsideRect(event.button.x, event.button.y, btn.rect);
}

const std::vector<MenuButton> &SceneMenu::ActiveButtons() const {
    return screen == MenuScreen::MAIN ? buttons : setupButtons;
}

// ═══════════════════════════════════════════════════════════════════
//  Input
// ═══════════════════════════════════════════════════════════════════

void SceneMenu::HandleInput(SDL_Event &event) {
    if (event.type == SDL_MOUSEMOTION) {
        const std::vector<MenuButton> &active = ActiveButtons();
        hoveredIndex = -1;
        for (int i = 0; i < (int)active.size(); i++) {
            if (GameManager::IsPointInsideRect(event.motion.x, event.motion.y, active[i].rect)) {
                hoveredIndex = i;
                break;
            }
        }
        return;
    }

    if (screen == MenuScreen::MAIN)
        HandleMainInput(event);
    else
        HandleSetupInput(event);
}

void SceneMenu::HandleMainInput(const SDL_Event &event) {
    if (IsButtonClicked(buttons[0], event)) {
        screen = MenuScreen::OPPONENT_SELECT;
        hoveredIndex = -1;
        std::cout << "[MENU] Escolha a raça e o nivel do oponente." << std::endl;
        return;
    }
    if (IsButtonClicked(buttons[1], event)) {
        std::cout << "Coleção clicado" << std::endl;
        // GameManager.ChangeScene(new SceneCollection());
        return;
    }
    if (IsButtonClicked(buttons[2], event)) {
        std::cout << "Sair clicado" << std::endl;
        SDL_Event quit;
        quit.type = SDL_QUIT;
        SDL_PushEvent(&quit);
        return;
    }
}

void SceneMenu::HandleSetupInput(const SDL_Event &event) {
    for (int i = 0; i < kRaceCount; ++i) {
        if (!IsButtonClicked(setupButtons[i], event)) continue;
        selectedRace = kRaces[i];
        std::cout << "[MENU] Raça do oponente: " << translateRace(selectedRace) << std::endl;
        return;
    }

    for (int i = 0; i < kLevelCount; ++i) {
        if (!IsButtonClicked(setupButtons[kRaceCount + i], event)) continue;
        selectedLevel = i + 1;
        std::cout << "[MENU] Nivel do oponente: " << selectedLevel << std::endl;
        return;
    }

    if (IsButtonClicked(setupButtons[kIdxFight], event)) {
        StartOpponentBattle();
        return; // ChangeScene deletou esta cena: nada pode tocar em `this` aqui
    }

    if (IsButtonClicked(setupButtons[kIdxBack], event)) {
        screen = MenuScreen::MAIN;
        hoveredIndex = -1;
        return;
    }
}

void SceneMenu::StartOpponentBattle() {
    // A escolha vira deckType/deckPart antes do SceneBattle montar o baralho.
    gameManager.SetOpponentDeck(selectedRace, selectedLevel);
    std::cout << "[MENU] Oponente: " << translateRace(selectedRace) << " (nivel " << selectedLevel
              << ")" << std::endl;

    SceneBattle *battle = new SceneBattle();
    battle->Initialize(gameManager.GetRenderer());
    battle->StartBattle(&gameManager.GetPlayer(), &gameManager.GetOpponent(),
                        gameManager.GetRenderer());
    gameManager.ChangeMusic("assets/audio/music/battle_theme.mp3");
    gameManager.ChangeScene(battle);
}

void SceneMenu::Update(float dt) {}

// ═══════════════════════════════════════════════════════════════════
//  Render
// ═══════════════════════════════════════════════════════════════════

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

    if (screen == MenuScreen::OPPONENT_SELECT) {
        RenderOpponentSetup(renderer, w);
        return;
    }

    for (int i = 0; i < (int)buttons.size(); i++) {
        ui::UIRenderUtils::RenderButton(renderer, buttons[i].rect, buttons[i].label, font,
                                        i == hoveredIndex);
    }
}

void SceneMenu::RenderOpponentSetup(SDL_Renderer *renderer, int w) {
    const SDL_Color white = {255, 255, 255, 255};
    // Verde marca a opção escolhida; o resto usa as cores padrão do botão.
    const SDL_Color pickedNormal = {40, 130, 70, 255};
    const SDL_Color pickedHover = {60, 180, 100, 255};
    const SDL_Color pickedBorder = {160, 255, 190, 255};

    auto centeredLabel = [&](const std::string &text, int y) {
        if (!font) return;
        int textW = 0, textH = 0;
        TTF_SizeUTF8(font, text.c_str(), &textW, &textH);
        ui::UIRenderUtils::RenderText(renderer, text, (w - textW) / 2, y, white, font);
    };

    centeredLabel("Raça do oponente", setupButtons[0].rect.y - 40);
    centeredLabel("Nível do oponente", setupButtons[kRaceCount].rect.y - 40);

    for (int i = 0; i < (int)setupButtons.size(); ++i) {
        const bool hovered = (i == hoveredIndex);

        const bool isPickedRace = i < kRaceCount && kRaces[i] == selectedRace;
        const bool isPickedLevel =
            i >= kRaceCount && i < kIdxFight && (i - kRaceCount + 1) == selectedLevel;

        if (isPickedRace || isPickedLevel)
            ui::UIRenderUtils::RenderButton(renderer, setupButtons[i].rect, setupButtons[i].label,
                                            font, hovered, pickedNormal, pickedHover, pickedBorder,
                                            white);
        else
            ui::UIRenderUtils::RenderButton(renderer, setupButtons[i].rect, setupButtons[i].label,
                                            font, hovered);
    }
}
