#include "SceneBattle.hpp"
#include "../objects/CreatureCard.hpp"
#include "../objects/SpellCard.hpp"
#include "../core/GameManager.hpp"
#include "../logic/CardFactory.hpp"
#include <iostream>

SceneBattle::SceneBattle() {
    draggedCard = nullptr;
}

SceneBattle::~SceneBattle() {}

void SceneBattle::Initialize() {
    std::cout << "A inicializar a Cena de Batalha..." << std::endl;

    if (!cardDatabase.LoadFromJson("assets/data/cards.json")) {
        std::cerr << "Falha ao carregar a base de dados de cartas." << std::endl;
    }

    enemyPreparationZone  = { 280, 20,  720, 150 };
    enemyBattleZone       = { 280, 180, 720, 150 };
    playerBattleZone      = { 280, 390, 720, 150 };
    playerPreparationZone = { 280, 550, 720, 150 };

    btnBuyCard = { 1050, 600, 150, 50 };
}

bool SceneBattle::IsBuyCardButtonClick(const SDL_Event& event) const {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) {
        return false;
    }

    return GameManager::IsPointInsideRect(event.button.x, event.button.y, btnBuyCard);
}

void SceneBattle::HandleBuyCardAction() {
    std::cout << "Botao clicado! Gerando carta..." << std::endl;

    CreatureCard* card = CardFactory::CreateCreatureCard(cardDatabase, 1);
    if (!card) {
        std::cout << "Falha ao criar carta: ID invalido ou sem estagios." << std::endl;
        return;
    }

    card->Initialize();
    AddCardToPlayerPreparation(card);
}

void SceneBattle::HandleInput(SDL_Event& event) {
    if (IsBuyCardButtonClick(event)) {
        HandleBuyCardAction();
    }
}
void SceneBattle::Update(float dt) {
    for (auto obj : objects) {
        obj->Update(dt);
    }
}

void SceneBattle::Render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    
    SDL_RenderDrawRect(renderer, &enemyPreparationZone);
    SDL_RenderDrawRect(renderer, &enemyBattleZone);
    SDL_RenderDrawRect(renderer, &playerBattleZone);
    SDL_RenderDrawRect(renderer, &playerPreparationZone);

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderFillRect(renderer, &btnBuyCard);

    for (auto obj : objects) {
        obj->Render(renderer);
    }
}

void SceneBattle::OrganizeZone(std::vector<Card*>& zoneCards, SDL_Rect zoneRect) {
    int numCards = zoneCards.size();
    if (numCards == 0) return;

    int cardWidth = 100;
    int cardHeight = 140;
    int gap = 15; 

    int totalWidth = (numCards * cardWidth) + ((numCards - 1) * gap);
    int startX = (zoneRect.x + (zoneRect.w / 2)) - (totalWidth / 2);
    int yPos = zoneRect.y + (zoneRect.h - cardHeight) / 2;

    for (int i = 0; i < numCards; ++i) {
        int cardX = startX + i * (cardWidth + gap);
        zoneCards[i]->SetPosition(cardX, yPos); 
    }
}

void SceneBattle::AddCardToPlayerPreparation(Card* card) {
    if (playerPreparationCards.size() < 6) {
        playerPreparationCards.push_back(card);
        objects.push_back(card);

        OrganizeZone(playerPreparationCards, playerPreparationZone);

        std::cout << card->GetName()
                  << " adicionado ao campo de Preparacao do Jogador!" << std::endl;

        if (auto* creature = dynamic_cast<CreatureCard*>(card)) {
            std::cout << creature->GetAttack()
                      << " de ATK e "
                      << creature->GetHealth()
                      << " de HP." << std::endl;
        }
    } else {
        std::cout << "Campo de Preparacao esta cheio! Limite de 6 cartas." << std::endl;
    }
}