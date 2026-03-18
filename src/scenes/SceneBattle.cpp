#include "SceneBattle.hpp"
#include "../objects/CreatureCard.hpp"
#include "../objects/SpellCard.hpp"
#include <iostream>

SceneBattle::SceneBattle() {
    draggedCard = nullptr;
}

SceneBattle::~SceneBattle() {}

void SceneBattle::Initialize() {
    std::cout << "A inicializar a Cena de Batalha..." << std::endl;

    enemyPreparationZone  = { 280, 20,  720, 150 };
    enemyBattleZone       = { 280, 180, 720, 150 };
    playerBattleZone      = { 280, 390, 720, 150 };
    playerPreparationZone = { 280, 550, 720, 150 };

    btnBuyCard = { 1050, 600, 150, 50 };
}

void SceneBattle::HandleInput(SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mouseX = event.button.x;
        int mouseY = event.button.y;

        if (mouseX >= btnBuyCard.x && mouseX <= btnBuyCard.x + btnBuyCard.w &&
            mouseY >= btnBuyCard.y && mouseY <= btnBuyCard.y + btnBuyCard.h) {
            
            std::cout << "Botão clicado! Gerando carta..." << std::endl;
            CreatureCard* novaCarta = new CreatureCard("Guerreiro Teste", 2, Rarity::COMMON, "", 3, 5, "Humano", 0, 0, 0);

            AddCardToPlayerPreparation(novaCarta);
        }
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
    } else {
        std::cout << "Campo de Preparação está cheio! Limite de 6 cartas." << std::endl;
    }
}