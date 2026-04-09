#pragma once
#include "../core/GameWorld.hpp"
#include "../logic/CardDatabase.hpp"
#include "../logic/TurnManager.hpp"
#include "../objects/cards/Card.hpp"
#include "../logic/Player.hpp"
#include <string>
#include <vector>

class CreatureCard;

class SceneBattle : public GameWorld {
private:
    // ── Zonas do tabuleiro ─────────────────────────────────────────
    SDL_Rect enemyPreparationZone;
    SDL_Rect enemyBattleZone;
    SDL_Rect playerBattleZone;
    SDL_Rect playerPreparationZone;
    SDL_Rect playerHandZone;

    // ── Botões ─────────────────────────────────────────────────────
    SDL_Rect btnBuyCard;
    SDL_Rect btnNextPhase;   // <<< NOVO: botão "Passar Fase"

    // ── Estado ────────────────────────────────────────────────────
    Player*      currentState = nullptr;
    TurnManager  turnManager;             // <<< NOVO

    // ── Pilhas de cartas ──────────────────────────────────────────
    std::vector<Card*> playerPreparationCards;
    std::vector<Card*> playerBattleCards;
    std::vector<Card*> drawPile;
    std::vector<Card*> hand;
    std::vector<Card*> discardPile;
    CardDatabase cardDatabase;
    Card* draggedCard = nullptr;

    // ── Helpers de input ──────────────────────────────────────────
    bool IsBuyCardButtonClick       (const SDL_Event& event) const;
    bool IsNextPhaseButtonClick     (const SDL_Event& event) const;   // <<< NOVO
    bool IsHandCardClick            (const SDL_Event& event, const Card* card) const;
    bool IsPreparationCardClick     (const SDL_Event& event, const Card* card) const;

    // ── Handlers de ação ─────────────────────────────────────────
    void HandleBuyCardAction();
    void HandleNextPhaseAction();                                      // <<< NOVO
    bool HandleHandCardAction       (const SDL_Event& event);
    bool HandlePreparationCardAction(const SDL_Event& event);

    // ── Lógica de campo ──────────────────────────────────────────
    bool AddCardToPlayerPreparation (Card* card);
    bool AddCardToPlayerBattle      (Card* card);
    void OrganizeZone               (std::vector<Card*>& zoneCards, SDL_Rect zoneRect);

    // ── Lógica de turno ──────────────────────────────────────────
    void OnPhaseChanged(TurnOwner owner, BattlePhase phase);          // <<< NOVO
    void OnTurnChanged (TurnOwner owner);                             // <<< NOVO
    void RunOpponentTurn();                                           // <<< NOVO

    // ── Construção do deck ───────────────────────────────────────
    bool SetCurrentPlayerState  (Player* playerState);
    void ResetBattleDeckState   ();
    void AddDeckCardToDrawPile  (const std::string& cardId);
    void BuildDrawPileFromMasterDeck();
    void ShuffleDrawPile        ();

    // ── Render de UI ─────────────────────────────────────────────
    void RenderTurnInfo (SDL_Renderer* renderer) const;               // <<< NOVO
    void RenderButton   (SDL_Renderer* renderer, SDL_Rect rect,
                         Uint8 r, Uint8 g, Uint8 b) const;

public:
    SceneBattle();
    ~SceneBattle() override;

    void Initialize ()                    override;
    void HandleInput(SDL_Event& event)    override;
    void Update     (float dt)            override;
    void Render     (SDL_Renderer* renderer) override;

    void StartBattle(Player* state);
    void DrawCards  (int amount);
};