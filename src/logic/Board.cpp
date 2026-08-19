#include "Board.hpp"
#include <algorithm>
#include <iostream>
#include "../core/GameManager.hpp"

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
    defenderAssignments.clear();
    attackDeclared = false;
}

// ═══════════════════════════════════════════════════════════════════
//  Acesso generico as zonas
// ═══════════════════════════════════════════════════════════════════

std::vector<Card *> &Board::PreparationZone(TurnOwner side) {
    return side == TurnOwner::PLAYER ? playerPreparationCards : enemyPreparationCards;
}

std::vector<Card *> &Board::BattleZone(TurnOwner side) {
    return side == TurnOwner::PLAYER ? playerBattleCards : enemyBattleCards;
}

const std::vector<Card *> &Board::PreparationZone(TurnOwner side) const {
    return const_cast<Board *>(this)->PreparationZone(side);
}

const std::vector<Card *> &Board::BattleZone(TurnOwner side) const {
    return const_cast<Board *>(this)->BattleZone(side);
}

const SDL_Rect &Board::PreparationRect(TurnOwner side) const {
    return side == TurnOwner::PLAYER ? playerPreparationRect : enemyPreparationRect;
}

const SDL_Rect &Board::BattleRect(TurnOwner side) const {
    return side == TurnOwner::PLAYER ? playerBattleRect : enemyBattleRect;
}

bool Board::MoveCard(Card *card, std::vector<Card *> &from, const SDL_Rect &fromRect,
                     std::vector<Card *> &to, const SDL_Rect &toRect) {
    if (!card) return false;

    auto it = std::find(from.begin(), from.end(), card);
    if (it == from.end()) return false;
    if (static_cast<int>(to.size()) >= kZoneLimit) return false;

    from.erase(it);
    to.push_back(card);
    OrganizeZone(from, fromRect);
    OrganizeZone(to, toRect);
    return true;
}

