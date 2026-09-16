#include "SceneMenu.hpp"
#include "../core/GameManager.hpp"
#include "../objects/ui/UIRenderUtils.hpp"
#include "SceneBattle.hpp"
#include <iostream>

SceneMenu::SceneMenu(GameManager &manager) : SceneUI(manager) {}

SceneMenu::~SceneMenu() { SetTextInputActive(false); }

void SceneMenu::Initialize(SDL_Renderer *renderer) {
    SceneUI::Initialize(renderer);
    LoadBackground(renderer, "assets/images/start_menu.png");
    title = "Apex Ascent";

    nameField.placeholder = "Digite seu nome";
    nameField.maxLength = kNameMaxLength;

    ShowMainScreen();
}

// O campo de nome fica com o teclado enquanto estiver em foco; o resto do
// evento segue para os botoes da tela.
void SceneMenu::HandleInput(SDL_Event &event) {
    if (screen == Screen::OPPONENT_SETUP && nameField.HandleEvent(event)) {
        if (!nameField.Trimmed().empty()) nameMissing = false;
        return;
    }

    SceneUI::HandleInput(event);
}

void SceneMenu::SetTextInputActive(bool active) {
    if (active)
        SDL_StartTextInput();
    else
        SDL_StopTextInput();
}

// ── Tela principal ────────────────────────────────────────────────

void SceneMenu::ShowMainScreen() {
    screen = Screen::MAIN;
    title = "Apex Ascent";
    hoveredIndex = -1;
    buttons.clear();
    SetTextInputActive(false);
    nameField.focused = false;

    const int btnW = 200, btnH = 50, gap = 30, margin = 40;
    const int stackHeight = btnH * 4 + gap * 3;
    const int x = screenWidth - margin - btnW;
    const int y = screenHeight - margin - stackHeight;

    buttons.push_back({{x, y, btnW, btnH}, "Começar Jogo", [this] {
                           std::cout << "[MENU] Escolha a raça e o nivel do oponente." << std::endl;
                           ShowOpponentSetup();
                       }});

    buttons.push_back({{x, y + gap + btnH, btnW, btnH}, "Coleção",
                       [] { std::cout << "Coleção clicado" << std::endl; }});

    buttons.push_back({{x, y + (gap + btnH) * 2, btnW, btnH}, "Pontuações",
                       [this] { ShowScores(); }});

    buttons.push_back({{x, y + (gap + btnH) * 3, btnW, btnH}, "Sair", [] {
                           std::cout << "Sair clicado" << std::endl;
                           SDL_Event quit;
                           quit.type = SDL_QUIT;
                           SDL_PushEvent(&quit);
                       }});
}

// ── Escolha do oponente ───────────────────────────────────────────
// Duas fileiras centralizadas (raça e nivel) e as ações embaixo.

