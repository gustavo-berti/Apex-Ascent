#include "SceneBattle.hpp"
#include "../core/GameManager.hpp"
#include "../logic/CardFactory.hpp"
#include "../objects/cards/SpellCard.hpp"
#include <algorithm>
#include <iostream>
#include <random>

// ═══════════════════════════════════════════════════════════════════
//  Construtor / Destrutor
// ═══════════════════════════════════════════════════════════════════

SceneBattle::SceneBattle() : draggedCard(nullptr), attackDeclared(false) {}

SceneBattle::~SceneBattle() {}

// ═══════════════════════════════════════════════════════════════════
//  Initialize
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::Initialize() {
    std::cout << "Inicializando SceneBattle..." << std::endl;

    if (!cardDatabase.LoadFromJson("assets/data/cards.json"))
        std::cerr << "Falha ao carregar cards.json" << std::endl;

    const int boardWidth = 1000;
    const int boardX = (1600 - boardWidth) / 2;

    enemyPreparationZone = {boardX, 25, boardWidth, 150};
    enemyBattleZone = {boardX, 200, boardWidth, 150};
    playerBattleZone = {boardX, 413, boardWidth, 150};
    playerPreparationZone = {boardX, 600, boardWidth, 150};
    playerHandZone = {0, 740, 1600, 160};

    btnCancel = {1420, 300, 150, 50};
    btnNextPhase = {1420, 360, 150, 50};
    btnAttack = {1420, 430, 150, 50};

    turnManager.SetOnPhaseChanged([this](TurnOwner o, BattlePhase p) { OnPhaseChanged(o, p); });
    turnManager.SetOnTurnChanged([this](TurnOwner o) { OnTurnChanged(o); });
    turnManager.SetOnCombatStepChanged([this](CombatStep s) { OnCombatStepChanged(s); });
}

