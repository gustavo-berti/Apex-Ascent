#pragma once
#include "../core/GameWorld.hpp"
#include "../core/data/CardDatabase.hpp"
#include "../logic/Player.hpp"
#include "../logic/TurnManager.hpp"
#include "../objects/cards/Card.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include <string>
#include <vector>

class SceneBattle : public GameWorld {
  private:
    // ── Zonas do tabuleiro ────────────────────────────────────────
    SDL_Rect enemyPreparationZone;
    SDL_Rect enemyBattleZone;
    SDL_Rect playerBattleZone;
    SDL_Rect playerPreparationZone;
    SDL_Rect playerHandZone;

    // ── Botões ────────────────────────────────────────────────────
    SDL_Rect btnNextPhase;
    SDL_Rect btnAttack;
    SDL_Rect btnCancel;

    // ── Estado do jogo ────────────────────────────────────────────
    Player *currentState = nullptr;
    TurnManager turnManager;
    CardDatabase cardDatabase;
    Card *draggedCard = nullptr;

    // ── Mana do oponente (simulado) ───────────────────────────────
    ManaState opponentMana;

    // ── Pilhas de cartas ──────────────────────────────────────────
    std::vector<Card *> drawPile;
    std::vector<Card *> hand;
    std::vector<Card *> discardPile;

    // ── Zonas de campo ────────────────────────────────────────────
    std::vector<Card *> playerPreparationCards;
    std::vector<Card *> playerBattleCards;
    std::vector<Card *> enemyPreparationCards;
    std::vector<Card *> enemyBattleCards;

    // ── Estado do combate ─────────────────────────────────────────
    std::vector<Card *> selectedAttackers;
    bool attackDeclared = false;

    // ── Callbacks do TurnManager ──────────────────────────────────
    void OnPhaseChanged(TurnOwner owner, BattlePhase phase);
    void OnTurnChanged(TurnOwner owner);
    void OnCombatStepChanged(CombatStep step);

    // ── Lógica de início de turno ─────────────────────────────────
    void HandleTurnStart();
    void RunOpponentTurn();

    // ── Mana ──────────────────────────────────────────────────────
    ManaState &CurrentMana();
    bool SpendPlayerMana(int cost, const std::string &cardName);

    // ── Lógica da fase de combate ─────────────────────────────────
    void HandleAttackButton();
    void HandleCancelAttack();
    void ConfirmAttack();
    void ResolveDefenders();
    void ResolveCombat();
    void ToggleAttackerSelection(Card *card);
    bool MoveCardFromPreparationToBattle(Card *card);
    bool MoveCardFromBattleToPreparation(Card *card);
    void ReturnSelectedAttackersToPreparation();
    bool RemoveRightmostPreparationCard();

    // ── Helpers de estado ─────────────────────────────────────────
    bool CanPlayCreature() const;
    bool CanPlaySpell() const;
    bool CanDeclareAttack() const;
    bool ShowAttackButton() const;

    // ── Input ─────────────────────────────────────────────────────
    bool HandleNextPhaseClick(const SDL_Event &e);
    bool HandleAttackClick(const SDL_Event &e);
    bool HandleCancelClick(const SDL_Event &e);
    bool HandleHandCardClick(const SDL_Event &e);
    bool HandleBattleCardClick(const SDL_Event &e);

    // ── Campo ─────────────────────────────────────────────────────
    bool AddCardToPlayerPreparation(Card *card);
    bool AddCardToPlayerBattle(Card *card);
    void OrganizeZone(std::vector<Card *> &zone, SDL_Rect rect);

    // ── Deck ──────────────────────────────────────────────────────
    bool SetCurrentPlayerState(Player *p);
    void ResetBattleDeckState();
    void AddDeckCardToDrawPile(const std::string &cardId);
    void BuildDrawPileFromPlayerDeck();
    void ShuffleDrawPile();

    // ── Render ────────────────────────────────────────────────────
    void RenderZones(SDL_Renderer *renderer) const;
    void RenderButtons(SDL_Renderer *renderer) const;
    void RenderCards(SDL_Renderer *renderer) const;
    void RenderHUD(SDL_Renderer *renderer) const;
    void RenderMana(SDL_Renderer *renderer) const; // <<< NOVO
    void RenderButton(SDL_Renderer *renderer, SDL_Rect r, Uint8 red, Uint8 grn, Uint8 blu,
                      bool enabled = true) const;

  public:
    SDL_Renderer *renderer = nullptr;

    SceneBattle();
    ~SceneBattle() override;

    void Initialize() override;
    void HandleInput(SDL_Event &e) override;
    void Update(float dt) override;
    void Render(SDL_Renderer *renderer) override;
    void StartBattle(Player *state, SDL_Renderer *renderer);
    void DrawCards(int amount);
};