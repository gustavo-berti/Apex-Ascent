#include "SceneMenu.hpp"
#include "../core/GameManager.hpp"
#include "SceneBattle.hpp"
#include <iostream>

SceneMenu::SceneMenu(GameManager &manager) : SceneUI(manager) {}

void SceneMenu::Initialize(SDL_Renderer *renderer) {
    SceneUI::Initialize(renderer);
    LoadBackground(renderer, "assets/images/start_menu.png");
    title = "Apex Ascent";
    ShowMainScreen();
}

// ── Tela principal ────────────────────────────────────────────────

void SceneMenu::ShowMainScreen() {
    showingOpponentSetup = false;
    hoveredIndex = -1;
    buttons.clear();

    const int btnW = 200, btnH = 50, gap = 30, margin = 40;
    const int stackHeight = btnH * 3 + gap * 2;
    const int x = screenWidth - margin - btnW;
    const int y = screenHeight - margin - stackHeight;

    buttons.push_back({{x, y, btnW, btnH}, "Começar Jogo", [this] {
                           std::cout << "[MENU] Escolha a raça e o nivel do oponente." << std::endl;
                           ShowOpponentSetup();
                       }});

    buttons.push_back({{x, y + gap + btnH, btnW, btnH}, "Coleção",
                       [] { std::cout << "Coleção clicado" << std::endl; }});

    buttons.push_back({{x, y + (gap + btnH) * 2, btnW, btnH}, "Sair", [] {
                           std::cout << "Sair clicado" << std::endl;
                           SDL_Event quit;
                           quit.type = SDL_QUIT;
                           SDL_PushEvent(&quit);
                       }});
}

// ── Escolha do oponente ───────────────────────────────────────────
// Duas fileiras centralizadas (raça e nivel) e as ações embaixo.

void SceneMenu::ShowOpponentSetup() {
    showingOpponentSetup = true;
    hoveredIndex = -1;
    buttons.clear();

    const int raceW = 210, raceH = 50, raceGap = 20;
    const int raceRowW = kRaceCount * raceW + (kRaceCount - 1) * raceGap;
    const int raceX = (screenWidth - raceRowW) / 2;
    const int raceY = screenHeight / 2 - 150;

    for (int i = 0; i < kRaceCount; ++i) {
        const Race race = kRaces[i];
        buttons.push_back({{raceX + i * (raceW + raceGap), raceY, raceW, raceH},
                           translateRace(race),
                           [this, race] { SelectRace(race); },
                           race == selectedRace ? ui::styles::kSelected : ui::styles::kDefault});
    }

    const int lvlW = 100, lvlH = 50, lvlGap = 20;
    const int lvlRowW = kLevelCount * lvlW + (kLevelCount - 1) * lvlGap;
    const int lvlX = (screenWidth - lvlRowW) / 2;
    const int lvlY = screenHeight / 2;

    for (int i = 0; i < kLevelCount; ++i) {
        const int level = i + 1;
        buttons.push_back({{lvlX + i * (lvlW + lvlGap), lvlY, lvlW, lvlH},
                           std::to_string(level),
                           [this, level] { SelectLevel(level); },
                           level == selectedLevel ? ui::styles::kSelected : ui::styles::kDefault});
    }

    const int actW = 200, actH = 50, actGap = 40;
    const int actY = screenHeight / 2 + 150;
    buttons.push_back({{screenWidth / 2 - actW - actGap / 2, actY, actW, actH}, "Lutar",
                       [this] { StartOpponentBattle(); }});
    buttons.push_back(
        {{screenWidth / 2 + actGap / 2, actY, actW, actH}, "Voltar", [this] { ShowMainScreen(); }});
}

void SceneMenu::SelectRace(Race race) {
    selectedRace = race;
    std::cout << "[MENU] Raça do oponente: " << translateRace(race) << std::endl;

    for (int i = 0; i < kRaceCount; ++i)
        buttons[i].style = kRaces[i] == race ? ui::styles::kSelected : ui::styles::kDefault;
}

void SceneMenu::SelectLevel(int level) {
    selectedLevel = level;
    std::cout << "[MENU] Nivel do oponente: " << level << std::endl;

    for (int i = 0; i < kLevelCount; ++i)
        buttons[kRaceCount + i].style =
            i + 1 == level ? ui::styles::kSelected : ui::styles::kDefault;
}

void SceneMenu::StartOpponentBattle() {
    // A escolha vira deckType/deckPart antes do SceneBattle montar o baralho.
    gameManager.SetOpponentDeck(selectedRace, selectedLevel);
    std::cout << "[MENU] Oponente: " << translateRace(selectedRace) << " (nivel " << selectedLevel
              << ")" << std::endl;

    SceneBattle *battle = new SceneBattle(gameManager);
    battle->Initialize(gameManager.GetRenderer());
    battle->StartBattle(&gameManager.GetPlayer(), &gameManager.GetOpponent(),
                        gameManager.GetRenderer());
    gameManager.ChangeMusic("assets/audio/music/battle_theme.mp3");
    gameManager.ChangeScene(battle);
}

// ── Render ────────────────────────────────────────────────────────

void SceneMenu::RenderContent(SDL_Renderer *renderer) {
    if (!showingOpponentSetup) return;

    const SDL_Color white = {255, 255, 255, 255};
    RenderCenteredText(renderer, font, "Raça do oponente", buttons[0].rect.y - 40, white);
    RenderCenteredText(renderer, font, "Nível do oponente", buttons[kRaceCount].rect.y - 40, white);
}
