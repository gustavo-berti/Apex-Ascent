#pragma once
#include "../logic/TurnManager.hpp"
#include "../objects/cards/Card.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include <SDL2/SDL.h>
#include <vector>

struct CombatResult {
    int damageDealt = 0;
    bool opponentDied = false;
};

class Board {
  public:
    std::vector<Card *> playerPreparationCards;
    std::vector<Card *> playerBattleCards;
    std::vector<Card *> enemyPreparationCards;
    std::vector<Card *> enemyBattleCards;

  private:
    TurnManager &turnManager;

    SDL_Rect playerPreparationRect{};
    SDL_Rect playerBattleRect{};
    SDL_Rect enemyPreparationRect{};
    SDL_Rect enemyBattleRect{};

    std::vector<Card *> selectedAttackers;
    bool attackDeclared = false;

    void OrganizeZone(std::vector<Card *> &zone, const SDL_Rect &rect);
    bool RemoveRightmostCard(std::vector<Card *> &zone, const SDL_Rect &rect,
                             std::vector<Card *> &objectsPool);

  public:
    explicit Board(TurnManager &tm);

    // ── Configuração ──────────────────────────────────────────────
    void SetZoneRects(SDL_Rect playerPrep, SDL_Rect playerBattle, SDL_Rect enemyPrep,
                      SDL_Rect enemyBattle);

    // ── Reset ─────────────────────────────────────────────────────
    void Reset();

    // ── Adição de cartas ──────────────────────────────────────────
    bool AddToPlayerPreparation(Card *card, std::vector<Card *> &objectsPool);
    bool AddToPlayerBattle(Card *card);
    bool AddToEnemyPreparation(Card *card);
    bool AddToEnemyBattle(Card *card);

    // ── Movimentação ──────────────────────────────────────────────
    bool MoveFromPreparationToBattle(Card *card);
    bool MoveFromBattleToPreparation(Card *card, std::vector<Card *> &objectsPool);

    // ── Seleção de atacantes ──────────────────────────────────────
    void ToggleAttackerSelection(Card *card, std::vector<Card *> &objectsPool);
    void ReturnAllAttackersToPreparation(std::vector<Card *> &objectsPool);
    void ClearAttackers() { selectedAttackers.clear(); }

    // ── Combate ───────────────────────────────────────────────────
    // Retorna false se não houver atacantes (cena passa o combate sem atacar)
    bool ConfirmAttack();

    // Oponente sem IA: passa automaticamente
    void ResolveDefenders();

    // Calcula dano direto ao HP do oponente (sem defensores por enquanto).
    // opponentCurrentHP é passado por referência — Board aplica o dano e
    // preenche CombatResult. Atacantes voltam para a preparação.
    CombatResult ResolveCombat(int &opponentCurrentHP, std::vector<Card *> &objectsPool);

    // ── Consultas ─────────────────────────────────────────────────
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
};