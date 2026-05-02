#include "Board.hpp"
#include <algorithm>
#include <iostream>

// ═══════════════════════════════════════════════════════════════════
//  Construtor
// ═══════════════════════════════════════════════════════════════════

Board::Board(TurnManager &tm) : turnManager(tm) {}

// ═══════════════════════════════════════════════════════════════════
//  Configuração
// ═══════════════════════════════════════════════════════════════════

void Board::SetZoneRects(SDL_Rect playerPrep, SDL_Rect playerBattle, SDL_Rect enemyPrep,
                         SDL_Rect enemyBattle) {
    playerPreparationRect = playerPrep;
    playerBattleRect = playerBattle;
    enemyPreparationRect = enemyPrep;
    enemyBattleRect = enemyBattle;
}

// ═══════════════════════════════════════════════════════════════════
//  Reset
// ═══════════════════════════════════════════════════════════════════

void Board::Reset() {
    // Não destrói as cartas — isso é responsabilidade de quem gere objects.
    // Apenas limpa as referências das zonas.
    playerPreparationCards.clear();
    playerBattleCards.clear();
    enemyPreparationCards.clear();
    enemyBattleCards.clear();
    selectedAttackers.clear();
    attackDeclared = false;
}

// ═══════════════════════════════════════════════════════════════════
//  Organização de zona
// ═══════════════════════════════════════════════════════════════════

void Board::OrganizeZone(std::vector<Card *> &zone, const SDL_Rect &rect) {
    int n = static_cast<int>(zone.size());
    if (n == 0) return;

    const int cw = 100;
    const int ch = 140;
    const int gap = 15;

    int totalW = n * cw + (n - 1) * gap;
    int startX = rect.x + (rect.w - totalW) / 2;
    int y = rect.y + (rect.h - ch) / 2;

    for (int i = 0; i < n; ++i)
        zone[i]->SetPosition(startX + i * (cw + gap), y);
}

// ═══════════════════════════════════════════════════════════════════
//  Remoção da carta mais à direita (libera espaço)
// ═══════════════════════════════════════════════════════════════════

