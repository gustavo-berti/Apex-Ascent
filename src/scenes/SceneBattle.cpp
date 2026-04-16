#include "SceneBattle.hpp"
#include "../core/GameManager.hpp"
#include "../logic/CardFactory.hpp"
#include "../objects/cards/SpellCard.hpp"
#include <algorithm>
#include <iostream>
#include <random>

SceneBattle::SceneBattle() : draggedCard(nullptr), attackDeclared(false) {}

SceneBattle::~SceneBattle() {}

void SceneBattle::Initialize() {
    std::cout << "Inicializando SceneBattle..." << std::endl;

    if (!cardDatabase.LoadFromJson("assets/data/cards.json"))
        std::cerr << "Falha ao carregar cards.json" << std::endl;

    // ── Layout do tabuleiro (1600x900) ────────────────────────────
    //
    //  [enemyPreparationZone ]   y=25
    //  [enemyBattleZone      ]   y=200
    //  ── divisória ──────────   y=387  (centro)
    //  [playerBattleZone     ]   y=413
    //  [playerPreparationZone]   y=600
    //  [playerHandZone       ]   y=740

    const int boardWidth = 1000;
    const int boardX = (1600 - boardWidth) / 2;

    enemyPreparationZone = {boardX, 25, boardWidth, 150};
    enemyBattleZone = {boardX, 200, boardWidth, 150};
    playerBattleZone = {boardX, 413, boardWidth, 150};
    playerPreparationZone = {boardX, 600, boardWidth, 150};
    playerHandZone = {0, 740, 1600, 160};

    // ── Botões (lado direito, na altura da divisória) ─────────────
    btnCancel = {1420, 300, 150, 50};
    btnNextPhase = {1420, 360, 150, 50};
    btnAttack = {1420, 430, 150, 50};

    // ── Registrar callbacks do TurnManager ───────────────────────
    turnManager.SetOnPhaseChanged([this](TurnOwner o, BattlePhase p) { OnPhaseChanged(o, p); });
    turnManager.SetOnTurnChanged([this](TurnOwner o) { OnTurnChanged(o); });
    turnManager.SetOnCombatStepChanged([this](CombatStep s) { OnCombatStepChanged(s); });
}

