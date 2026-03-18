#pragma once
#include "../core/GameWorld.hpp"
#include "../logic/CardDatabase.hpp"
#include "../objects/cards/Card.hpp"

class CreatureCard;

class SceneBattle : public GameWorld {
private:
    SDL_Rect enemyPreparationZone;
    SDL_Rect enemyBattleZone;
    SDL_Rect playerBattleZone;
    SDL_Rect playerPreparationZone;
    SDL_Rect btnBuyCard;

    std::vector<Card*> playerPreparationCards;
    std::vector<Card*> playerBattleCards;
    CardDatabase cardDatabase;
    Card* draggedCard = nullptr;

    bool IsBuyCardButtonClick(const SDL_Event& event) const;
    void HandleBuyCardAction();
    void OrganizeZone(std::vector<Card*>& zoneCards, SDL_Rect zoneRect);

public:
    SceneBattle();
    ~SceneBattle() override;

    void Initialize() override;
    void HandleInput(SDL_Event& event) override;
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;

    void AddCardToPlayerPreparation(Card* card);
};