void SceneMenu::ShowOpponentSetup() {
    screen = Screen::OPPONENT_SETUP;
    hoveredIndex = -1;
    buttons.clear();
    nameMissing = false;

    // Quem ja jogou uma partida volta com o nome preenchido.
    const std::string &savedName = gameManager.GetPlayer().name;
    if (nameField.text.empty() && savedName != Player::kDefaultName) nameField.text = savedName;

    const int nameW = 360, nameH = 50;
    nameField.rect = {(screenWidth - nameW) / 2, screenHeight / 2 - 270, nameW, nameH};
    nameField.focused = true; // pronto pra digitar assim que a tela abre
    SetTextInputActive(true);

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

// ── Placar ────────────────────────────────────────────────────────

void SceneMenu::ShowScores() {
    screen = Screen::SCORES;
    title = "Melhores Pontuações";
    hoveredIndex = -1;
    buttons.clear();
    SetTextInputActive(false);
    nameField.focused = false;

    // Le do disco na hora: a partida que acabou de terminar ja esta no arquivo.
    scores.Load();
    std::cout << "[MENU] Placar com " << scores.GetEntries().size() << " pontuação(oes)."
              << std::endl;

    const int btnW = 200, btnH = 50;
    buttons.push_back({{(screenWidth - btnW) / 2, screenHeight - 110, btnW, btnH}, "Voltar",
                       [this] { ShowMainScreen(); }});
}

void SceneMenu::StartOpponentBattle() {
    // Sem nome nao começa: a partida ja nasce sabendo como vai assinar o placar.
    const std::string playerName = nameField.Trimmed();
    if (playerName.empty()) {
        nameMissing = true;
        std::cout << "[MENU] Escolha um nome antes de lutar." << std::endl;
        return;
    }

    gameManager.GetPlayer().name = playerName;

    // A escolha vira deckType/deckPart antes do SceneBattle montar o baralho.
    gameManager.SetOpponentDeck(selectedRace, selectedLevel);
    std::cout << "[MENU] " << playerName << " contra " << translateRace(selectedRace) << " (nivel "
              << selectedLevel << ")" << std::endl;

    SetTextInputActive(false);

    SceneBattle *battle = new SceneBattle(gameManager);
    battle->Initialize(gameManager.GetRenderer());
    battle->StartBattle(&gameManager.GetPlayer(), &gameManager.GetOpponent(),
                        gameManager.GetRenderer());
    gameManager.ChangeMusic("assets/audio/music/battle_theme.mp3");
    gameManager.ChangeScene(battle);
}

// ── Render ────────────────────────────────────────────────────────

void SceneMenu::RenderContent(SDL_Renderer *renderer) {
    if (screen == Screen::SCORES) {
        RenderScoreTable(renderer);
        return;
    }

    if (screen != Screen::OPPONENT_SETUP) return;

    const SDL_Color white = {255, 255, 255, 255};
    RenderCenteredText(renderer, font, "Seu nome", nameField.rect.y - 40, white);
    ui::RenderTextField(renderer, nameField, font);
    RenderCenteredText(renderer, font, "Raça do oponente", buttons[0].rect.y - 40, white);
    RenderCenteredText(renderer, font, "Nível do oponente", buttons[kRaceCount].rect.y - 40, white);

    if (nameMissing)
        RenderCenteredText(renderer, font, "Digite um nome para entrar no placar.",
                           buttons.back().rect.y + 70, {255, 120, 120, 255});
}

// Cada linha e uma partida: quem jogou, quanto fez e como a run acabou (contra
// que raça, em que dificuldade, com quanta vida e carta sobrando).
void SceneMenu::RenderScoreTable(SDL_Renderer *renderer) const {
    const SDL_Color white = {255, 255, 255, 255};
    const auto &entries = scores.GetEntries();

    // Painel escuro: o fundo do menu deixaria a tabela ilegivel.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_Rect panel = {300, 150, 1000, 530};
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 180, 180, 255, 255);
    SDL_RenderDrawRect(renderer, &panel);

    if (entries.empty()) {
        RenderCenteredText(renderer, font, "Nenhuma partida registrada ainda.", 380, white);
        return;
    }

    constexpr int kColX[] = {330, 380, 610, 730, 890, 1080, 1180};
    constexpr const char *kColLabel[] = {"#",           "Nome", "Pontos", "Raça",
                                         "Dificuldade", "Vida", "Cartas"};
    constexpr int kColCount = 7;
    constexpr int kRowHeight = 44;
    constexpr int kTableY = 175;

    for (int col = 0; col < kColCount; ++col)
        ui::UIRenderUtils::RenderText(renderer, kColLabel[col], kColX[col], kTableY,
                                      {255, 220, 80, 255}, font);

    for (int row = 0; row < static_cast<int>(entries.size()); ++row) {
        const ScoreEntry &entry = entries[row];

        // Placar antigo nao tem nome gravado.
        const std::string name = entry.name.empty() ? "-" : entry.name;
        const std::string cells[kColCount] = {
            std::to_string(row + 1) + "o",     name,
            std::to_string(entry.score),       translateRace(entry.opponentRace),
            std::to_string(entry.difficulty),  std::to_string(entry.health),
            std::to_string(entry.cardsLeft)};

        const int y = kTableY + (row + 1) * kRowHeight;
        for (int col = 0; col < kColCount; ++col)
            ui::UIRenderUtils::RenderText(renderer, cells[col], kColX[col], y, white, font);
    }
}
