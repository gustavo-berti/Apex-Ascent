#include "SceneBattle.hpp"
#include "../core/GameManager.hpp"
#include "../logic/CardFactory.hpp"
#include "../objects/cards/SpellCard.hpp"
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <iostream>
#include <random>

// ═══════════════════════════════════════════════════════════════════
//  Construtor / Destrutor
//  TurnManager é declarado antes de Board no .hpp, garantindo que
//  board(turnManager) seja inicializado na ordem correta.
// ═══════════════════════════════════════════════════════════════════

SceneBattle::SceneBattle() : board(turnManager), draggedCard(nullptr) {}

SceneBattle::~SceneBattle() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (fontSmall) {
        TTF_CloseFont(fontSmall);
        fontSmall = nullptr;
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Initialize
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::Initialize(SDL_Renderer *renderer) {
    std::cout << "Inicializando SceneBattle..." << std::endl;

    if (!cardDatabase.LoadFromJson("assets/data/cards.json"))
        std::cerr << "Falha ao carregar cards.json" << std::endl;

    // Carrega a fonte TTF para a tela de vitória/derrota
    font = TTF_OpenFont("./assets/fonts/arial.ttf", 72);
    if (!font) {
        std::cerr << "Falha ao carregar fonte TTF (72): " << TTF_GetError() << std::endl;
    }

    // Carrega a fonte pequena para HP display
    fontSmall = TTF_OpenFont("./assets/fonts/arial.ttf", 28);
    if (!fontSmall) {
        std::cerr << "Falha ao carregar fonte pequena (28): " << TTF_GetError() << std::endl;
    }

    const int boardWidth = 1000;
    const int boardX = (1600 - boardWidth) / 2; // 300

    enemyPreparationZone = {boardX, 25, boardWidth, 150};
    enemyBattleZone = {boardX, 200, boardWidth, 150};
    playerBattleZone = {boardX, 413, boardWidth, 150};
    playerPreparationZone = {boardX, 600, boardWidth, 150};
    playerHandZone = {0, 740, 1600, 160};

    btnCancel = {1420, 300, 150, 50};
    btnNextPhase = {1420, 360, 150, 50};
    btnAttack = {1420, 430, 150, 50};

    board.SetZoneRects(playerPreparationZone, playerBattleZone, enemyPreparationZone,
                       enemyBattleZone);

    turnManager.SetOnPhaseChanged([this](TurnOwner o, BattlePhase p) { OnPhaseChanged(o, p); });
    turnManager.SetOnTurnChanged([this](TurnOwner o) { OnTurnChanged(o); });
    turnManager.SetOnCombatStepChanged([this](CombatStep s) { OnCombatStepChanged(s); });
}

// ═══════════════════════════════════════════════════════════════════
//  StartBattle
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::StartBattle(Player *playerState, Opponent *opp, SDL_Renderer *sdlRenderer) {
    if (!SetCurrentPlayerState(playerState)) return;
    if (!opp) {
        std::cerr << "Opponent invalido." << std::endl;
        return;
    }

    opponent = opp;
    this->renderer = sdlRenderer;
    outcome = BattleOutcome::ONGOING;

    ResetBattleState();
    BuildDrawPileFromPlayerDeck();
    ShuffleDrawPile();
    DrawCards(5);

    srand(static_cast<unsigned>(SDL_GetTicks()));
    turnManager.RollForFirstTurn();
    std::cout << "=== SORTEIO: " << turnManager.GetOwnerName() << " comeca! ===" << std::endl;
    std::cout << "[BATALHA] Oponente e " << (opponent->isGuardian ? "GUARDIAO" : "normal")
              << " | HP: " << opponent->currentHealth << "/" << opponent->maxHealth << std::endl;

    // Quem perdeu o sorteio recebe +1 mana de compensação
    if (turnManager.GetFirstOwner() == TurnOwner::PLAYER) {
        opponent->mana.total = 2;
        opponent->mana.current = 2;
        std::cout << "[MANA] Oponente recebeu +1 mana de compensacao." << std::endl;
    } else {
        currentState->mana.total = 2;
        currentState->mana.current = 2;
        std::cout << "[MANA] Jogador recebeu +1 mana de compensacao." << std::endl;
    }

    if (turnManager.IsPlayerTurn()) {
        HandleTurnStart();
    } else {
        RunOpponentTurn();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Callbacks do TurnManager
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::OnPhaseChanged(TurnOwner owner, BattlePhase phase) {
    std::cout << "[" << turnManager.GetOwnerName() << "] " << turnManager.GetPhaseName()
              << std::endl;

    if (phase == BattlePhase::TURN_START && owner == TurnOwner::PLAYER) HandleTurnStart();
}

void SceneBattle::OnTurnChanged(TurnOwner owner) {
    std::cout << "=== TURNO: " << turnManager.GetOwnerName() << " ===" << std::endl;
    if (owner == TurnOwner::OPPONENT) RunOpponentTurn();
}

void SceneBattle::OnCombatStepChanged(CombatStep step) {
    std::cout << "[COMBATE] " << turnManager.GetCombatStepName() << std::endl;

    if (step == CombatStep::DECLARE_DEFENDERS) {
        board.ResolveDefenders();
        turnManager.AdvanceCombatStep(); // → RESOLUTION
    }

    if (step == CombatStep::RESOLUTION) {
        // Board calcula dano e retorna o resultado
        CombatResult result = board.ResolveCombat(opponent->currentHealth, cardObjects);
        CheckBattleOutcome(result);

        if (!IsBattleOver()) turnManager.AdvancePhase(); // COMBAT → SECOND_MAIN
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Condição de vitória / derrota
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::CheckBattleOutcome(const CombatResult &result) {
    if (result.damageDealt > 0) {
        std::cout << "[BATALHA] Oponente recebeu " << result.damageDealt
                  << " de dano. HP: " << opponent->currentHealth << "/" << opponent->maxHealth
                  << std::endl;
    }

    if (opponent->IsDefeated()) {
        outcome = BattleOutcome::PLAYER_WIN;
        std::cout << "=== JOGADOR VENCEU A BATALHA! ===" << std::endl;
        return;
    }

    // Verifica derrota do jogador
    if (currentState->currentHealth <= 0) {
        outcome = BattleOutcome::PLAYER_LOSE;
        std::cout << "=== JOGADOR PERDEU A BATALHA! ===" << std::endl;
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Lógica de turno
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleTurnStart() {
    bool gainMana = turnManager.ShouldGainManaThisTurn();
    bool drawCard = turnManager.ShouldDrawThisTurn();

    currentState->mana.OnTurnStart(gainMana);
    std::cout << "[INICIO DE TURNO] Mana: " << currentState->mana.current << "/"
              << currentState->mana.total << (gainMana ? "" : " (1o turno, sem incremento)")
              << std::endl;

    if (drawCard)
        DrawCards(currentState->GetTurnStartDrawCount());
    else
        std::cout << "[INICIO DE TURNO] Sem compra — primeiro turno." << std::endl;

    turnManager.AdvancePhase(); // TURN_START → MAIN
}

void SceneBattle::RunOpponentTurn() {
    std::cout << "[OPONENTE] Processando turno automaticamente..." << std::endl;

    bool gainMana = turnManager.ShouldGainManaThisTurn();
    opponent->mana.OnTurnStart(gainMana);
    std::cout << "[OPONENTE] Mana: " << opponent->mana.current << "/" << opponent->mana.total
              << (gainMana ? "" : " (1o turno, sem incremento)") << std::endl;

    turnManager.AdvancePhase(); // TURN_START  → MAIN
    turnManager.AdvancePhase(); // MAIN        → COMBAT
    turnManager.AdvancePhase(); // COMBAT      → SECOND_MAIN
    turnManager.AdvancePhase(); // SECOND_MAIN → fim (volta ao jogador)
}

// ═══════════════════════════════════════════════════════════════════
//  Mana
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::SpendPlayerMana(int cost, const std::string &cardName) {
    if (!currentState->mana.CanAfford(cost)) {
        std::cout << "[MANA] Insuficiente para " << cardName << " (custo: " << cost
                  << ", disponivel: " << currentState->mana.current << ")" << std::endl;
        return false;
    }
    currentState->mana.Spend(cost);
    std::cout << "[MANA] Gastou " << cost << " em " << cardName << " ("
              << currentState->mana.current << "/" << currentState->mana.total << ")" << std::endl;
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Helpers de estado
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::CanPlayCreature() const {
    if (!turnManager.IsPlayerTurn()) return false;
    auto p = turnManager.GetPhase();
    return p == BattlePhase::MAIN || p == BattlePhase::SECOND_MAIN;
}

bool SceneBattle::CanPlaySpell() const {
    if (!turnManager.IsPlayerTurn()) return false;
    auto p = turnManager.GetPhase();
    auto s = turnManager.GetCombatStep();
    return p == BattlePhase::MAIN || p == BattlePhase::SECOND_MAIN ||
           s == CombatStep::ATTACK_MAGIC || s == CombatStep::DECLARE_DEFENDERS;
}

// ═══════════════════════════════════════════════════════════════════
//  Combate
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleAttackButton() {
    if (!board.CanDeclareAttack()) return;
    turnManager.AdvanceCombatStep(); // DECLARE_ATTACKERS → ATTACK_MAGIC
    std::cout << "[COMBATE] Selecione atacantes e confirme (ou cancele)." << std::endl;
}

void SceneBattle::HandleCancelAttack() {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return;
    board.ReturnAllAttackersToPreparation(cardObjects);
    std::cout << "[COMBATE] Ataque cancelado." << std::endl;
}

void SceneBattle::HandleConfirmAttack() {
    if (!board.ConfirmAttack()) {
        turnManager.AdvancePhase(); // COMBAT → SECOND_MAIN sem atacar
        return;
    }
    turnManager.AdvanceCombatStep(); // ATTACK_MAGIC → DECLARE_DEFENDERS
    // Fluxo continua via OnCombatStepChanged
}

// ═══════════════════════════════════════════════════════════════════
//  HandleInput
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleInput(SDL_Event &event) {
    // Após a batalha terminar, ignora toda entrada de jogo
    if (IsBattleOver()) return;

    if (event.type != SDL_MOUSEBUTTONDOWN) return;
    if (HandleCancelClick(event)) return;
    if (HandleAttackClick(event)) return;
    if (HandleNextPhaseClick(event)) return;
    if (!turnManager.IsPlayerTurn()) return;
    if (HandleBattleCardClick(event)) return;
    HandleHandCardClick(event);
}

bool SceneBattle::HandleNextPhaseClick(const SDL_Event &e) {
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;
    if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, btnNextPhase)) return false;
    if (!turnManager.IsPlayerTurn()) return true;

    auto phase = turnManager.GetPhase();
    auto step = turnManager.GetCombatStep();

    if (phase == BattlePhase::COMBAT && step == CombatStep::ATTACK_MAGIC) {
        HandleConfirmAttack();
        return true;
    }
    if (phase == BattlePhase::COMBAT && step == CombatStep::DECLARE_ATTACKERS) {
        turnManager.AdvancePhase(); // COMBAT → SECOND_MAIN sem atacar
        return true;
    }

    turnManager.AdvancePhase();
    return true;
}

bool SceneBattle::HandleAttackClick(const SDL_Event &e) {
    if (!board.ShouldShowAttackButton()) return false;
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;
    if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, btnAttack)) return false;
    HandleAttackButton();
    return true;
}

bool SceneBattle::HandleCancelClick(const SDL_Event &e) {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return false;
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;
    if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, btnCancel)) return false;
    HandleCancelAttack();
    return true;
}

bool SceneBattle::HandleHandCardClick(const SDL_Event &e) {
    for (auto it = hand.rbegin(); it != hand.rend(); ++it) {
        Card *card = *it;
        if (!card) continue;

        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;

        bool isCreature = dynamic_cast<CreatureCard *>(card) != nullptr;

        if (isCreature && CanPlayCreature()) {
            if (!SpendPlayerMana(card->GetManaCost(), card->GetName())) return true;
            if (board.AddToPlayerPreparation(card, cardObjects)) {
                hand.erase(std::next(it).base());
                // Reorganiza a mão
                int n = hand.size(), cw = 100, gap = 15;
                int totalW = n * cw + (n - 1) * gap;
                int startX = playerHandZone.x + (playerHandZone.w - totalW) / 2;
                int y = playerHandZone.y + (playerHandZone.h - 140) / 2;
                for (int i = 0; i < n; ++i)
                    hand[i]->SetPosition(startX + i * (cw + gap), y);
            } else {
                currentState->mana.current += card->GetManaCost(); // devolve mana
            }
        } else if (!isCreature && CanPlaySpell()) {
            if (!SpendPlayerMana(card->GetManaCost(), card->GetName())) return true;
            // TODO: pilha de resposta (implementar depois)
            std::cout << "[MAGIA] " << card->GetName() << " jogada (efeito pendente)." << std::endl;
        } else {
            std::cout << "[AVISO] Nao e possivel jogar essa carta agora." << std::endl;
        }
        return true;
    }
    return false;
}

bool SceneBattle::HandleBattleCardClick(const SDL_Event &e) {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return false;

    for (auto *card : board.GetPlayerBattleCards()) {
        if (!card) continue;
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;
        board.ToggleAttackerSelection(card, cardObjects);
        return true;
    }
    for (auto *card : board.GetPlayerPreparationCards()) {
        if (!card) continue;
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;
        board.ToggleAttackerSelection(card, cardObjects);
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════
//  Update / Render
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::Update(float dt) {
    for (auto obj : objects)
        obj->Update(dt);
}

void SceneBattle::Render(SDL_Renderer *renderer) {
    board.Render(renderer);
    RenderHand(renderer);
    RenderButtons(renderer);
    RenderMana(renderer);
    RenderHealthBars(renderer);
    RenderHUD(renderer);

    if (IsBattleOver()) RenderOutcome(renderer);
}

void SceneBattle::RenderHand(SDL_Renderer *renderer) const {
    for (auto *c : hand)
        c->Render(renderer);
}

void SceneBattle::RenderButton(SDL_Renderer *renderer, SDL_Rect r, Uint8 red, Uint8 grn, Uint8 blu,
                               bool enabled) const {
    SDL_SetRenderDrawColor(renderer, enabled ? red : 60, enabled ? grn : 60, enabled ? blu : 60,
                           255);
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &r);
}

void SceneBattle::RenderButtons(SDL_Renderer *renderer) const {
    if (IsBattleOver()) return;

    bool isPlayer = turnManager.IsPlayerTurn();
    auto step = turnManager.GetCombatStep();

    RenderButton(renderer, btnNextPhase, 220, 160, 0, isPlayer);

    if (board.ShouldShowAttackButton())
        RenderButton(renderer, btnAttack, 200, 50, 50, board.CanDeclareAttack());

    if (step == CombatStep::ATTACK_MAGIC && isPlayer)
        RenderButton(renderer, btnCancel, 80, 80, 200, true);
}

// ── Barras de HP ──────────────────────────────────────────────────
// Jogador: canto inferior esquerdo
// Oponente: canto superior esquerdo (abaixo do HUD de turno)
void SceneBattle::RenderHealthBars(SDL_Renderer *renderer) const {
    if (!currentState || !opponent || !fontSmall) return;

    // ── HP do oponente (posição superior esquerda, abaixo do painel de turno) ─────────
    {
        char hpText[64];
        snprintf(hpText, sizeof(hpText), "Oponente: %d/%d", opponent->currentHealth, opponent->maxHealth);
        
        SDL_Color color = opponent->isGuardian ? SDL_Color{220, 130, 30, 255} : SDL_Color{200, 50, 50, 255};
        SDL_Surface *surface = TTF_RenderText_Solid(fontSmall, hpText, color);
        
        if (surface) {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {10, 90, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, nullptr, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

    // ── HP do jogador (canto inferior esquerdo) ────────────────────
    {
        char hpText[64];
        snprintf(hpText, sizeof(hpText), "Você: %d/%d", currentState->currentHealth, currentState->maxHealth);
        
        SDL_Color color{50, 200, 80, 255};  // verde para jogador
        SDL_Surface *surface = TTF_RenderText_Solid(fontSmall, hpText, color);
        
        if (surface) {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {10, 870, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, nullptr, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
}

void SceneBattle::RenderMana(SDL_Renderer *renderer) const {
    if (!currentState || !opponent) return;

    const ManaState &pm = currentState->mana;
    const ManaState &om = opponent->mana;

    auto clampMana = [](int v) { return v < 0 ? 0 : v > 99 ? 99 : v; };

    auto drawDigit = [&](int x, int y, int digit, Uint8 r, Uint8 g, Uint8 b) {
        static const bool segs[10][7] = {
            {1, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 0, 1},
            {1, 1, 1, 1, 0, 0, 1}, {0, 1, 1, 0, 0, 1, 1}, {1, 0, 1, 1, 0, 1, 1},
            {1, 0, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 1, 1},
        };
        const int t = 4, w = 22, h = 36;
        SDL_Rect seg[7] = {
            {x + t, y, w - 2 * t, t},
            {x + w - t, y + t, t, h / 2 - t},
            {x + w - t, y + h / 2, t, h / 2 - t},
            {x + t, y + h - t, w - 2 * t, t},
            {x, y + h / 2, t, h / 2 - t},
            {x, y + t, t, h / 2 - t},
            {x + t, y + h / 2 - t / 2, w - 2 * t, t},
        };
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        for (int i = 0; i < 7; ++i)
            if (segs[digit][i]) SDL_RenderFillRect(renderer, &seg[i]);
    };

    auto drawMana = [&](int value, int x, int y, Uint8 bgR, Uint8 bgG, Uint8 bgB, Uint8 fgR,
                        Uint8 fgG, Uint8 fgB) {
        value = clampMana(value);
        SDL_Rect bg = {x - 8, y - 8, 62, 52};
        SDL_SetRenderDrawColor(renderer, bgR, bgG, bgB, 210);
        SDL_RenderFillRect(renderer, &bg);
        SDL_SetRenderDrawColor(renderer, fgR, fgG, fgB, 255);
        SDL_RenderDrawRect(renderer, &bg);
        if (value / 10 > 0) drawDigit(x, y, value / 10, fgR, fgG, fgB);
        drawDigit(x + 28, y, value % 10, fgR, fgG, fgB);
    };

    drawMana(pm.current, 18, 760, 10, 24, 70, 100, 200, 255);
    drawMana(om.current, 1512, 16, 70, 12, 12, 255, 120, 120);
}

void SceneBattle::RenderHUD(SDL_Renderer *renderer) const {
    SDL_Rect panel = {10, 10, 185, 70};
    bool isPlayer = turnManager.IsPlayerTurn();

    SDL_SetRenderDrawColor(renderer, isPlayer ? 30 : 180, isPlayer ? 80 : 30, isPlayer ? 180 : 30,
                           210);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &panel);

    int phaseIdx = static_cast<int>(turnManager.GetPhase());
    for (int i = 0; i < 4; ++i) {
        SDL_Rect pip = {15 + i * 44, 60, 36, 10};
        SDL_SetRenderDrawColor(renderer, i == phaseIdx ? 255 : 60, i == phaseIdx ? 220 : 60,
                               i == phaseIdx ? 50 : 60, 255);
        SDL_RenderFillRect(renderer, &pip);
    }

    if (turnManager.GetPhase() == BattlePhase::COMBAT) {
        int stepIdx = static_cast<int>(turnManager.GetCombatStep());
        for (int i = 1; i <= 4; ++i) {
            SDL_Rect pip = {15 + (i - 1) * 44, 75, 36, 6};
            SDL_SetRenderDrawColor(renderer, i <= stepIdx ? 255 : 40, i <= stepIdx ? 100 : 40, 40,
                                   255);
            SDL_RenderFillRect(renderer, &pip);
        }
    }
}

// Overlay simples de vitória/derrota — fundo semitransparente + retângulo colorido
void SceneBattle::RenderOutcome(SDL_Renderer *renderer) const {
    if (!font) return;

    // Fundo escurecido
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, 1600, 900};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Renderiza texto baseado no resultado
    const char *text = (outcome == BattleOutcome::PLAYER_WIN) ? "VITÓRIA!" : "DERROTA!";
    SDL_Color color = (outcome == BattleOutcome::PLAYER_WIN)
                          ? SDL_Color{30, 255, 60, 255}   // verde para vitória
                          : SDL_Color{255, 50, 50, 255};  // vermelho para derrota

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) {
        std::cerr << "Falha ao renderizar texto: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Falha ao criar textura: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return;
    }

    // Centraliza o texto na tela
    SDL_Rect dest = {(1600 - surface->w) / 2, (900 - surface->h) / 2, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dest);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// ═══════════════════════════════════════════════════════════════════
//  Deck / cartas
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::SetCurrentPlayerState(Player *p) {
    if (!p) {
        std::cout << "Player invalido." << std::endl;
        return false;
    }
    currentState = p;
    return true;
}

void SceneBattle::ResetBattleState() {
    // Destrói todos os objetos anteriores antes de limpar
    for (auto *obj : objects)
        delete obj;
    objects.clear();

    // limpa ponteiros específicos de carta (não delete novamente — já foram deletados acima)
    cardObjects.clear();

    drawPile.clear();
    hand.clear();
    discardPile.clear();
    board.Reset();

    if (currentState) currentState->mana = ManaState{};
    if (opponent) opponent->Reset();
}

void SceneBattle::AddDeckCardToDrawPile(const std::string &cardId) {
    int id = 0;
    try {
        id = std::stoi(cardId);
    } catch (...) {
        std::cout << "ID invalido: " << cardId << std::endl;
        return;
    }

    Card *card = nullptr;
    if (cardDatabase.GetCreature(id)) {
        card = CardFactory::CreateCreatureCard(cardDatabase, id);
    } else if (const SpellData *sd = cardDatabase.GetSpell(id)) {
        card = new SpellCard(sd, -200, -200);
    } else {
        std::cout << "Carta ID " << id << " nao encontrada." << std::endl;
        return;
    }

    if (!card) return;
    card->SetPosition(-200, -200);
    drawPile.push_back(card);
    objects.push_back(card);
    cardObjects.push_back(card);
}

void SceneBattle::BuildDrawPileFromPlayerDeck() {
    if (!currentState) return;
    for (const auto &id : currentState->GetMasterDeck())
        AddDeckCardToDrawPile(id);
    std::cout << "[DECK] " << drawPile.size() << " carta(s) na pilha." << std::endl;
}

void SceneBattle::ShuffleDrawPile() {
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::shuffle(drawPile.begin(), drawPile.end(), rng);
}

void SceneBattle::DrawCards(int amount) {
    if (!currentState || amount <= 0) return;

    for (int i = 0; i < amount; ++i) {
        if (drawPile.empty()) {
            std::cout << "Pilha vazia!" << std::endl;
            break;
        }

        Card *card = drawPile.back();
        drawPile.pop_back();

        if (currentState->IsHandFull(static_cast<int>(hand.size()))) {
            std::cout << "Mao cheia! " << card->GetName() << " destruida." << std::endl;
            objects.erase(std::remove(objects.begin(), objects.end(), card), objects.end());
            cardObjects.erase(std::remove(cardObjects.begin(), cardObjects.end(), card), cardObjects.end());
            delete card;
        } else {
            hand.push_back(card);
            if (renderer) card->LoadTexture(renderer);
            std::cout << "Carta comprada: " << card->GetName() << std::endl;
        }
    }

    // Reorganiza a mão
    int n = hand.size(), cw = 100, gap = 15;
    int totalW = n * cw + (n - 1) * gap;
    int startX = playerHandZone.x + (playerHandZone.w - totalW) / 2;
    int y = playerHandZone.y + (playerHandZone.h - 140) / 2;
    for (int i = 0; i < n; ++i)
        hand[i]->SetPosition(startX + i * (cw + gap), y);
}