// ═══════════════════════════════════════════════════════════════════
//  Começo de Batalha
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::StartBattle(Player *playerState) {
    if (!SetCurrentPlayerState(playerState)) return;

    ResetBattleDeckState();
    BuildDrawPileFromPlayerDeck();
    ShuffleDrawPile();

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
//  Lógica de início de turno
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleTurnStart() {
    std::cout << "[INICIO DE TURNO] Restaurando mana e comprando carta..." << std::endl;

    // TODO: restaurar mana e incrementar máximo (implementar junto com mana)

    if (currentState) {
        DrawCards(currentState->GetTurnStartDrawCount());
    }

    turnManager.AdvancePhase();
}

// ═══════════════════════════════════════════════════════════════════
//  Oponente automático
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::RunOpponentTurn() {
    std::cout << "[OPONENTE] Passando turno automaticamente..." << std::endl;
    turnManager.AdvancePhase(); // TURN_START → MAIN
    turnManager.AdvancePhase(); // MAIN → COMBAT
    turnManager.AdvancePhase(); // COMBAT → SECOND_MAIN
    turnManager.AdvancePhase(); // SECOND_MAIN → fim de turno (volta para o jogador)
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
           turnManager.GetPhase() == BattlePhase::COMBAT &&
           (turnManager.GetCombatStep() == CombatStep::DECLARE_ATTACKERS ||
            turnManager.GetCombatStep() == CombatStep::ATTACK_MAGIC);
}

bool SceneBattle::CanDeclareAttack() const {
    return turnManager.IsPlayerTurn() && turnManager.GetPhase() == BattlePhase::COMBAT &&
           turnManager.GetCombatStep() == CombatStep::DECLARE_ATTACKERS &&
           !playerPreparationCards.empty();
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

    std::cout << "[COMBATE] Ataque cancelado. Criaturas voltaram ao estado normal." << std::endl;

    ReturnSelectedAttackersToPreparation();
    attackDeclared = false;
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

    auto prepIt = std::find(playerPreparationCards.begin(), playerPreparationCards.end(), card);
    if (prepIt == playerPreparationCards.end()) return false;

    if (playerBattleCards.size() >= 6) {
        std::cout << "[COMBATE] Campo de batalha cheio! " << card->GetName() << " nao pode atacar."
                  << std::endl;
        return false;
    }

    playerPreparationCards.erase(prepIt);
    playerBattleCards.push_back(card);
    OrganizeZone(playerPreparationCards, playerPreparationZone);
    OrganizeZone(playerBattleCards, playerBattleZone);
    return true;
}

bool SceneBattle::MoveCardFromBattleToPreparation(Card *card) {
    if (!card) return false;

    auto battleIt = std::find(playerBattleCards.begin(), playerBattleCards.end(), card);
    if (battleIt == playerBattleCards.end()) return false;

    if (playerPreparationCards.size() >= 6) {
        if (!RemoveRightmostPreparationCard()) {
            std::cout << "[COMBATE] Falha ao liberar espaco no campo de preparacao." << std::endl;
            return false;
        }
    }

    playerBattleCards.erase(battleIt);
    playerPreparationCards.push_back(card);
    OrganizeZone(playerBattleCards, playerBattleZone);
    OrganizeZone(playerPreparationCards, playerPreparationZone);
    return true;
}

void SceneBattle::ReturnSelectedAttackersToPreparation() {
    // Copia para evitar invalidar iteração caso a movimentação altere estado interno.
    std::vector<Card *> attackers = selectedAttackers;
    for (Card *card : attackers) {
        MoveCardFromBattleToPreparation(card);
    }
    selectedAttackers.clear();
}

bool SceneBattle::RemoveRightmostPreparationCard() {
    if (playerPreparationCards.empty()) return false;

    auto it = std::max_element(playerPreparationCards.begin(), playerPreparationCards.end(),
                               [](const Card *a, const Card *b) {
                                   if (!a || !b) return a < b;
                                   return a->GetX() < b->GetX();
                               });

    Card *toDelete = *it;
    if (!toDelete) {
        playerPreparationCards.erase(it);
        OrganizeZone(playerPreparationCards, playerPreparationZone);
        return true;
    }

    std::cout << "[PREPARACAO] Campo cheio, removendo carta mais a direita: " << toDelete->GetName()
              << std::endl;

    playerPreparationCards.erase(it);
    objects.erase(std::remove(objects.begin(), objects.end(), toDelete), objects.end());
    delete toDelete;

    OrganizeZone(playerPreparationCards, playerPreparationZone);
    return true;
}

void SceneBattle::ConfirmAttack() {
    if (selectedAttackers.empty()) {
        std::cout << "[COMBATE] Nenhum atacante selecionado. Passando fase de combate."
                  << std::endl;
        turnManager.AdvancePhase(); // COMBAT → SECOND_MAIN
        return;
    }

    attackDeclared = true;
    std::cout << "[COMBATE] Ataque declarado com " << selectedAttackers.size() << " criatura(s)!"
              << std::endl;
    turnManager.AdvanceCombatStep(); // ATTACK_MAGIC → DECLARE_DEFENDERS
}

void SceneBattle::ResolveDefenders() {
    // Oponente não tem IA por enquanto: defende com zero criaturas
    std::cout << "[COMBATE] Oponente passou a defesa." << std::endl;
    turnManager.AdvanceCombatStep(); // DECLARE_DEFENDERS → RESOLUTION
}

void SceneBattle::ResolveCombat() {
    std::cout << "[COMBATE] Resolucao do combate (implementar depois)." << std::endl;

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
    if (!turnManager.IsPlayerTurn()) return true; // absorve o clique mas não faz nada

    auto phase = turnManager.GetPhase();
    auto step = turnManager.GetCombatStep();

    if (phase == BattlePhase::COMBAT && step == CombatStep::ATTACK_MAGIC) {
        // Confirma o ataque (ou passa o combate se não selecionou nada)
        ConfirmAttack();
        return true;
    }

    if (phase == BattlePhase::COMBAT && step == CombatStep::DECLARE_ATTACKERS) {
        // Passou o combate sem atacar
        turnManager.AdvancePhase(); // COMBAT → SECOND_MAIN
        return true;
    }

    // Nas demais fases: avança normalmente
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

        bool isCreature = dynamic_cast<CreatureCard *>(card) != nullptr;

        if (isCreature && CanPlayCreature()) {
            if (AddCardToPlayerPreparation(card)) {
                hand.erase(std::next(it).base());
                OrganizeZone(hand, playerHandZone);
            }
        } else if (!isCreature && CanPlaySpell()) {
            // TODO: ativar efeito da magia (pilha de resposta — implementar depois)
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

    // Primeiro tenta desselecionar atacantes já movidos para batalha.
    for (auto *card : playerBattleCards) {
        if (!card) continue;
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;
        ToggleAttackerSelection(card);
        return true;
    }

    // Depois tenta selecionar cartas do campo de preparação para atacar.
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
    RenderHUD(renderer);
}

void SceneBattle::RenderZones(SDL_Renderer *renderer) const {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &enemyPreparationZone);
    SDL_RenderDrawRect(renderer, &enemyBattleZone);
    SDL_RenderDrawRect(renderer, &playerBattleZone);
    SDL_RenderDrawRect(renderer, &playerPreparationZone);

    // Linha divisória entre os campos
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawLine(renderer, playerBattleZone.x, 387, playerBattleZone.x + playerBattleZone.w,
                       387);
}

void SceneBattle::RenderButton(SDL_Renderer *renderer, SDL_Rect r, Uint8 red, Uint8 grn, Uint8 blu,
                               bool enabled) const {
    if (enabled) {
        SDL_SetRenderDrawColor(renderer, red, grn, blu, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255); // cinza = desabilitado
    }
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &r);
}

