#pragma once
#include "../core/GameObject.hpp"
#include "../logic/TurnManager.hpp"
#include "../objects/cards/Card.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include <SDL2/SDL.h>
#include <vector>

// Board gerencia as quatro zonas do campo de batalha e toda a lógica
// de movimentação e combate entre elas.
// SceneBattle possui um Board e delega a ele todas as operações de campo.
//
// Zonas:
//   playerPreparation  — criaturas baixadas pelo jogador, prontas para atacar
//   playerBattle       — criaturas atacando neste turno
//   enemyPreparation   — criaturas do oponente prontas para defender
//   enemyBattle        — criaturas do oponente atacando
//
class Board {
  public:
    // ── Zonas de campo (posições geridas pela SceneBattle via SetZoneRects) ──
    std::vector<Card *> playerPreparationCards;
    std::vector<Card *> playerBattleCards;
    std::vector<Card *> enemyPreparationCards;
    std::vector<Card *> enemyBattleCards;

  private:
    // Referência ao TurnManager — Board consulta fase e passo diretamente
    TurnManager &turnManager;

    // Retângulos das zonas (recebidos da SceneBattle)
    SDL_Rect playerPreparationRect{};
    SDL_Rect playerBattleRect{};
    SDL_Rect enemyPreparationRect{};
    SDL_Rect enemyBattleRect{};

    // Estado do combate
    std::vector<Card *> selectedAttackers;
    bool attackDeclared = false;

    // ── Internos de campo ─────────────────────────────────────────
    void OrganizeZone(std::vector<Card *> &zone, const SDL_Rect &rect);
    bool RemoveRightmostCard(std::vector<Card *> &zone, const SDL_Rect &rect,
                 std::vector<GameObject *> &objectsPool);

  public:
    explicit Board(TurnManager &tm);

    // ── Configuração ──────────────────────────────────────────────
    void SetZoneRects(SDL_Rect playerPrep, SDL_Rect playerBattle, SDL_Rect enemyPrep,
                      SDL_Rect enemyBattle);

    // ── Reset entre combates ──────────────────────────────────────
    void Reset();

    // ── Adição de cartas nas zonas ────────────────────────────────
    // Retorna false se a zona estiver cheia e não houver como liberar espaço.
    bool AddToPlayerPreparation(Card *card, std::vector<GameObject *> &objectsPool);
    bool AddToPlayerBattle(Card *card);
    bool AddToEnemyPreparation(Card *card);
    bool AddToEnemyBattle(Card *card);

    // ── Movimentação interna (preparação ↔ batalha) ───────────────
    bool MoveFromPreparationToBattle(Card *card);
    bool MoveFromBattleToPreparation(Card *card, std::vector<GameObject *> &objectsPool);

    // ── Seleção de atacantes ──────────────────────────────────────
    // Chamado quando o jogador clica numa carta em ATTACK_MAGIC.
    // Selecionar move prep→battle; desselecionar move battle→prep.
    void ToggleAttackerSelection(Card *card, std::vector<GameObject *> &objectsPool);
    void ReturnAllAttackersToPreparation(std::vector<GameObject *> &objectsPool);
    void ClearAttackers() { selectedAttackers.clear(); }

    // ── Lógica de combate ─────────────────────────────────────────
    // Retorna false se não houver atacantes selecionados
    // (SceneBattle deve então pular direto para SECOND_MAIN).
    bool ConfirmAttack();
    void ResolveDefenders();                              // oponente sem IA: passa automaticamente
    void ResolveCombat(std::vector<GameObject *> &objectsPool); // placeholder; libera atacantes

    // ── Consultas de estado ───────────────────────────────────────
    bool HasPlayerPreparationCards() const { return !playerPreparationCards.empty(); }
    bool HasSelectedAttackers() const { return !selectedAttackers.empty(); }
    bool IsAttackDeclared() const { return attackDeclared; }
    bool IsCardSelectedAsAttacker(const Card *card) const;

    // Botão de ataque aparece quando há criaturas na preparação do jogador
    // e o turno/fase/passo permitem ataque.
    bool ShouldShowAttackButton() const;
    bool CanDeclareAttack() const;

    // ── Render ────────────────────────────────────────────────────
    void Render(SDL_Renderer *renderer) const;
    void RenderZoneBorders(SDL_Renderer *renderer) const;

    // ── Acesso às zonas para iteração externa (ex: input) ─────────
    const std::vector<Card *> &GetPlayerBattleCards() const { return playerBattleCards; }
    const std::vector<Card *> &GetPlayerPreparationCards() const { return playerPreparationCards; }
    const std::vector<Card *> &GetEnemyPreparationCards() const { return enemyPreparationCards; }
    const std::vector<Card *> &GetEnemyBattleCards() const { return enemyBattleCards; }
    const std::vector<Card *> &GetSelectedAttackers() const { return selectedAttackers; }
};