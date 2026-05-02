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

    const int cw = 100, ch = 140, gap = 15;
    int totalW = n * cw + (n - 1) * gap;
    int startX = rect.x + (rect.w - totalW) / 2;
    int y = rect.y + (rect.h - ch) / 2;

    for (int i = 0; i < n; ++i)
        zone[i]->SetPosition(startX + i * (cw + gap), y);
}

// ═══════════════════════════════════════════════════════════════════
//  Remoção da carta mais à direita
// ═══════════════════════════════════════════════════════════════════

bool Board::RemoveRightmostCard(std::vector<Card *> &zone, const SDL_Rect &rect,
                                std::vector<Card *> &objectsPool) {
    if (zone.empty()) return false;

    auto it = std::max_element(zone.begin(), zone.end(),
                               [](const Card *a, const Card *b) { return a->GetX() < b->GetX(); });

    Card *toDelete = *it;
    std::cout << "[BOARD] Campo cheio, removendo: " << toDelete->GetName() << std::endl;

    zone.erase(it);
    objectsPool.erase(std::remove(objectsPool.begin(), objectsPool.end(), toDelete),
                      objectsPool.end());
    delete toDelete;

    OrganizeZone(zone, rect);
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Adição de cartas
// ═══════════════════════════════════════════════════════════════════

bool Board::AddToPlayerPreparation(Card *card, std::vector<Card *> &objectsPool) {
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
        std::cout << "[BOARD] Campo de batalha cheio, carta nao pode atacar." << std::endl;
        return false;
    }

    playerPreparationCards.erase(it);
    playerBattleCards.push_back(card);
    OrganizeZone(playerPreparationCards, playerPreparationRect);
    OrganizeZone(playerBattleCards, playerBattleRect);
    return true;
}

bool Board::MoveFromBattleToPreparation(Card *card, std::vector<Card *> &objectsPool) {
    if (!card) return false;

    auto it = std::find(playerBattleCards.begin(), playerBattleCards.end(), card);
    if (it == playerBattleCards.end()) return false;

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

void Board::ToggleAttackerSelection(Card *card, std::vector<Card *> &objectsPool) {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return;

    auto it = std::find(selectedAttackers.begin(), selectedAttackers.end(), card);

    if (it != selectedAttackers.end()) {
        selectedAttackers.erase(it);
        MoveFromBattleToPreparation(card, objectsPool);
        std::cout << "[BOARD] " << card->GetName() << " removido dos atacantes." << std::endl;
    } else {
        if (MoveFromPreparationToBattle(card)) {
            selectedAttackers.push_back(card);
            std::cout << "[BOARD] " << card->GetName() << " adicionado como atacante." << std::endl;
        }
    }
}

void Board::ReturnAllAttackersToPreparation(std::vector<Card *> &objectsPool) {
    std::vector<Card *> copy = selectedAttackers;
    for (Card *c : copy)
        MoveFromBattleToPreparation(c, objectsPool);
    selectedAttackers.clear();
}

// ═══════════════════════════════════════════════════════════════════
//  Combate
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

CombatResult Board::ResolveCombat(int &opponentCurrentHP, std::vector<Card *> &objectsPool) {
    CombatResult result;

    // ── Sem defensores: dano direto ao HP do oponente ─────────────
    // Cada atacante contribui com seu ATK.
    // A lógica de defensor (pareamento, dano excedente, morte de criaturas)
    // será implementada depois quando a interação defensor→atacante estiver pronta.

    if (enemyBattleCards.empty()) {
        for (Card *attacker : selectedAttackers) {
            const CreatureCard *creature = dynamic_cast<const CreatureCard *>(attacker);
            if (!creature) continue;

            int atk = creature->GetAttack();
            result.damageDealt += atk;

            std::cout << "[COMBATE] " << attacker->GetName() << " causou " << atk
                      << " de dano direto." << std::endl;
        }

        opponentCurrentHP -= result.damageDealt;
        if (opponentCurrentHP < 0) opponentCurrentHP = 0;

        result.opponentDied = (opponentCurrentHP <= 0);

        std::cout << "[COMBATE] Dano total: " << result.damageDealt
                  << " | HP restante do oponente: " << opponentCurrentHP << std::endl;
    } else {
        // Defensores presentes: placeholder até a implementação completa
        std::cout << "[COMBATE] Resolucao com defensores (implementar depois)." << std::endl;
    }

    // Atacantes voltam para a preparação após o combate
    ReturnAllAttackersToPreparation(objectsPool);
    attackDeclared = false;

    return result;
}

// ═══════════════════════════════════════════════════════════════════
//  Consultas
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