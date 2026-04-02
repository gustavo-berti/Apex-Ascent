#pragma once
#include "../core/GameWorld.hpp"
#include "../logic/CardDatabase.hpp"
#include "../objects/cards/Card.hpp"
#include "../logic/Player.hpp"
#include <string>
#include <vector>

class CreatureCard;

class SceneBattle : public GameWorld {
private:
    SDL_Rect enemyPreparationZone;
    SDL_Rect enemyBattleZone;
    SDL_Rect playerBattleZone;
    SDL_Rect playerPreparationZone;
    SDL_Rect playerHandZone;
    SDL_Rect btnBuyCard;
    Player* currentState = nullptr;

    std::vector<Card*> playerPreparationCards;
    std::vector<Card*> playerBattleCards;
    std::vector<Card*> drawPile;
    std::vector<Card*> hand;
    std::vector<Card*> discardPile;
    CardDatabase cardDatabase;
    Card* draggedCard = nullptr;

    bool IsBuyCardButtonClick(const SDL_Event& event) const;
    void HandleBuyCardAction();
    bool IsHandCardClick(const SDL_Event& event, const Card* card) const;
    bool HandleHandCardAction(const SDL_Event& event);
    bool IsPreparationCardClick(const SDL_Event& event, const Card* card) const;
    bool HandlePreparationCardAction(const SDL_Event& event);
    bool AddCardToPlayerBattle(Card* card);
    void OrganizeZone(std::vector<Card*>& zoneCards, SDL_Rect zoneRect);
    bool SetCurrentPlayerState(Player* playerState);
    void ResetBattleDeckState();
    void AddDeckCardToDrawPile(const std::string& cardId);
    void BuildDrawPileFromMasterDeck();
    void ShuffleDrawPile();

public:
    SceneBattle();
    ~SceneBattle() override;

    void Initialize() override;
    void HandleInput(SDL_Event& event) override;
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
    void StartBattle(Player* state);
    void DrawCards(int amount);
    bool AddCardToPlayerPreparation(Card* card);
};