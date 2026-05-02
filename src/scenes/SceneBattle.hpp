#pragma once
#include "../core/GameWorld.hpp"
#include "../core/data/CardDatabase.hpp"
#include "../logic/Board.hpp"
#include "../logic/Player.hpp"
#include "../logic/TurnManager.hpp"
#include "../objects/cards/Card.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include <string>
#include <vector>

class SceneBattle : public GameWorld {
  private:
    // ── Tabuleiro ─────────────────────────────────────────────────
    TurnManager turnManager;
    Board board;

    // ── Zonas SDL (apenas para layout — passadas ao Board) ────────
    SDL_Rect enemyPreparationZone;
    SDL_Rect enemyBattleZone;
    SDL_Rect playerBattleZone;
    SDL_Rect playerPreparationZone;
    SDL_Rect playerHandZone;

    // ── Botões ────────────────────────────────────────────────────
    SDL_Rect btnNextPhase;
    SDL_Rect btnAttack;
    SDL_Rect btnCancel;

    // ── Estado da cena ────────────────────────────────────────────
    Player *currentState = nullptr;
    SDL_Renderer *renderer = nullptr;
    CardDatabase cardDatabase;
    Card *draggedCard = nullptr;

    // ── Mana do oponente (simulado) ───────────────────────────────
    ManaState opponentMana;

    // ── Pilhas de cartas ──────────────────────────────────────────
    std::vector<Card *> drawPile;
    std::vector<Card *> hand;
    std::vector<Card *> discardPile;

    // ── Callbacks do TurnManager ──────────────────────────────────
    void OnPhaseChanged(TurnOwner owner, BattlePhase phase);
    void OnTurnChanged(TurnOwner owner);
    void OnCombatStepChanged(CombatStep step);

    // ── Lógica de turno ───────────────────────────────────────────
    void HandleTurnStart();
    void RunOpponentTurn();

    // ── Mana ──────────────────────────────────────────────────────
    bool SpendPlayerMana(int cost, const std::string &cardName);

    // ── Lógica de combate (orquestra Board + TurnManager) ─────────
    void HandleAttackButton();
    void HandleCancelAttack();
    void HandleConfirmAttack();

    // ── Helpers de estado ─────────────────────────────────────────
    bool CanPlayCreature() const;
    bool CanPlaySpell() const;

    // ── Input ─────────────────────────────────────────────────────
    bool HandleNextPhaseClick(const SDL_Event &e);
    bool HandleAttackClick(const SDL_Event &e);
    bool HandleCancelClick(const SDL_Event &e);
    bool HandleHandCardClick(const SDL_Event &e);
    bool HandleBattleCardClick(const SDL_Event &e);

    // ── Deck ──────────────────────────────────────────────────────
    bool SetCurrentPlayerState(Player *p);
    void ResetBattleState();
    void AddDeckCardToDrawPile(const std::string &cardId);
    void BuildDrawPileFromPlayerDeck();
    void ShuffleDrawPile();

    // ── Render ────────────────────────────────────────────────────
    void RenderButtons(SDL_Renderer *renderer) const;
    void RenderHand(SDL_Renderer *renderer) const;
    void RenderHUD(SDL_Renderer *renderer) const;
    void RenderMana(SDL_Renderer *renderer) const;
    void RenderButton(SDL_Renderer *renderer, SDL_Rect r, Uint8 red, Uint8 grn, Uint8 blu,
                      bool enabled = true) const;

  public:
    SceneBattle();
    ~SceneBattle() override;

    void Initialize() override;
    void HandleInput(SDL_Event &e) override;
    void Update(float dt) override;
    void Render(SDL_Renderer *renderer) override;

    void StartBattle(Player *state, SDL_Renderer *renderer);
    void DrawCards(int amount);
};