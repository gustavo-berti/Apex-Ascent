#include "SceneBattle.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include "../objects/cards/SpellCard.hpp"
#include "../core/GameManager.hpp"
#include "../logic/Player.hpp"
#include "../logic/CardFactory.hpp"
#include <algorithm>
#include <iostream>
#include <random>

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
    playerHandZone = { 0, 570, 1280, 150 };

    btnBuyCard = { 1150, 600, 150, 50 };
}

bool SceneBattle::SetCurrentPlayerState(Player* playerState) {
    if (!playerState) {
        std::cout << "Estado do jogador invalido. Nao foi possivel iniciar a batalha." << std::endl;
        return false;
    }

    currentState = playerState;
    return true;
}

void SceneBattle::ResetBattleDeckState() {
    drawPile.clear();
    hand.clear();
    discardPile.clear();
}

void SceneBattle::AddDeckCardToDrawPile(const std::string& cardId) {
    int creatureId = 0;
    try {
        creatureId = std::stoi(cardId);
    } catch (...) {
        std::cout << "ID de carta invalido no deck: " << cardId << std::endl;
        return;
    }

    CreatureCard* createdCard = CardFactory::CreateCreatureCard(cardDatabase, creatureId);
    if (createdCard) {
        createdCard->SetPosition(-200, -200);
        drawPile.push_back(createdCard);
        objects.push_back(createdCard);
    }
}

void SceneBattle::BuildDrawPileFromMasterDeck() {
    for (const std::string& cardId : currentState->masterDeck) {
        AddDeckCardToDrawPile(cardId);
    }
}

void SceneBattle::ShuffleDrawPile() {
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::shuffle(drawPile.begin(), drawPile.end(), rng);
}

void SceneBattle::StartBattle(Player* playerState) {
    if (!SetCurrentPlayerState(playerState)) {
        return;
    }

    std::cout << "Montando o Battle Deck a partir do Master Deck...\n";

    ResetBattleDeckState();
    BuildDrawPileFromMasterDeck();
    ShuffleDrawPile();

    DrawCards(5);
}

void SceneBattle::DrawCards(int amount) {
    for (int i = 0; i < amount; ++i) {
        if (drawPile.empty()) {
            std::cout << "Pilha de compra vazia! Num jogo real, você perderia aqui.\n";
            break;
        }

        Card* drawnCard = drawPile.back();
        drawPile.pop_back();
        hand.push_back(drawnCard);
    }
    OrganizeZone(hand, playerHandZone);
}

bool SceneBattle::IsBuyCardButtonClick(const SDL_Event& event) const {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) {
        return false;
    }

    return GameManager::IsPointInsideRect(event.button.x, event.button.y, btnBuyCard);
}

bool SceneBattle::IsHandCardClick(const SDL_Event& event, const Card* card) const {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT || !card) {
        return false;
    }

    SDL_Rect cardRect = { card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight() };
    return GameManager::IsPointInsideRect(event.button.x, event.button.y, cardRect);
}

void SceneBattle::HandleBuyCardAction() {
    std::cout << "Botao clicado! Puxando a proxima carta do jogador..." << std::endl;
    DrawCards(1);
}

void SceneBattle::HandleHandCardAction(const SDL_Event& event) {
    for (auto it = hand.rbegin(); it != hand.rend(); ++it) {
        Card* card = *it;

        if (!IsHandCardClick(event, card)) {
            continue;
        }

        if (!AddCardToPlayerPreparation(card)) {
            return;
        }

        hand.erase(std::next(it).base());
        OrganizeZone(hand, playerHandZone);
        return;
    }
}

void SceneBattle::HandleInput(SDL_Event& event) {
    if (IsBuyCardButtonClick(event)) {
        HandleBuyCardAction();
        return;
    }

    HandleHandCardAction(event);
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

    for (auto* card : playerPreparationCards) {
        card->Render(renderer);
    }

    for (auto* card : playerBattleCards) {
        card->Render(renderer);
    }

    for (auto* card : hand) {
        card->Render(renderer);
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

bool SceneBattle::AddCardToPlayerPreparation(Card* card) {
    if (playerPreparationCards.size() < 6) {
        playerPreparationCards.push_back(card);

        OrganizeZone(playerPreparationCards, playerPreparationZone);

        std::cout << card->GetName()
                  << " adicionado ao campo de Preparacao do Jogador!" << std::endl;

        if (auto* creature = dynamic_cast<CreatureCard*>(card)) {
            std::cout << creature->GetAttack()
                      << " de ATK e "
                      << creature->GetHealth()
                      << " de HP." << std::endl;
        }

        return true;
    } else {
        std::cout << "Campo de Preparacao esta cheio! Limite de 6 cartas." << std::endl;
        return false;
    }
}