void SceneBattle::RenderButtons(SDL_Renderer *renderer) const {
    bool isPlayer = turnManager.IsPlayerTurn();
    auto phase = turnManager.GetPhase();
    auto step = turnManager.GetCombatStep();

    // ── Botão Passar Fase / Confirmar ─────────────────────────────
    // Texto visual futuro: "Passar" ou "Confirmar Ataque"
    bool nextEnabled = isPlayer;
    RenderButton(renderer, btnNextPhase, 220, 160, 0, nextEnabled); // amarelo-ouro

    // ── Botão Atacar ──────────────────────────────────────────────
    // Visível quando há criaturas no campo, ativo apenas em DECLARE_ATTACKERS
    if (ShowAttackButton()) {
        bool attackEnabled = CanDeclareAttack();
        RenderButton(renderer, btnAttack, 200, 50, 50, attackEnabled); // vermelho
    }

    // ── Botão Cancelar ────────────────────────────────────────────
    // Visível apenas em ATTACK_MAGIC
    if (step == CombatStep::ATTACK_MAGIC && isPlayer) {
        RenderButton(renderer, btnCancel, 80, 80, 200, true); // azul
    }
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
    // Painel de turno (canto superior esquerdo)
    SDL_Rect panel = {10, 10, 185, 70};
    bool isPlayer = turnManager.IsPlayerTurn();

    SDL_SetRenderDrawColor(renderer, isPlayer ? 30 : 180, isPlayer ? 80 : 30, isPlayer ? 180 : 30,
                           210);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &panel);

    // Indicadores de fase: 4 pips
    int phaseIdx = static_cast<int>(turnManager.GetPhase());
    for (int i = 0; i < 4; ++i) {
        SDL_Rect pip = {15 + i * 44, 60, 36, 10};
        if (i == phaseIdx) {
            SDL_SetRenderDrawColor(renderer, 255, 220, 50, 255); // ativo
        } else {
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255); // inativo
        }
        SDL_RenderFillRect(renderer, &pip);
    }

    // Indicador de sub-passo de combate (visível só na fase de combate)
    if (turnManager.GetPhase() == BattlePhase::COMBAT) {
        int stepIdx =
            static_cast<int>(turnManager.GetCombatStep()); // NONE=0, DA=1, AM=2, DD=3, RES=4
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
    drawPile.clear();
    hand.clear();
    discardPile.clear();
}

void SceneBattle::AddDeckCardToDrawPile(const std::string &cardId) {
    int id = 0;
    try {
        id = std::stoi(cardId);
    } catch (...) {
        std::cout << "ID invalido: " << cardId << std::endl;
        return;
    }

    CreatureCard *card = CardFactory::CreateCreatureCard(cardDatabase, id);
    if (card) {
        card->SetPosition(-200, -200);
        drawPile.push_back(card);
        objects.push_back(card);
    }
}

void SceneBattle::BuildDrawPileFromPlayerDeck() {
    if (!currentState) return;

    for (const auto &id : currentState->GetMasterDeck())
        AddDeckCardToDrawPile(id);
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
    }
    OrganizeZone(hand, playerHandZone);
}