#include "SceneBattle.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include "../objects/cards/SpellCard.hpp"
#include "../core/GameManager.hpp"
#include "../logic/Player.hpp"
#include "../logic/CardFactory.hpp"
#include <algorithm>
#include <iostream>
#include <random>

// ═══════════════════════════════════════════════════════════════════
//  Construtor / Destrutor
// ═══════════════════════════════════════════════════════════════════

SceneBattle::SceneBattle() {
    draggedCard = nullptr;
}

SceneBattle::~SceneBattle() {}

// ═══════════════════════════════════════════════════════════════════
//  Initialize
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::Initialize() {
    std::cout << "A inicializar a Cena de Batalha..." << std::endl;

    if (!cardDatabase.LoadFromJson("assets/data/cards.json")) {
        std::cerr << "Falha ao carregar a base de dados de cartas." << std::endl;
    }

    // Zonas do tabuleiro
    enemyPreparationZone  = { 440,  25, 720, 150 };
    enemyBattleZone       = { 440, 200, 720, 150 };
    playerBattleZone      = { 440, 488, 720, 150 };
    playerPreparationZone = { 440, 663, 720, 150 };
    playerHandZone        = {   0, 740, 1600, 188 };

    // Botões
    btnBuyCard   = { 1420, 740, 150,  50 };
    btnNextPhase = { 1420, 660, 150,  50 };   // acima do botão de compra

    // ── Registrar callbacks do TurnManager ────────────────────────
    turnManager.SetOnPhaseChanged([this](TurnOwner owner, BattlePhase phase) {
        OnPhaseChanged(owner, phase);
    });
    turnManager.SetOnTurnChanged([this](TurnOwner owner) {
        OnTurnChanged(owner);
    });
}