bool Board::RemoveRightmostCard(std::vector<Card *> &zone, const SDL_Rect &rect,
                                std::vector<GameObject *> &objectsPool) {
    if (zone.empty()) return false;

    auto it = std::max_element(zone.begin(), zone.end(),
                               [](const Card *a, const Card *b) { return a->GetX() < b->GetX(); });

    Card *toDelete = *it;
    std::cout << "[BOARD] Campo cheio, removendo carta mais a direita: " << toDelete->GetName()
              << std::endl;

    zone.erase(it);
    objectsPool.erase(std::remove(objectsPool.begin(), objectsPool.end(), toDelete),
                      objectsPool.end());
    delete toDelete;

    OrganizeZone(zone, rect);
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Adição de cartas nas zonas
// ═══════════════════════════════════════════════════════════════════

bool Board::AddToPlayerPreparation(Card *card, std::vector<GameObject *> &objectsPool) {
    if (playerPreparationCards.size() >= 6) {
        std::cout << "[BOARD] Campo de preparacao cheio! (max 6)" << std::endl;
        return false;
    }
    playerPreparationCards.push_back(card);
    OrganizeZone(playerPreparationCards, playerPreparationRect);
    std::cout << "[BOARD] " << card->GetName() << " entrou na preparacao." << std::endl;
    return true;
}

bool Board::AddToPlayerBattle(Card *card) {
    if (playerBattleCards.size() >= 6) {
        std::cout << "[BOARD] Campo de batalha cheio! (max 6)" << std::endl;
        return false;
    }
    playerBattleCards.push_back(card);
    OrganizeZone(playerBattleCards, playerBattleRect);
    std::cout << "[BOARD] " << card->GetName() << " entrou no campo de batalha." << std::endl;
    return true;
}

bool Board::AddToEnemyPreparation(Card *card) {
    if (enemyPreparationCards.size() >= 6) return false;
    enemyPreparationCards.push_back(card);
    OrganizeZone(enemyPreparationCards, enemyPreparationRect);
    return true;
}

bool Board::AddToEnemyBattle(Card *card) {
    if (enemyBattleCards.size() >= 6) return false;
    enemyBattleCards.push_back(card);
    OrganizeZone(enemyBattleCards, enemyBattleRect);
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Movimentação interna
// ═══════════════════════════════════════════════════════════════════

bool Board::MoveFromPreparationToBattle(Card *card) {
    if (!card) return false;

    auto it = std::find(playerPreparationCards.begin(), playerPreparationCards.end(), card);
    if (it == playerPreparationCards.end()) return false;

    if (playerBattleCards.size() >= 6) {
        std::cout << "[BOARD] Campo de batalha cheio, nao pode atacar." << std::endl;
        return false;
    }

    playerPreparationCards.erase(it);
    playerBattleCards.push_back(card);

    OrganizeZone(playerPreparationCards, playerPreparationRect);
    OrganizeZone(playerBattleCards, playerBattleRect);
    return true;
}

bool Board::MoveFromBattleToPreparation(Card *card, std::vector<GameObject *> &objectsPool) {
    if (!card) return false;

    auto it = std::find(playerBattleCards.begin(), playerBattleCards.end(), card);
    if (it == playerBattleCards.end()) return false;

    // Se a preparação estiver cheia, não há espaço para voltar
    if (playerPreparationCards.size() >= 6) {
        std::cout << "[BOARD] Preparacao cheia, carta nao pode voltar." << std::endl;
        return false;
    }

    playerBattleCards.erase(it);
    playerPreparationCards.push_back(card);

    OrganizeZone(playerBattleCards, playerBattleRect);
    OrganizeZone(playerPreparationCards, playerPreparationRect);
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Seleção de atacantes
// ═══════════════════════════════════════════════════════════════════

void Board::ToggleAttackerSelection(Card *card, std::vector<GameObject *> &objectsPool) {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return;

    auto it = std::find(selectedAttackers.begin(), selectedAttackers.end(), card);

    if (it != selectedAttackers.end()) {
        // Já estava selecionado → desseleciona e volta para preparação
        selectedAttackers.erase(it);
        MoveFromBattleToPreparation(card, objectsPool);
        std::cout << "[BOARD] " << card->GetName() << " removido dos atacantes." << std::endl;
    } else {
        // Não estava selecionado → move para batalha e adiciona à lista
        if (MoveFromPreparationToBattle(card)) {
            selectedAttackers.push_back(card);
            std::cout << "[BOARD] " << card->GetName() << " adicionado como atacante." << std::endl;
        }
    }
}

void Board::ReturnAllAttackersToPreparation(std::vector<GameObject *> &objectsPool) {
    // Copia para evitar invalidar iteração durante remoção
    std::vector<Card *> copy = selectedAttackers;
    for (Card *c : copy)
        MoveFromBattleToPreparation(c, objectsPool);
    selectedAttackers.clear();
}

// ═══════════════════════════════════════════════════════════════════
//  Lógica de combate
// ═══════════════════════════════════════════════════════════════════

bool Board::ConfirmAttack() {
    if (selectedAttackers.empty()) {
        std::cout << "[BOARD] Nenhum atacante selecionado." << std::endl;
        return false;
    }
    attackDeclared = true;
    std::cout << "[BOARD] Ataque declarado com " << selectedAttackers.size() << " criatura(s)!"
              << std::endl;
    return true;
}

void Board::ResolveDefenders() {
    // Oponente sem IA: passa automaticamente sem defender
    std::cout << "[BOARD] Oponente passou a defesa." << std::endl;
}

void Board::ResolveCombat(std::vector<GameObject *> &objectsPool) {
    // Placeholder: nenhum dano calculado ainda.
    // Atacantes voltam para a preparação após o combate.
    std::cout << "[BOARD] Resolucao do combate (implementar depois)." << std::endl;
    ReturnAllAttackersToPreparation(objectsPool);
    attackDeclared = false;
}

// ═══════════════════════════════════════════════════════════════════
//  Consultas de estado
// ═══════════════════════════════════════════════════════════════════

bool Board::IsCardSelectedAsAttacker(const Card *card) const {
    return std::find(selectedAttackers.begin(), selectedAttackers.end(), card) !=
           selectedAttackers.end();
}

bool Board::ShouldShowAttackButton() const {
    return !playerPreparationCards.empty() && turnManager.IsPlayerTurn() &&
           turnManager.GetPhase() == BattlePhase::COMBAT && turnManager.CanAttackThisTurn() &&
           (turnManager.GetCombatStep() == CombatStep::DECLARE_ATTACKERS ||
            turnManager.GetCombatStep() == CombatStep::ATTACK_MAGIC);
}

bool Board::CanDeclareAttack() const {
    return turnManager.IsPlayerTurn() && turnManager.GetPhase() == BattlePhase::COMBAT &&
           turnManager.GetCombatStep() == CombatStep::DECLARE_ATTACKERS &&
           turnManager.CanAttackThisTurn() && !playerPreparationCards.empty();
}

// ═══════════════════════════════════════════════════════════════════
//  Render
// ═══════════════════════════════════════════════════════════════════

void Board::RenderZoneBorders(SDL_Renderer *renderer) const {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &enemyPreparationRect);
    SDL_RenderDrawRect(renderer, &enemyBattleRect);
    SDL_RenderDrawRect(renderer, &playerBattleRect);
    SDL_RenderDrawRect(renderer, &playerPreparationRect);

    // Linha divisória no centro do tabuleiro
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawLine(renderer, playerBattleRect.x, 387, playerBattleRect.x + playerBattleRect.w,
                       387);
}

void Board::Render(SDL_Renderer *renderer) const {
    RenderZoneBorders(renderer);

    for (auto *c : enemyPreparationCards)
        c->Render(renderer);
    for (auto *c : enemyBattleCards)
        c->Render(renderer);
    for (auto *c : playerBattleCards)
        c->Render(renderer);
    for (auto *c : playerPreparationCards)
        c->Render(renderer);
}