bool Board::RemoveFromZone(Card *card, std::vector<Card *> &zone) {
    auto it = std::find(zone.begin(), zone.end(), card);
    if (it == zone.end()) return false;
    zone.erase(it);
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Organização de zona
// ═══════════════════════════════════════════════════════════════════

void Board::OrganizeZone(std::vector<Card *> &zone, const SDL_Rect &rect) {
    int n = static_cast<int>(zone.size());
    if (n == 0) return;

    int totalW = n * kCardWidth + (n - 1) * kCardGap;
    int startX = rect.x + (rect.w - totalW) / 2;
    int y = rect.y + (rect.h - kCardHeight) / 2;

    for (int i = 0; i < n; ++i)
        zone[i]->SetPosition(startX + i * (kCardWidth + kCardGap), y);
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

bool Board::AddToPreparation(Card *card, TurnOwner side) {
    if (!card) return false;

    std::vector<Card *> &prep = PreparationZone(side);
    if (static_cast<int>(prep.size()) >= kZoneLimit) {
        std::cout << "[BOARD] Campo de preparacao cheio! (max " << kZoneLimit << ")" << std::endl;
        return false;
    }

    prep.push_back(card);
    OrganizeZone(prep, PreparationRect(side));
    GameManager::PlaySFX("card_place");
    std::cout << "[BOARD] " << card->GetName() << " entrou na preparacao." << std::endl;
    return true;
}

bool Board::AddToPlayerPreparation(Card *card, std::vector<Card *> &objectsPool) {
    (void)objectsPool;
    return AddToPreparation(card, TurnOwner::PLAYER);
}

bool Board::AddToEnemyPreparation(Card *card) { return AddToPreparation(card, TurnOwner::OPPONENT); }

bool Board::RemoveFromPreparation(Card *card, TurnOwner side) {
    std::vector<Card *> &prep = PreparationZone(side);
    if (!RemoveFromZone(card, prep)) return false;
    OrganizeZone(prep, PreparationRect(side));
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Declaração de atacantes
// ═══════════════════════════════════════════════════════════════════

bool Board::DeclareAttacker(Card *card, bool playSound) {
    if (!card || !dynamic_cast<CreatureCard *>(card)) return false;
    if (IsCardSelectedAsAttacker(card)) return false;
    if (!turnManager.CanAttackThisTurn()) return false;

    const TurnOwner side = GetAttackingSide();
    if (!MoveCard(card, PreparationZone(side), PreparationRect(side), BattleZone(side),
                  BattleRect(side))) {
        std::cout << "[BOARD] " << card->GetName() << " nao pode entrar no campo de batalha."
                  << std::endl;
        return false;
    }

    selectedAttackers.push_back(card);
    if (playSound) GameManager::PlaySFX("card_change");
    std::cout << "[BOARD] " << card->GetName() << " declarada como atacante." << std::endl;
    return true;
}

bool Board::UndeclareAttacker(Card *card, bool playSound) {
    auto it = std::find(selectedAttackers.begin(), selectedAttackers.end(), card);
    if (it == selectedAttackers.end()) return false;

    const TurnOwner side = GetAttackingSide();
    if (!MoveCard(card, BattleZone(side), BattleRect(side), PreparationZone(side),
                  PreparationRect(side))) {
        std::cout << "[BOARD] Preparacao cheia, " << card->GetName() << " nao pode voltar."
                  << std::endl;
        return false;
    }

    selectedAttackers.erase(it);
    if (playSound) GameManager::PlaySFX("card_change");
    std::cout << "[BOARD] " << card->GetName() << " removida dos atacantes." << std::endl;
    return true;
}

int Board::DeclareAllAttackers() {
    // Mesma regra do jogador: ninguem ataca no proprio primeiro turno.
    if (!turnManager.CanAttackThisTurn()) {
        std::cout << "[BOARD] Primeiro turno de " << turnManager.GetOwnerName()
                  << ": ataque nao permitido." << std::endl;
        return 0;
    }

    const std::vector<Card *> candidates = PreparationZone(GetAttackingSide());

    int declared = 0;
    for (Card *card : candidates)
        if (DeclareAttacker(card, false)) ++declared;

    if (declared > 0) GameManager::PlaySFX("card_change");
    return declared;
}

void Board::ToggleAttackerSelection(Card *card, std::vector<Card *> &objectsPool) {
    (void)objectsPool;
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return;

    if (IsCardSelectedAsAttacker(card))
        UndeclareAttacker(card, true);
    else
        DeclareAttacker(card, true);
}

void Board::ReturnAllAttackersToPreparation(std::vector<Card *> &objectsPool) {
    (void)objectsPool;

    const std::vector<Card *> copy = selectedAttackers;
    int returned = 0;
    for (Card *card : copy)
        if (UndeclareAttacker(card, false)) ++returned;

    if (returned > 0) GameManager::PlaySFX("card_change");
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

// ═══════════════════════════════════════════════════════════════════
//  Declaração de defensores
// ═══════════════════════════════════════════════════════════════════

Card *Board::GetDefenderOf(const Card *attacker) const {
    for (const DefenderAssignment &assignment : defenderAssignments)
        if (assignment.attacker == attacker) return assignment.defender;
    return nullptr;
}

Card *Board::GetAttackerBlockedBy(const Card *defender) const {
    for (const DefenderAssignment &assignment : defenderAssignments)
        if (assignment.defender == defender) return assignment.attacker;
    return nullptr;
}

bool Board::IsDefending(const Card *card) const { return GetAttackerBlockedBy(card) != nullptr; }

bool Board::IsAttacking(const Card *card) const {
    const std::vector<Card *> &attackers = GetAttackers();
    return std::find(attackers.begin(), attackers.end(), card) != attackers.end();
}

std::vector<Card *> Board::GetAvailableDefenders() const {
    std::vector<Card *> available;
    for (Card *card : PreparationZone(GetDefendingSide())) {
        if (!dynamic_cast<CreatureCard *>(card)) continue;
        if (IsDefending(card)) continue;
        available.push_back(card);
    }
    return available;
}

bool Board::AssignDefender(Card *defender, Card *attacker) {
    if (turnManager.GetCombatStep() != CombatStep::DECLARE_DEFENDERS) return false;
    if (!defender || !attacker) return false;
    if (!dynamic_cast<CreatureCard *>(defender) || !dynamic_cast<CreatureCard *>(attacker))
        return false;

    if (!IsAttacking(attacker)) {
        std::cout << "[DEFESA] " << attacker->GetName() << " nao esta atacando." << std::endl;
        return false;
    }
    if (GetDefenderOf(attacker)) {
        std::cout << "[DEFESA] " << attacker->GetName() << " ja esta sendo defendida." << std::endl;
        return false;
    }
    if (IsDefending(defender)) return false;

    const TurnOwner side = GetDefendingSide();
    if (!MoveCard(defender, PreparationZone(side), PreparationRect(side), BattleZone(side),
                  BattleRect(side)))
        return false;

    defenderAssignments.push_back({defender, attacker});
    GameManager::PlaySFX("card_change");
    std::cout << "[DEFESA] " << defender->GetName() << " vai defender contra "
              << attacker->GetName() << "." << std::endl;
    return true;
}

bool Board::UnassignDefender(Card *defender) {
    auto it = std::find_if(defenderAssignments.begin(), defenderAssignments.end(),
                           [defender](const DefenderAssignment &a) { return a.defender == defender; });
    if (it == defenderAssignments.end()) return false;

    const TurnOwner side = GetDefendingSide();
    MoveCard(defender, BattleZone(side), BattleRect(side), PreparationZone(side),
             PreparationRect(side));
    defenderAssignments.erase(it);
    GameManager::PlaySFX("card_change");
    std::cout << "[DEFESA] " << defender->GetName() << " nao vai mais defender." << std::endl;
    return true;
}

void Board::ClearDefenders() {
    std::vector<DefenderAssignment> copy = defenderAssignments;
    for (const DefenderAssignment &assignment : copy)
        UnassignDefender(assignment.defender);
    defenderAssignments.clear();
}

// ═══════════════════════════════════════════════════════════════════
//  Resolução do combate
// ═══════════════════════════════════════════════════════════════════

CombatResult Board::ResolveCombat(int &defendingEntityHP, std::vector<Card *> &objectsPool) {
    (void)objectsPool;

    CombatResult result;

    const TurnOwner attackingSide = GetAttackingSide();
    const TurnOwner defendingSide = GetDefendingSide();

    // Copia: as zonas mudam durante a resolucao (mortes e retorno a preparacao)
    const std::vector<Card *> attackers = GetAttackers();
    const std::vector<DefenderAssignment> assignments = defenderAssignments;

    // ── Troca de dano ─────────────────────────────────────────────
    for (Card *attackerCard : attackers) {
        CreatureCard *attacker = dynamic_cast<CreatureCard *>(attackerCard);
        if (!attacker) continue;

        CreatureCard *blocker = dynamic_cast<CreatureCard *>(GetDefenderOf(attackerCard));

        if (!blocker) {
            result.damageDealt += attacker->GetAttack();
            std::cout << "[COMBATE] " << attacker->GetName() << " passou sem bloqueio e causou "
                      << attacker->GetAttack() << " de dano direto." << std::endl;
            continue;
        }

        const int attackerDamage = attacker->GetAttack();
        const int blockerDamage = blocker->GetAttack();

        blocker->AddHealth(-attackerDamage);
        attacker->AddHealth(-blockerDamage);

        std::cout << "[COMBATE] " << attacker->GetName() << " (" << attackerDamage << " dmg, vida "
                  << attacker->GetHealth() << ") x " << blocker->GetName() << " (" << blockerDamage
                  << " dmg, vida " << blocker->GetHealth() << ")" << std::endl;
    }

    if (!attackers.empty()) GameManager::PlaySFX("card_combat");

    // ── Mortes ────────────────────────────────────────────────────
    auto buryIfDead = [&](Card *card, TurnOwner side) {
        const CreatureCard *creature = dynamic_cast<const CreatureCard *>(card);
        if (!creature || creature->GetHealth() > 0) return;

        RemoveFromZone(card, BattleZone(side));
        if (side == TurnOwner::PLAYER)
            result.deadPlayerCards.push_back(card);
        else
            result.deadEnemyCards.push_back(card);
    };

    for (Card *attacker : attackers)
        buryIfDead(attacker, attackingSide);
    for (const DefenderAssignment &assignment : assignments)
        buryIfDead(assignment.defender, defendingSide);

    // ── Dano na entidade defensora ────────────────────────────────
    defendingEntityHP -= result.damageDealt;
    if (defendingEntityHP < 0) defendingEntityHP = 0;
    result.opponentDied = (defendingEntityHP <= 0);

    std::cout << "[COMBATE] Dano direto total: " << result.damageDealt
              << " | HP restante do defensor: " << defendingEntityHP << std::endl;

    // ── Sobreviventes voltam para a preparação ────────────────────
    int survivorsReturned = 0;
    for (TurnOwner side : {attackingSide, defendingSide}) {
        const std::vector<Card *> survivors = BattleZone(side);
        for (Card *card : survivors)
            if (MoveCard(card, BattleZone(side), BattleRect(side), PreparationZone(side),
                         PreparationRect(side)))
                ++survivorsReturned;
    }
    if (survivorsReturned > 0) GameManager::PlaySFX("card_change");

    defenderAssignments.clear();
    selectedAttackers.clear();
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

// As zonas nao sao desenhadas: o proprio fundo (arena.png) marca o campo.
// Os rects continuam valendo para posicionar as cartas e testar cliques.
void Board::Render(SDL_Renderer *renderer) const {
    for (auto *c : enemyPreparationCards)
        c->Render(renderer);
    for (auto *c : enemyBattleCards)
        c->Render(renderer);
    for (auto *c : playerBattleCards)
        c->Render(renderer);
    for (auto *c : playerPreparationCards)
        c->Render(renderer);
}