// ═══════════════════════════════════════════════════════════════════
//  StartBattle
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::StartBattle(Player* playerState) {
    if (!SetCurrentPlayerState(playerState)) return;

    std::cout << "Montando o Battle Deck a partir do Master Deck...\n";
    ResetBattleDeckState();
    BuildDrawPileFromMasterDeck();
    ShuffleDrawPile();
    DrawCards(5);

    // ── Sorteio: quem começa? ─────────────────────────────────────
    srand(static_cast<unsigned>(SDL_GetTicks()));
    turnManager.RollForFirstTurn();

    std::cout << "=== SORTEIO: " << turnManager.GetOwnerName()
              << " comeca! ===" << std::endl;

    // Se o oponente tirou o primeiro turno, ele já joga automaticamente
    if (!turnManager.IsPlayerTurn()) {
        RunOpponentTurn();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Callbacks de turno / fase
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::OnPhaseChanged(TurnOwner owner, BattlePhase phase) {
    std::cout << "[TURNO] " << (owner == TurnOwner::PLAYER ? "Jogador" : "Oponente")
              << " — Fase: " << turnManager.GetPhaseName() << std::endl;
}

void SceneBattle::OnTurnChanged(TurnOwner owner) {
    std::cout << "=== TURNO passou para: "
              << (owner == TurnOwner::PLAYER ? "JOGADOR" : "OPONENTE")
              << " ===" << std::endl;

    // Se agora é turno do oponente, ele passa tudo automaticamente
    if (owner == TurnOwner::OPPONENT) {
        RunOpponentTurn();
    }
}

// Oponente sem IA: apenas avança as 3 fases automaticamente
void SceneBattle::RunOpponentTurn() {
    std::cout << "[OPONENTE] Passando todas as fases automaticamente..." << std::endl;

    // Fase PREPARE → BATTLE → END → passa turno de volta ao jogador
    while (!turnManager.IsPlayerTurn()) {
        turnManager.AdvancePhase();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  HandleInput
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleInput(SDL_Event& event) {
    // Botão "Passar Fase" — sempre disponível para o jogador
    if (IsNextPhaseButtonClick(event)) {
        HandleNextPhaseAction();
        return;
    }

    // Ações de carta só fazem sentido no turno do jogador
    if (!turnManager.IsPlayerTurn()) return;

    if (IsBuyCardButtonClick(event)) {
        HandleBuyCardAction();
        return;
    }

    if (HandleHandCardAction(event)) return;

    HandlePreparationCardAction(event);
}

// ═══════════════════════════════════════════════════════════════════
//  Ação: Passar Fase
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::IsNextPhaseButtonClick(const SDL_Event& event) const {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) return false;
    return GameManager::IsPointInsideRect(event.button.x, event.button.y, btnNextPhase);
}

void SceneBattle::HandleNextPhaseAction() {
    // Só o jogador aperta o botão; o oponente já passa automaticamente
    if (!turnManager.IsPlayerTurn()) return;

    std::cout << "[JOGADOR] Passando de fase: "
              << turnManager.GetPhaseName() << " ..." << std::endl;

    bool sameTurn = turnManager.AdvancePhase();

    // Se o turno foi para o oponente, RunOpponentTurn() já foi chamado
    // via callback OnTurnChanged — nada mais a fazer aqui.
    (void)sameTurn;
}

// ═══════════════════════════════════════════════════════════════════
//  Update / Render
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::Update(float dt) {
    for (auto obj : objects) {
        obj->Update(dt);
    }
}

void SceneBattle::Render(SDL_Renderer* renderer) {
    // ── Zonas ────────────────────────────────────────────────────
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &enemyPreparationZone);
    SDL_RenderDrawRect(renderer, &enemyBattleZone);
    SDL_RenderDrawRect(renderer, &playerBattleZone);
    SDL_RenderDrawRect(renderer, &playerPreparationZone);

    // ── Botão Comprar ─────────────────────────────────────────────
    RenderButton(renderer, btnBuyCard, 0, 200, 0);

    // ── Botão Passar Fase ─────────────────────────────────────────
    // Verde quando é turno do jogador, cinza quando não é
    if (turnManager.IsPlayerTurn()) {
        RenderButton(renderer, btnNextPhase, 220, 160, 0);   // amarelo-ouro
    } else {
        RenderButton(renderer, btnNextPhase, 80, 80, 80);    // cinza (desabilitado)
    }

    // ── Cartas ───────────────────────────────────────────────────
    for (auto* card : playerPreparationCards) card->Render(renderer);
    for (auto* card : playerBattleCards)      card->Render(renderer);
    for (auto* card : hand)                   card->Render(renderer);

    // ── HUD de turno ─────────────────────────────────────────────
    RenderTurnInfo(renderer);
}

// Renderiza um retângulo colorido simples como botão
void SceneBattle::RenderButton(SDL_Renderer* renderer, SDL_Rect rect,
                                Uint8 r, Uint8 g, Uint8 b) const {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

// HUD: retângulo no canto superior esquerdo indicando turno e fase
void SceneBattle::RenderTurnInfo(SDL_Renderer* renderer) const {
    // Fundo do painel
    SDL_Rect panel = { 10, 10, 200, 60 };

    if (turnManager.IsPlayerTurn()) {
        SDL_SetRenderDrawColor(renderer, 30, 80, 180, 220);  // azul = jogador
    } else {
        SDL_SetRenderDrawColor(renderer, 180, 30, 30, 220);  // vermelho = oponente
    }
    SDL_RenderFillRect(renderer, &panel);

    // Borda
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &panel);

    // Indicador de fase atual como sub-barras (3 quadradinhos)
    // Fase 0 = PREPARE, 1 = BATTLE, 2 = END
    int phaseIdx = static_cast<int>(turnManager.GetPhase());
    for (int i = 0; i < 3; ++i) {
        SDL_Rect pip = { 20 + i * 30, 50, 20, 10 };
        if (i <= phaseIdx) {
            SDL_SetRenderDrawColor(renderer, 255, 220, 50, 255);  // ativo
        } else {
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);    // inativo
        }
        SDL_RenderFillRect(renderer, &pip);
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Helpers de input existentes
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::IsBuyCardButtonClick(const SDL_Event& event) const {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) return false;
    return GameManager::IsPointInsideRect(event.button.x, event.button.y, btnBuyCard);
}

bool SceneBattle::IsHandCardClick(const SDL_Event& event, const Card* card) const {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT || !card) return false;
    SDL_Rect cardRect = { card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight() };
    return GameManager::IsPointInsideRect(event.button.x, event.button.y, cardRect);
}

bool SceneBattle::IsPreparationCardClick(const SDL_Event& event, const Card* card) const {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT || !card) return false;
    SDL_Rect cardRect = { card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight() };
    return GameManager::IsPointInsideRect(event.button.x, event.button.y, cardRect);
}

void SceneBattle::HandleBuyCardAction() {
    std::cout << "Botao clicado! Puxando a proxima carta do jogador..." << std::endl;
    DrawCards(1);
}

