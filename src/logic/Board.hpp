#pragma once
#include "../logic/TurnManager.hpp"
#include "../objects/cards/Card.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include "CombatTypes.hpp"
#include <SDL2/SDL.h>
#include <vector>

class Board {
  public:
    static constexpr int kZoneLimit = 6; // limite de cartas por campo

  private:
    // As zonas sao privadas de proposito: toda entrada/saida de carta passa
    // pelos metodos do Board, que aplicam os limites, reorganizam o layout e
    // tocam os efeitos sonoros. Jogador e IA usam exatamente o mesmo caminho.
    std::vector<Card *> playerPreparationCards;
    std::vector<Card *> playerBattleCards;
    std::vector<Card *> enemyPreparationCards;
    std::vector<Card *> enemyBattleCards;

    TurnManager &turnManager;

    SDL_Rect playerPreparationRect{};
    SDL_Rect playerBattleRect{};
    SDL_Rect enemyPreparationRect{};
    SDL_Rect enemyBattleRect{};

    std::vector<Card *> selectedAttackers;
    std::vector<DefenderAssignment> defenderAssignments;
    bool attackDeclared = false;

    void OrganizeZone(std::vector<Card *> &zone, const SDL_Rect &rect);
    bool RemoveRightmostCard(std::vector<Card *> &zone, const SDL_Rect &rect,
                             std::vector<Card *> &objectsPool);

    // ── Acesso generico as zonas por lado ─────────────────────────
    std::vector<Card *> &PreparationZone(TurnOwner side);
    std::vector<Card *> &BattleZone(TurnOwner side);
    const std::vector<Card *> &PreparationZone(TurnOwner side) const;
    const std::vector<Card *> &BattleZone(TurnOwner side) const;
    const SDL_Rect &PreparationRect(TurnOwner side) const;
    const SDL_Rect &BattleRect(TurnOwner side) const;

    bool MoveCard(Card *card, std::vector<Card *> &from, const SDL_Rect &fromRect,
                  std::vector<Card *> &to, const SDL_Rect &toRect);

    // Remove a carta da zona sem mexer em outras listas.
    bool RemoveFromZone(Card *card, std::vector<Card *> &zone);

    // playSound = false nas operacoes em lote, que tocam um som so no final.
    bool DeclareAttacker(Card *card, bool playSound);
    bool UndeclareAttacker(Card *card, bool playSound);

  public:
    explicit Board(TurnManager &tm);

    // ── Configuração ──────────────────────────────────────────────
    void SetZoneRects(SDL_Rect playerPrep, SDL_Rect playerBattle, SDL_Rect enemyPrep,
                      SDL_Rect enemyBattle);

    // ── Reset ─────────────────────────────────────────────────────
    void Reset();

    // ── Adição e remoção de cartas ────────────────────────────────
    bool AddToPreparation(Card *card, TurnOwner side);
    bool AddToPlayerPreparation(Card *card, std::vector<Card *> &objectsPool);
    bool AddToEnemyPreparation(Card *card);

    // Tira a carta da preparacao (sacrificio, efeito, etc.). Quem chama decide
    // o destino dela (cemiterio, mao...).
    bool RemoveFromPreparation(Card *card, TurnOwner side);

    // ── Declaração de atacantes ───────────────────────────────────
    // Sempre do lado que esta no turno: a criatura sai da preparacao e vai
    // para o campo de batalha.
    bool DeclareAttacker(Card *card) { return DeclareAttacker(card, true); }
    bool UndeclareAttacker(Card *card) { return UndeclareAttacker(card, true); }
    int DeclareAllAttackers(); // retorna quantas criaturas entraram no ataque
    void ToggleAttackerSelection(Card *card, std::vector<Card *> &objectsPool);
    void ReturnAllAttackersToPreparation(std::vector<Card *> &objectsPool);
    void ClearAttackers() { selectedAttackers.clear(); }

    // ── Declaração de defensores ──────────────────────────────────
    // O defensor sai da preparacao e vai para o campo de batalha do seu lado,
    // pareado com o atacante que ele vai bloquear.
    bool AssignDefender(Card *defender, Card *attacker);
    bool UnassignDefender(Card *defender);
    void ClearDefenders();

    const std::vector<DefenderAssignment> &GetDefenderAssignments() const {
        return defenderAssignments;
    }
    Card *GetDefenderOf(const Card *attacker) const;
    Card *GetAttackerBlockedBy(const Card *defender) const;
    bool IsDefending(const Card *card) const;
    bool IsAttacking(const Card *card) const;

    // Criaturas do lado defensor que ainda podem ser declaradas como defensoras.
    std::vector<Card *> GetAvailableDefenders() const;

    // ── Combate ───────────────────────────────────────────────────
    // Retorna false se não houver atacantes (cena passa o combate sem atacar)
    bool ConfirmAttack();

    // Resolve o combate do lado que esta atacando neste turno:
    // atacantes bloqueados trocam dano com o defensor (simultaneo — se o dano
    // for suficiente para os dois se matarem, os dois morrem) e atacantes sem
    // defensor batem direto na vida da entidade defensora.
    // defendingEntityHP e passado por referencia — Board aplica o dano e
    // preenche CombatResult. Sobreviventes voltam para a preparação.
    CombatResult ResolveCombat(int &defendingEntityHP, std::vector<Card *> &objectsPool);

    // ── Consultas ─────────────────────────────────────────────────
    TurnOwner GetAttackingSide() const { return turnManager.GetOwner(); }
    TurnOwner GetDefendingSide() const {
        return turnManager.GetOwner() == TurnOwner::PLAYER ? TurnOwner::OPPONENT
                                                           : TurnOwner::PLAYER;
    }
    int GetFieldCount(TurnOwner side) const {
        return static_cast<int>(PreparationZone(side).size() + BattleZone(side).size());
    }
    int GetFreePreparationSlots(TurnOwner side) const {
        return kZoneLimit - static_cast<int>(PreparationZone(side).size());
    }
    bool IsPreparationFull(TurnOwner side) const { return GetFreePreparationSlots(side) <= 0; }
    bool HasPlayerPreparationCards() const { return !playerPreparationCards.empty(); }
    bool HasSelectedAttackers() const { return !selectedAttackers.empty(); }
    bool IsAttackDeclared() const { return attackDeclared; }
    bool IsCardSelectedAsAttacker(const Card *card) const;
    bool ShouldShowAttackButton() const;
    bool CanDeclareAttack() const;

    // ── Render ────────────────────────────────────────────────────
    void Render(SDL_Renderer *renderer) const;
    void RenderZoneBorders(SDL_Renderer *renderer) const;

    // ── Acesso externo ────────────────────────────────────────────
    const std::vector<Card *> &GetPlayerBattleCards() const { return playerBattleCards; }
    const std::vector<Card *> &GetPlayerPreparationCards() const { return playerPreparationCards; }
    const std::vector<Card *> &GetEnemyPreparationCards() const { return enemyPreparationCards; }
    const std::vector<Card *> &GetEnemyBattleCards() const { return enemyBattleCards; }
    const std::vector<Card *> &GetSelectedAttackers() const { return selectedAttackers; }

    // Atacantes declarados no turno atual (mesma lista para jogador e IA).
    const std::vector<Card *> &GetAttackers() const { return selectedAttackers; }
};
