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

struct SummonPendingState {
    Card *cardToSummon = nullptr;
    Card *cardToSacrifice = nullptr;
    bool active = false;

    void Begin(Card *toSummon) {
        cardToSummon = toSummon;
        cardToSacrifice = nullptr;
        active = true;
    }

    void Clear() {
        cardToSummon = nullptr;
        cardToSacrifice = nullptr;
        active = false;
    }
};

struct BattlePiles {
    std::vector<Card *> drawPile;
    std::vector<Card *> hand;
    std::vector<Card *> discardPile;
};

class SceneBattle : public GameWorld {
  private:
    // ── Núcleo do jogo ────────────────────────────────────────────
    TurnManager turnManager;
    Board board;

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
    bool hasRendered = false;
    bool matchStartPending = false;

    // ── Fontes ────────────────────────────────────────────────────
    TTF_Font *font = nullptr;      // grande — vitória/derrota
    TTF_Font *fontSmall = nullptr; // pequena — HP display
    TTF_Font *fontUI = nullptr;    // frozen — textos de UI (sacrifício, etc.)

    // ── Estado de sacrifício ──────────────────────────────────────
    SummonPendingState summonPending;

    // ── Pilhas de cartas ──────────────────────────────────────────
    BattlePiles playerPiles;
    BattlePiles opponentPiles;
    std::vector<Card *> cardObjects;

    // ── Callbacks do TurnManager ──────────────────────────────────
    void OnPhaseChanged(TurnOwner owner, BattlePhase phase);
    void OnTurnChanged(TurnOwner owner);
    void OnCombatStepChanged(CombatStep step);

    // ── Lógica de turno ───────────────────────────────────────────
    void HandleTurnStart();
    void RunOpponentTurn();
    void StartMatchFlow();

    // ── Mana ──────────────────────────────────────────────────────
    bool SpendMana(Entity *entity, int cost, const std::string &cardName);

    // ── Invocação e sacrifício ─────────────────────────────────────
    void TrySummonCard(Card *card, std::vector<Card *>::reverse_iterator handIt);
    void ConfirmSummon();
    void CancelSummon();

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
    bool HandleSummonPendingInput(const SDL_Event &e);

    // ── Deck ──────────────────────────────────────────────────────
    bool SetCurrentPlayerState(Player *p);
    void ResetBattleState();
    void AddDeckCardToDrawPile(Entity* owner, const std::string &cardId);
    void BuildDrawPile(Entity *entity);
    void ShuffleDrawPile(Entity* entity);
    void RearrangeHand();
        BattlePiles& GetPilesFor(Entity* entity) {
        if (entity == currentState) return playerPiles;
        return opponentPiles;
    }

    // ── Render ────────────────────────────────────────────────────
    void RenderButtons(SDL_Renderer *renderer) const;
    void RenderHand(SDL_Renderer *renderer) const;
    void RenderHUD(SDL_Renderer *renderer) const;
    void RenderMana(SDL_Renderer *renderer) const;
    void RenderHealthBars(SDL_Renderer *renderer) const;
    void RenderOutcome(SDL_Renderer *renderer) const;
    void RenderSummonPending(SDL_Renderer *renderer) const;
    void RenderButton(SDL_Renderer *renderer, SDL_Rect r, Uint8 red, Uint8 grn, Uint8 blu,
                      bool enabled = true) const;
    void RenderText(SDL_Renderer *renderer, TTF_Font *f, const char *text, SDL_Color color, int x,
                    int y) const;

  public:
    SceneBattle();
    ~SceneBattle() override;

    void Initialize(SDL_Renderer *renderer) override;
    void HandleInput(SDL_Event &e) override;
    void Update(float dt) override;
    void Render(SDL_Renderer *renderer) override;
    void StartBattle(Player *state, Opponent *opp, SDL_Renderer *renderer);
    void DrawCards(Entity *entity, int amount);
};