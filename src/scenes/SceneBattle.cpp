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
    if (fontUI) {
        TTF_CloseFont(fontUI);
        fontUI = nullptr;
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Initialize
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::Initialize() {
    std::cout << "Inicializando SceneBattle..." << std::endl;

    if (!cardDatabase.LoadFromJson("assets/data/cards.json"))
        std::cerr << "Falha ao carregar cards.json" << std::endl;

    font = TTF_OpenFont("assets/fonts/frozen.ttf", 72);
    fontSmall = TTF_OpenFont("assets/fonts/frozen.ttf", 28);
    fontUI = TTF_OpenFont("assets/fonts/frozen.ttf", 32);

    if (!font) std::cerr << "Falha ao carregar fonte (72): " << TTF_GetError() << std::endl;
    if (!fontSmall) std::cerr << "Falha ao carregar fonte (28): " << TTF_GetError() << std::endl;
    if (!fontUI) std::cerr << "Falha ao carregar frozen (32): " << TTF_GetError() << std::endl;

    const int boardWidth = 1000;
    const int boardX = (1600 - boardWidth) / 2;

    enemyPreparationZone = {boardX, 25, boardWidth, 150};
    enemyBattleZone = {boardX, 200, boardWidth, 150};
    playerBattleZone = {boardX, 413, boardWidth, 150};
    playerPreparationZone = {boardX, 600, boardWidth, 150};
    playerHandZone = {0, 740, 1600, 160};

    btnCancel = {1420, 300, 150, 50};
    btnNextPhase = {1420, 360, 150, 50};
    btnAttack = {1420, 420, 150, 50};

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
    summonPending.Clear();

    ResetBattleState();
    BuildDrawPileFromPlayerDeck();
    ShuffleDrawPile();
    DrawCards(5);

    srand(static_cast<unsigned>(SDL_GetTicks()));
    turnManager.RollForFirstTurn();
    std::cout << "=== SORTEIO: " << turnManager.GetOwnerName() << " comeca! ===" << std::endl;
    std::cout << "[BATALHA] Oponente e " << (opponent->isGuardian ? "GUARDIAO" : "normal")
              << " | HP: " << opponent->currentHealth << "/" << opponent->maxHealth << std::endl;

    if (turnManager.GetFirstOwner() == TurnOwner::PLAYER) {
        opponent->mana.total = 2;
        opponent->mana.current = 2;
    } else {
        currentState->mana.total = 2;
        currentState->mana.current = 2;
    }

    if (turnManager.IsPlayerTurn())
        HandleTurnStart();
    else
        RunOpponentTurn();
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
        turnManager.AdvanceCombatStep();
    }
    if (step == CombatStep::RESOLUTION) {
        CombatResult result = board.ResolveCombat(opponent->currentHealth, cardObjects);
        CheckBattleOutcome(result);
        if (!IsBattleOver()) turnManager.AdvancePhase();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Vitória / derrota
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::CheckBattleOutcome(const CombatResult &result) {
    if (result.damageDealt > 0) {
        std::cout << "[BATALHA] Oponente recebeu " << result.damageDealt
                  << " de dano. HP: " << opponent->currentHealth << "/" << opponent->maxHealth
                  << std::endl;
    }
    if (opponent->IsDefeated()) {
        outcome = BattleOutcome::PLAYER_WIN;
        std::cout << "=== JOGADOR VENCEU! ===" << std::endl;
        return;
    }
    if (currentState->currentHealth <= 0) {
        outcome = BattleOutcome::PLAYER_LOSE;
        std::cout << "=== JOGADOR PERDEU! ===" << std::endl;
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Lógica de turno
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleTurnStart() {
    bool gainMana = turnManager.ShouldGainManaThisTurn();
    currentState->mana.OnTurnStart(gainMana);
    std::cout << "[INICIO DE TURNO] Mana: " << currentState->mana.current << "/"
              << currentState->mana.total << (gainMana ? "" : " (1o turno)") << std::endl;

    if (turnManager.ShouldDrawThisTurn()) DrawCards(currentState->GetTurnStartDrawCount());

    turnManager.AdvancePhase();
}

void SceneBattle::RunOpponentTurn() {
    std::cout << "[OPONENTE] Turno automatico..." << std::endl;
    opponent->mana.OnTurnStart(turnManager.ShouldGainManaThisTurn());
    turnManager.AdvancePhase();
    turnManager.AdvancePhase();
    turnManager.AdvancePhase();
    turnManager.AdvancePhase();
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
//  Invocação e sacrifício
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::TrySummonCard(Card *card, std::vector<Card *>::reverse_iterator handIt) {
    // Campo com espaço → invoca diretamente
    if (board.playerPreparationCards.size() < 6) {
        if (!SpendPlayerMana(card->GetManaCost(), card->GetName())) return;

        if (board.AddToPlayerPreparation(card, cardObjects)) {
            hand.erase(std::next(handIt).base());
            RearrangeHand();
        } else {
            // Devolveu a mana pois o board recusou por algum motivo interno
            currentState->mana.current += card->GetManaCost();
        }
        return;
    }

    std::cout << "[INVOCACAO] Campo cheio. Selecione uma carta para sacrificar." << std::endl;
    summonPending.Begin(card);
}

void SceneBattle::ConfirmSummon() {
    if (!summonPending.active || !summonPending.cardToSummon) {
        CancelSummon();
        return;
    }

    if (!summonPending.cardToSacrifice) {
        std::cout << "[INVOCACAO] Selecione uma carta para sacrificar primeiro." << std::endl;
        return;
    }

    Card *toSummon = summonPending.cardToSummon;
    Card *toSacrifice = summonPending.cardToSacrifice;

    if (!SpendPlayerMana(toSummon->GetManaCost(), toSummon->GetName())) {
        CancelSummon();
        return;
    }

    auto &prep = board.playerPreparationCards;
    auto it = std::find(prep.begin(), prep.end(), toSacrifice);
    if (it != prep.end()) {
        prep.erase(it);
        discardPile.push_back(toSacrifice);
        std::cout << "[SACRIFICIO] " << toSacrifice->GetName() << " foi enviada ao cemiterio."
                  << std::endl;
    }

    auto handIt = std::find(hand.begin(), hand.end(), toSummon);
    if (handIt != hand.end()) hand.erase(handIt);

    board.AddToPlayerPreparation(toSummon, cardObjects);
    RearrangeHand();
    summonPending.Clear();
    std::cout << "[INVOCACAO] " << toSummon->GetName() << " invocada com sucesso." << std::endl;
}

void SceneBattle::CancelSummon() {
    if (!summonPending.active) return;
    std::cout << "[INVOCACAO] Invocacao cancelada. Carta permanece na mao." << std::endl;
    summonPending.Clear();
}

// Reorganiza as cartas na mão após remoção
void SceneBattle::RearrangeHand() {
    int n = hand.size(), cw = 100, gap = 15;
    int totalW = n * cw + (n - 1) * gap;
    int startX = playerHandZone.x + (playerHandZone.w - totalW) / 2;
    int y = playerHandZone.y + (playerHandZone.h - 140) / 2;
    for (int i = 0; i < n; ++i)
        hand[i]->SetPosition(startX + i * (cw + gap), y);
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
    turnManager.AdvanceCombatStep();
}

void SceneBattle::HandleCancelAttack() {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return;
    board.ReturnAllAttackersToPreparation(cardObjects);
    std::cout << "[COMBATE] Ataque cancelado." << std::endl;
}

void SceneBattle::HandleConfirmAttack() {
    if (!board.ConfirmAttack()) {
        turnManager.AdvancePhase();
        return;
    }
    turnManager.AdvanceCombatStep();
}

// ═══════════════════════════════════════════════════════════════════
//  HandleInput
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleInput(SDL_Event &event) {
    if (IsBattleOver()) return;
    if (event.type != SDL_MOUSEBUTTONDOWN) return;
    if (summonPending.active) {
        HandleSummonPendingInput(event);
        return;
    }

    if (HandleCancelClick(event)) return;
    if (HandleAttackClick(event)) return;
    if (HandleNextPhaseClick(event)) return;
    if (!turnManager.IsPlayerTurn()) return;
    if (HandleBattleCardClick(event)) return;
    HandleHandCardClick(event);
}

// ── Input do modo sacrifício ──────────────────────────────────────

bool SceneBattle::HandleSummonPendingInput(const SDL_Event &e) {
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;

    if (GameManager::IsPointInsideRect(e.button.x, e.button.y, btnNextPhase)) {
        ConfirmSummon();
        return true;
    }

    if (GameManager::IsPointInsideRect(e.button.x, e.button.y, btnCancel)) {
        CancelSummon();
        return true;
    }

    for (auto *card : board.playerPreparationCards) {
        if (!card) continue;
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;
        if (summonPending.cardToSacrifice == card) {
            summonPending.cardToSacrifice = nullptr;
            std::cout << "[SACRIFICIO] Selecao removida." << std::endl;
        } else {
            summonPending.cardToSacrifice = card;
            std::cout << "[SACRIFICIO] " << card->GetName() << " selecionada." << std::endl;
        }
        return true;
    }

    return false;
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
        turnManager.AdvancePhase();
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
            TrySummonCard(card, it);
        } else if (!isCreature && CanPlaySpell()) {
            if (!SpendPlayerMana(card->GetManaCost(), card->GetName())) return true;
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
    if (summonPending.active) RenderSummonPending(renderer);
    if (IsBattleOver()) RenderOutcome(renderer);
}

void SceneBattle::RenderSummonPending(SDL_Renderer *renderer) const {
    if (!summonPending.active || !summonPending.cardToSummon) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
    SDL_Rect full = {0, 0, 1600, 900};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    {
        Card *c = summonPending.cardToSummon;
        int origX = c->GetX(), origY = c->GetY();
        c->SetPosition(660, 380);
        c->Render(renderer);
        c->SetPosition(origX, origY);
    }

    if (fontUI) {
        TTF_Font *f = fontUI;

        RenderText(renderer, f, "Escolha uma carta para enviar", {255, 220, 80, 255}, 800, 380);
        RenderText(renderer, f, "ao cemiterio", {255, 220, 80, 255}, 800, 425);
        RenderText(renderer, f, "(Cancelar: botao cancelar)", {180, 180, 180, 255}, 800, 490);

        if (summonPending.cardToSacrifice) {
            std::string confirmMsg = "Sacrificar: " + summonPending.cardToSacrifice->GetName();
            RenderText(renderer, f, confirmMsg.c_str(), {255, 80, 80, 255}, 800, 535);

            RenderText(renderer, f, "(Confirmar: botao confirmar)", {120, 255, 120, 255}, 800, 580);
        }
    }

    for (const Card *card : board.playerPreparationCards) {
        if (!card) continue;

        bool isSelected = (summonPending.cardToSacrifice == card);
        SDL_Rect highlight = {card->GetX() - 3, card->GetY() - 3, card->GetWidth() + 6,
                              card->GetHeight() + 6};

        if (isSelected) {
            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255); // vermelho = selecionada
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255); // amarelo = disponível
        }
        SDL_RenderDrawRect(renderer, &highlight);
        SDL_Rect inner = {highlight.x + 1, highlight.y + 1, highlight.w - 2, highlight.h - 2};
        SDL_RenderDrawRect(renderer, &inner);
    }
    RenderButton(renderer, btnNextPhase, 50, 180, 50,
                 summonPending.cardToSacrifice != nullptr); // só ativo se há seleção
    RenderButton(renderer, btnCancel, 80, 80, 200, true);
}

void SceneBattle::RenderText(SDL_Renderer *renderer, TTF_Font *f, const char *text, SDL_Color color,
                             int x, int y) const {
    if (!f || !text) return;
    SDL_Surface *surface = TTF_RenderUTF8_Solid(f, text, color);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dest = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &dest);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
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
    if (IsBattleOver() || summonPending.active) return;

    bool isPlayer = turnManager.IsPlayerTurn();
    auto step = turnManager.GetCombatStep();

    RenderButton(renderer, btnNextPhase, 220, 160, 0, isPlayer);

    if (board.ShouldShowAttackButton())
        RenderButton(renderer, btnAttack, 200, 50, 50, board.CanDeclareAttack());

    if (step == CombatStep::ATTACK_MAGIC && isPlayer)
        RenderButton(renderer, btnCancel, 80, 80, 200, true);
}

void SceneBattle::RenderHealthBars(SDL_Renderer *renderer) const {
    if (!currentState || !opponent || !fontSmall) return;

    char hpText[64];

    // Oponente
    snprintf(hpText, sizeof(hpText), "Oponente: %d/%d", opponent->currentHealth,
             opponent->maxHealth);
    SDL_Color oppColor =
        opponent->isGuardian ? SDL_Color{220, 130, 30, 255} : SDL_Color{200, 50, 50, 255};
    RenderText(renderer, fontSmall, hpText, oppColor, 10, 90);

    // Jogador
    snprintf(hpText, sizeof(hpText), "Voce: %d/%d", currentState->currentHealth,
             currentState->maxHealth);
    RenderText(renderer, fontSmall, hpText, {50, 200, 80, 255}, 10, 870);
}

void SceneBattle::RenderMana(SDL_Renderer *renderer) const {
    if (!currentState || !opponent) return;

    const ManaState &pm = currentState->mana;
    const ManaState &om = opponent->mana;

    auto clamp = [](int v) { return v < 0 ? 0 : v > 99 ? 99 : v; };

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
        value = clamp(value);
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

void SceneBattle::RenderOutcome(SDL_Renderer *renderer) const {
    if (!font) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, 1600, 900};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    const char *text = (outcome == BattleOutcome::PLAYER_WIN) ? "VITORIA!" : "DERROTA!";
    SDL_Color color = (outcome == BattleOutcome::PLAYER_WIN) ? SDL_Color{30, 255, 60, 255}
                                                             : SDL_Color{255, 50, 50, 255};

    SDL_Surface *surface = TTF_RenderUTF8_Solid(font, text, color);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dest = {(1600 - surface->w) / 2, (900 - surface->h) / 2, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &dest);
        SDL_DestroyTexture(texture);
    }
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
    for (auto *obj : objects)
        delete obj;
    objects.clear();
    cardObjects.clear();
    drawPile.clear();
    hand.clear();
    discardPile.clear();
    board.Reset();
    summonPending.Clear();

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
            cardObjects.erase(std::remove(cardObjects.begin(), cardObjects.end(), card),
                              cardObjects.end());
            delete card;
        } else {
            hand.push_back(card);
            if (renderer) card->LoadTexture(renderer);
            std::cout << "Carta comprada: " << card->GetName() << std::endl;
        }
    }

    RearrangeHand();
}