bool SceneBattle::HandleHandCardAction(const SDL_Event& event) {
    for (auto it = hand.rbegin(); it != hand.rend(); ++it) {
        Card* card = *it;
        if (!IsHandCardClick(event, card)) continue;
        if (!AddCardToPlayerPreparation(card)) return true;
        hand.erase(std::next(it).base());
        OrganizeZone(hand, playerHandZone);
        return true;
    }
    return false;
}

bool SceneBattle::HandlePreparationCardAction(const SDL_Event& event) {
    for (auto it = playerPreparationCards.rbegin(); it != playerPreparationCards.rend(); ++it) {
        Card* card = *it;
        if (!IsPreparationCardClick(event, card)) continue;
        if (!AddCardToPlayerBattle(card)) return true;
        playerPreparationCards.erase(std::next(it).base());
        OrganizeZone(playerPreparationCards, playerPreparationZone);
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════
//  Construção do deck
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::SetCurrentPlayerState(Player* playerState) {
    if (!playerState) {
        std::cout << "Estado do jogador invalido." << std::endl;
        return false;
    }
    currentState = playerState;
    return true;
}

void SceneBattle::ResetBattleDeckState() {
    drawPile.clear();
    hand.clear();
    discardPile.clear();
}

void SceneBattle::AddDeckCardToDrawPile(const std::string& cardId) {
    int creatureId = 0;
    try { creatureId = std::stoi(cardId); }
    catch (...) {
        std::cout << "ID de carta invalido no deck: " << cardId << std::endl;
        return;
    }

    CreatureCard* createdCard = CardFactory::CreateCreatureCard(cardDatabase, creatureId);
    if (createdCard) {
        createdCard->SetPosition(-200, -200);
        drawPile.push_back(createdCard);
        objects.push_back(createdCard);
    }
}

void SceneBattle::BuildDrawPileFromMasterDeck() {
    for (const std::string& cardId : currentState->masterDeck) {
        AddDeckCardToDrawPile(cardId);
    }
}

void SceneBattle::ShuffleDrawPile() {
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::shuffle(drawPile.begin(), drawPile.end(), rng);
}

void SceneBattle::DrawCards(int amount) {
    for (int i = 0; i < amount; ++i) {
        if (drawPile.empty()) {
            std::cout << "Pilha de compra vazia!\n";
            break;
        }
        Card* drawnCard = drawPile.back();
        drawPile.pop_back();
        hand.push_back(drawnCard);
    }
    OrganizeZone(hand, playerHandZone);
}

// ═══════════════════════════════════════════════════════════════════
//  Lógica de campo
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::OrganizeZone(std::vector<Card*>& zoneCards, SDL_Rect zoneRect) {
    int numCards = zoneCards.size();
    if (numCards == 0) return;

    int cardWidth  = 100;
    int cardHeight = 140;
    int gap        = 15;

    int totalWidth = (numCards * cardWidth) + ((numCards - 1) * gap);
    int startX     = (zoneRect.x + zoneRect.w / 2) - totalWidth / 2;
    int yPos       = zoneRect.y + (zoneRect.h - cardHeight) / 2;

    for (int i = 0; i < numCards; ++i) {
        zoneCards[i]->SetPosition(startX + i * (cardWidth + gap), yPos);
    }
}

bool SceneBattle::AddCardToPlayerPreparation(Card* card) {
    if (playerPreparationCards.size() < 6) {
        playerPreparationCards.push_back(card);
        OrganizeZone(playerPreparationCards, playerPreparationZone);
        std::cout << card->GetName() << " adicionado a Preparacao." << std::endl;
        if (auto* c = dynamic_cast<CreatureCard*>(card))
            std::cout << c->GetAttack() << " ATK / " << c->GetHealth() << " HP." << std::endl;
        return true;
    }
    std::cout << "Preparacao cheia! (max 6)" << std::endl;
    return false;
}

bool SceneBattle::AddCardToPlayerBattle(Card* card) {
    if (playerBattleCards.size() < 6) {
        playerBattleCards.push_back(card);
        OrganizeZone(playerBattleCards, playerBattleZone);
        std::cout << card->GetName() << " adicionado ao campo de Ataque." << std::endl;
        if (auto* c = dynamic_cast<CreatureCard*>(card))
            std::cout << c->GetAttack() << " ATK / " << c->GetHealth() << " HP." << std::endl;
        return true;
    }
    std::cout << "Campo de Ataque cheio! (max 6)" << std::endl;
    return false;
}