// ═══════════════════════════════════════════════════════════════════
//  StartBattle
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::StartBattle(Player *playerState, SDL_Renderer *renderer) {
    if (!SetCurrentPlayerState(playerState)) return;

    ResetBattleDeckState();
    BuildDrawPileFromPlayerDeck();
    ShuffleDrawPile();
    DrawCards(5);

    this->renderer = renderer;

    srand(static_cast<unsigned>(SDL_GetTicks()));
    turnManager.RollForFirstTurn();

    std::cout << "=== SORTEIO: " << turnManager.GetOwnerName() << " comeca! ===" << std::endl;

    if (turnManager.IsPlayerTurn()) {
        HandleTurnStart();
    } else {
        RunOpponentTurn();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Mana helpers
// ═══════════════════════════════════════════════════════════════════

ManaState &SceneBattle::CurrentMana() {
    return turnManager.IsPlayerTurn() ? currentState->mana : opponentMana;
}

bool SceneBattle::SpendPlayerMana(int cost, const std::string &cardName) {
    if (!currentState->mana.CanAfford(cost)) {
        std::cout << "[MANA] Mana insuficiente para jogar " << cardName << " (custo: " << cost
                  << ", disponivel: " << currentState->mana.current << ")" << std::endl;
        return false;
    }
    currentState->mana.Spend(cost);
    std::cout << "[MANA] Gastou " << cost << " de mana em " << cardName
              << " (restante: " << currentState->mana.current << "/" << currentState->mana.total
              << ")" << std::endl;
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Callbacks do TurnManager
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::OnPhaseChanged(TurnOwner owner, BattlePhase phase) {
    std::cout << "[" << turnManager.GetOwnerName() << "] " << turnManager.GetPhaseName()
              << std::endl;

    if (phase == BattlePhase::TURN_START && owner == TurnOwner::PLAYER) {
        HandleTurnStart();
    }
}

void SceneBattle::OnTurnChanged(TurnOwner owner) {
    std::cout << "=== TURNO: " << turnManager.GetOwnerName() << " ===" << std::endl;

    if (owner == TurnOwner::OPPONENT) {
        RunOpponentTurn();
    }
    // Se voltou ao jogador, OnPhaseChanged cuida do TURN_START
}

void SceneBattle::OnCombatStepChanged(CombatStep step) {
    std::cout << "[COMBATE] " << turnManager.GetCombatStepName() << std::endl;

    if (step == CombatStep::DECLARE_DEFENDERS) {
        ResolveDefenders();
    }
    if (step == CombatStep::RESOLUTION) {
        ResolveCombat();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Início de turno
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleTurnStart() {
    bool gainMana = turnManager.ShouldGainManaThisTurn();
    bool drawCard = turnManager.ShouldDrawThisTurn();

    // ── Mana ──────────────────────────────────────────────────────
    currentState->mana.OnTurnStart(gainMana);

    if (gainMana) {
        std::cout << "[INICIO DE TURNO] Mana do jogador: " << currentState->mana.current << "/"
                  << currentState->mana.total << std::endl;
    } else {
        std::cout << "[INICIO DE TURNO] Primeiro turno — mana nao incrementada. ("
                  << currentState->mana.current << "/" << currentState->mana.total << ")"
                  << std::endl;
    }

    // ── Compra ────────────────────────────────────────────────────
    if (drawCard) {
        DrawCards(currentState->GetTurnStartDrawCount());
    } else {
        std::cout << "[INICIO DE TURNO] Primeiro turno — sem compra de carta." << std::endl;
    }

    turnManager.AdvancePhase(); // TURN_START → MAIN
}

// ═══════════════════════════════════════════════════════════════════
//  Oponente automático
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::RunOpponentTurn() {
    std::cout << "[OPONENTE] Processando turno automaticamente..." << std::endl;

    bool gainMana = turnManager.ShouldGainManaThisTurn();
    opponentMana.OnTurnStart(gainMana);

    if (gainMana) {
        std::cout << "[OPONENTE] Mana: " << opponentMana.current << "/" << opponentMana.total
                  << std::endl;
    } else {
        std::cout << "[OPONENTE] Primeiro turno — mana nao incrementada. (" << opponentMana.current
                  << "/" << opponentMana.total << ")" << std::endl;
    }

    turnManager.AdvancePhase(); // TURN_START → MAIN
    turnManager.AdvancePhase(); // MAIN       → COMBAT
    turnManager.AdvancePhase(); // COMBAT     → SECOND_MAIN
    turnManager.AdvancePhase(); // SECOND_MAIN → fim (volta ao jogador)
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

bool SceneBattle::ShowAttackButton() const {
    return !playerPreparationCards.empty() && turnManager.IsPlayerTurn() &&
           turnManager.GetPhase() == BattlePhase::COMBAT && turnManager.CanAttackThisTurn() &&
           (turnManager.GetCombatStep() == CombatStep::DECLARE_ATTACKERS ||
            turnManager.GetCombatStep() == CombatStep::ATTACK_MAGIC);
}

bool SceneBattle::CanDeclareAttack() const {
    return turnManager.IsPlayerTurn() && turnManager.GetPhase() == BattlePhase::COMBAT &&
           turnManager.GetCombatStep() == CombatStep::DECLARE_ATTACKERS &&
           turnManager.CanAttackThisTurn() && !playerPreparationCards.empty();
}

// ═══════════════════════════════════════════════════════════════════
//  Lógica da fase de combate
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleAttackButton() {
    if (!CanDeclareAttack()) return;
    turnManager.AdvanceCombatStep(); // DECLARE_ATTACKERS → ATTACK_MAGIC
    std::cout << "[COMBATE] Selecione os atacantes e confirme (ou cancele)." << std::endl;
}

void SceneBattle::HandleCancelAttack() {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return;
    ReturnSelectedAttackersToPreparation();
    attackDeclared = false;
    std::cout << "[COMBATE] Ataque cancelado." << std::endl;
}

void SceneBattle::ToggleAttackerSelection(Card *card) {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return;

    auto it = std::find(selectedAttackers.begin(), selectedAttackers.end(), card);
    if (it != selectedAttackers.end()) {
        selectedAttackers.erase(it);
        MoveCardFromBattleToPreparation(card);
        std::cout << "[COMBATE] " << card->GetName() << " removido dos atacantes." << std::endl;
    } else {
        if (MoveCardFromPreparationToBattle(card)) {
            selectedAttackers.push_back(card);
            std::cout << "[COMBATE] " << card->GetName() << " adicionado como atacante."
                      << std::endl;
        }
    }
}

bool SceneBattle::MoveCardFromPreparationToBattle(Card *card) {
    if (!card) return false;
    auto it = std::find(playerPreparationCards.begin(), playerPreparationCards.end(), card);
    if (it == playerPreparationCards.end()) return false;
    if (playerBattleCards.size() >= 6) {
        std::cout << "[COMBATE] Campo de batalha cheio!" << std::endl;
        return false;
    }
    playerPreparationCards.erase(it);
    playerBattleCards.push_back(card);
    OrganizeZone(playerPreparationCards, playerPreparationZone);
    OrganizeZone(playerBattleCards, playerBattleZone);
    return true;
}

bool SceneBattle::MoveCardFromBattleToPreparation(Card *card) {
    if (!card) return false;
    auto it = std::find(playerBattleCards.begin(), playerBattleCards.end(), card);
    if (it == playerBattleCards.end()) return false;
    if (playerPreparationCards.size() >= 6) {
        if (!RemoveRightmostPreparationCard()) return false;
    }
    playerBattleCards.erase(it);
    playerPreparationCards.push_back(card);
    OrganizeZone(playerBattleCards, playerBattleZone);
    OrganizeZone(playerPreparationCards, playerPreparationZone);
    return true;
}

void SceneBattle::ReturnSelectedAttackersToPreparation() {
    std::vector<Card *> copy = selectedAttackers;
    for (Card *c : copy)
        MoveCardFromBattleToPreparation(c);
    selectedAttackers.clear();
}

bool SceneBattle::RemoveRightmostPreparationCard() {
    if (playerPreparationCards.empty()) return false;
    auto it = std::max_element(playerPreparationCards.begin(), playerPreparationCards.end(),
                               [](const Card *a, const Card *b) { return a->GetX() < b->GetX(); });
    Card *toDelete = *it;
    std::cout << "[PREPARACAO] Campo cheio, removendo: " << toDelete->GetName() << std::endl;
    playerPreparationCards.erase(it);
    objects.erase(std::remove(objects.begin(), objects.end(), toDelete), objects.end());
    delete toDelete;
    OrganizeZone(playerPreparationCards, playerPreparationZone);
    return true;
}

void SceneBattle::ConfirmAttack() {
    if (selectedAttackers.empty()) {
        std::cout << "[COMBATE] Sem atacantes. Passando fase de combate." << std::endl;
        turnManager.AdvancePhase(); // COMBAT → SECOND_MAIN
        return;
    }
    attackDeclared = true;
    std::cout << "[COMBATE] Ataque declarado com " << selectedAttackers.size() << " criatura(s)!"
              << std::endl;
    turnManager.AdvanceCombatStep(); // ATTACK_MAGIC → DECLARE_DEFENDERS
}

void SceneBattle::ResolveDefenders() {
    std::cout << "[COMBATE] Oponente passou a defesa." << std::endl;
    turnManager.AdvanceCombatStep(); // DECLARE_DEFENDERS → RESOLUTION
}

void SceneBattle::ResolveCombat() {
    std::cout << "[COMBATE] Resolucao (implementar depois)." << std::endl;
    ReturnSelectedAttackersToPreparation();
    attackDeclared = false;
    turnManager.AdvancePhase(); // COMBAT → SECOND_MAIN
}

// ═══════════════════════════════════════════════════════════════════
//  HandleInput
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleInput(SDL_Event &event) {
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
        ConfirmAttack();
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
    if (!ShowAttackButton()) return false;
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

        CreatureCard *creature = dynamic_cast<CreatureCard *>(card);
        bool isCreature = (creature != nullptr);

        if (isCreature && CanPlayCreature()) {
            if (!SpendPlayerMana(card->GetManaCost(), card->GetName())) return true;
            if (AddCardToPlayerPreparation(card)) {
                hand.erase(std::next(it).base());
                OrganizeZone(hand, playerHandZone);
            } else {
                currentState->mana.current += card->GetManaCost();
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

    for (auto *card : playerBattleCards) {
        if (!card) continue;
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;
        ToggleAttackerSelection(card);
        return true;
    }
    for (auto *card : playerPreparationCards) {
        if (!card) continue;
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;
        ToggleAttackerSelection(card);
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
    RenderZones(renderer);
    RenderButtons(renderer);
    RenderCards(renderer);
    RenderMana(renderer);
    RenderHUD(renderer);
}

void SceneBattle::RenderZones(SDL_Renderer *renderer) const {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &enemyPreparationZone);
    SDL_RenderDrawRect(renderer, &enemyBattleZone);
    SDL_RenderDrawRect(renderer, &playerBattleZone);
    SDL_RenderDrawRect(renderer, &playerPreparationZone);

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawLine(renderer, playerBattleZone.x, 387, playerBattleZone.x + playerBattleZone.w,
                       387);
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
    bool isPlayer = turnManager.IsPlayerTurn();
    auto step = turnManager.GetCombatStep();

    RenderButton(renderer, btnNextPhase, 220, 160, 0, isPlayer);

    if (ShowAttackButton()) RenderButton(renderer, btnAttack, 200, 50, 50, CanDeclareAttack());

    if (step == CombatStep::ATTACK_MAGIC && isPlayer)
        RenderButton(renderer, btnCancel, 80, 80, 200, true);
}

// ── Mana display ──────────────────────────────────────────────────
void SceneBattle::RenderMana(SDL_Renderer *renderer) const {
    if (!currentState) return;

    const ManaState &pm = currentState->mana;
    const ManaState &om = opponentMana;

    auto clampMana = [](int value) {
        if (value < 0) return 0;
        if (value > 99) return 99;
        return value;
    };

    auto drawDigit = [&](int x, int y, int digit, Uint8 r, Uint8 g, Uint8 b) {
        static const bool segments[10][7] = {
            {true, true, true, true, true, true, false},
            {false, true, true, false, false, false, false},
            {true, true, false, true, true, false, true},
            {true, true, true, true, false, false, true},
            {false, true, true, false, false, true, true},
            {true, false, true, true, false, true, true},
            {true, false, true, true, true, true, true},
            {true, true, true, false, false, false, false},
            {true, true, true, true, true, true, true},
            {true, true, true, true, false, true, true},
        };

        const int t = 4;
        const int w = 22;
        const int h = 36;

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
        for (int i = 0; i < 7; ++i) {
            if (segments[digit][i]) SDL_RenderFillRect(renderer, &seg[i]);
        }
    };

    auto drawManaNumber = [&](int value, int x, int y, Uint8 bgR, Uint8 bgG, Uint8 bgB, Uint8 fgR,
                              Uint8 fgG, Uint8 fgB) {
        value = clampMana(value);
        int tens = value / 10;
        int units = value % 10;

        SDL_Rect bg = {x - 8, y - 8, 62, 52};
        SDL_SetRenderDrawColor(renderer, bgR, bgG, bgB, 210);
        SDL_RenderFillRect(renderer, &bg);
        SDL_SetRenderDrawColor(renderer, fgR, fgG, fgB, 255);
        SDL_RenderDrawRect(renderer, &bg);

        if (tens > 0) drawDigit(x, y, tens, fgR, fgG, fgB);
        drawDigit(x + 28, y, units, fgR, fgG, fgB);
    };

    drawManaNumber(pm.current, 18, 760, 10, 24, 70, 100, 200, 255);
    drawManaNumber(om.current, 1512, 16, 70, 12, 12, 255, 120, 120);
}

void SceneBattle::RenderCards(SDL_Renderer *renderer) const {
    for (auto *c : enemyPreparationCards)
        c->Render(renderer);
    for (auto *c : enemyBattleCards)
        c->Render(renderer);
    for (auto *c : playerBattleCards)
        c->Render(renderer);
    for (auto *c : playerPreparationCards)
        c->Render(renderer);
    for (auto *c : hand)
        c->Render(renderer);
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

// ═══════════════════════════════════════════════════════════════════
//  Campo
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::OrganizeZone(std::vector<Card *> &zone, SDL_Rect rect) {
    int n = zone.size();
    if (n == 0) return;
    int cw = 100, ch = 140, gap = 15;
    int totalW = n * cw + (n - 1) * gap;
    int startX = rect.x + (rect.w - totalW) / 2;
    int y = rect.y + (rect.h - ch) / 2;
    for (int i = 0; i < n; ++i)
        zone[i]->SetPosition(startX + i * (cw + gap), y);
}

bool SceneBattle::AddCardToPlayerBattle(Card *card) {
    if (playerBattleCards.size() >= 6) {
        std::cout << "Campo de batalha cheio! (max 6)" << std::endl;
        return false;
    }
    playerBattleCards.push_back(card);
    OrganizeZone(playerBattleCards, playerBattleZone);
    std::cout << card->GetName() << " entrou no campo de batalha." << std::endl;
    return true;
}

bool SceneBattle::AddCardToPlayerPreparation(Card *card) {
    if (playerPreparationCards.size() >= 6) {
        if (!RemoveRightmostPreparationCard()) {
            std::cout << "Campo de preparacao cheio! (max 6)" << std::endl;
            return false;
        }
    }
    playerPreparationCards.push_back(card);
    OrganizeZone(playerPreparationCards, playerPreparationZone);
    std::cout << card->GetName() << " entrou no campo de preparacao." << std::endl;
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Deck
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::SetCurrentPlayerState(Player *p) {
    if (!p) {
        std::cout << "Player invalido." << std::endl;
        return false;
    }
    currentState = p;
    return true;
}

void SceneBattle::ResetBattleDeckState() {
    // ── Pilhas de cartas ──────────────────────────────────────────
    drawPile.clear();
    hand.clear();
    discardPile.clear();
    playerPreparationCards.clear();
    playerBattleCards.clear();
    enemyPreparationCards.clear();
    enemyBattleCards.clear();
    selectedAttackers.clear();
    attackDeclared = false;

    if (currentState) currentState->mana = ManaState{};
    opponentMana = ManaState{};
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
        if (!card) {
            std::cout << "Falha ao instanciar criatura ID " << id << std::endl;
            return;
        }
    } else if (const SpellData *spellData = cardDatabase.GetSpell(id)) {
        card = new SpellCard(spellData, -200, -200);
    } else {
        std::cout << "Carta ID " << id << " nao encontrada no banco (criatura/magia)."
                  << std::endl;
        return;
    }

    card->SetPosition(-200, -200);
    drawPile.push_back(card);
    objects.push_back(card);
}

void SceneBattle::BuildDrawPileFromPlayerDeck() {
    if (!currentState) return;
    const auto &deck = currentState->GetMasterDeck();
    std::cout << "[DECK] Montando draw pile com " << deck.size() << " carta(s)." << std::endl;

    for (const auto &id : deck)
        AddDeckCardToDrawPile(id);

    std::cout << "[DECK] Draw pile final: " << drawPile.size() << " carta(s)." << std::endl;
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
            delete card;
        } else {
            hand.push_back(card);
            std::cout << "Carta comprada: " << card->GetName() << std::endl;
        }

        card->LoadTexture(renderer);
    }
    OrganizeZone(hand, playerHandZone);
}