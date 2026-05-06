#pragma once
#include "../core/GameWorld.hpp"
#include "../core/data/CardDatabase.hpp"
#include "../logic/Board.hpp"
#include "../logic/Opponent.hpp"
#include "../logic/Player.hpp"
#include "../logic/TurnManager.hpp"
#include "../objects/cards/Card.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

enum class BattleOutcome { ONGOING, PLAYER_WIN, PLAYER_LOSE };

class SceneBattle : public GameWorld {
  private:
    // ── Núcleo do jogo ────────────────────────────────────────────
    TurnManager turnManager;
    Board board; // lógica de campo — referencia turnManager

    // ── Layout SDL ────────────────────────────────────────────────
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
    Opponent *opponent = nullptr;
    SDL_Renderer *renderer = nullptr;
    CardDatabase cardDatabase;
    Card *draggedCard = nullptr;
    BattleOutcome outcome = BattleOutcome::ONGOING;
    TTF_Font *font = nullptr;       // Fonte grande para tela de vitória/derrota
    TTF_Font *fontSmall = nullptr;  // Fonte pequena para HP display

    // ── Pilhas de cartas ──────────────────────────────────────────
    std::vector<Card *> drawPile;
    std::vector<Card *> hand;
    std::vector<Card *> discardPile;
    std::vector<Card *> cardObjects;

    // ── Callbacks do TurnManager ──────────────────────────────────
    void OnPhaseChanged(TurnOwner owner, BattlePhase phase);
    void OnTurnChanged(TurnOwner owner);
    void OnCombatStepChanged(CombatStep step);

    // ── Lógica de turno ───────────────────────────────────────────
    void HandleTurnStart();
    void RunOpponentTurn();

    // ── Mana ──────────────────────────────────────────────────────
    bool SpendPlayerMana(int cost, const std::string &cardName);

    // ── Combate ───────────────────────────────────────────────────
    void HandleAttackButton();
    void HandleCancelAttack();
    void HandleConfirmAttack();
    void CheckBattleOutcome(const CombatResult &result);

    // ── Helpers de estado ─────────────────────────────────────────
    bool CanPlayCreature() const;
    bool CanPlaySpell() const;
    bool IsBattleOver() const { return outcome != BattleOutcome::ONGOING; }

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
    void RenderHealthBars(SDL_Renderer *renderer) const;
    void RenderOutcome(SDL_Renderer *renderer) const;
    void RenderButton(SDL_Renderer *renderer, SDL_Rect r, Uint8 red, Uint8 grn, Uint8 blu,
                      bool enabled = true) const;

  public:
    SceneBattle();
    ~SceneBattle() override;

    void Initialize(SDL_Renderer *renderer) override;
    void HandleInput(SDL_Event &e) override;
    void Update(float dt) override;
    void Render(SDL_Renderer *renderer) override;

    // opponent deve ter SetGuardian() chamado antes se aplicável
    void StartBattle(Player *state, Opponent *opp, SDL_Renderer *renderer);
    void DrawCards(int amount);
};