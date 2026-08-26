#include "SceneCollection.hpp"
#include "../core/GameManager.hpp"
#include "../logic/CardFactory.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include "../objects/cards/SpellCard.hpp"
#include "../objects/ui/UIRenderUtils.hpp"
#include "SceneMenu.hpp"
#include <algorithm>
#include <iostream>

SceneCollection::SceneCollection(GameManager &gm) : gameManager(gm) {}

// allCards guarda os mesmos ponteiros que "objects" (herdado de GameWorld),
// que já se encarrega de dar delete em todos eles no destrutor.
SceneCollection::~SceneCollection() {}

void SceneCollection::Initialize(SDL_Renderer *renderer_) {
    renderer = renderer_;

    if (!cardDatabase.LoadFromJson("assets/data/cards.json"))
        std::cerr << "Falha ao carregar cards.json na colecao" << std::endl;

    font = ui::UIRenderUtils::LoadFont("assets/fonts/arial.ttf", 20);
    fontTitle = ui::UIRenderUtils::LoadFont("assets/fonts/frozen.ttf", 48);

    btnBack = {40, 30, 160, 50};

    BuildCollection();
    ArrangeGrid();

    for (Card *card : allCards)
        card->LoadTexture(renderer);
}

void SceneCollection::BuildCollection() {
    // Criaturas, em ordem de id
    std::vector<int> creatureIds;
    for (const auto &[id, data] : cardDatabase.GetAllCreatures())
        creatureIds.push_back(id);
    std::sort(creatureIds.begin(), creatureIds.end());

    for (int id : creatureIds) {
        CreatureCard *card = CardFactory::CreateCreatureCard(cardDatabase, id);
        if (!card) continue;
        allCards.push_back(card);
        objects.push_back(card);
    }

    // Feitiços, em ordem de id
    std::vector<int> spellIds;
    for (const auto &[id, data] : cardDatabase.GetAllSpells())
        spellIds.push_back(id);
    std::sort(spellIds.begin(), spellIds.end());

    for (int id : spellIds) {
        const SpellData *data = cardDatabase.GetSpell(id);
        if (!data) continue;
        SpellCard *card = new SpellCard(data, 0, 0);
        allCards.push_back(card);
        objects.push_back(card);
    }
}

void SceneCollection::ArrangeGrid() {
    for (size_t i = 0; i < allCards.size(); ++i) {
        int col = static_cast<int>(i) % cols;
        int row = static_cast<int>(i) / cols;

        int x = marginLeft + col * (cardWidth + gapX);
        int y = marginTop + row * (cardHeight + gapY);

        allCards[i]->SetPosition(x, y);
    }

    int totalRows = (static_cast<int>(allCards.size()) + cols - 1) / cols;
    int totalHeight = marginTop + totalRows * (cardHeight + gapY);
    maxScroll = std::max(0, totalHeight - screenH + 40);
}

void SceneCollection::HandleInput(SDL_Event &event) {
    if (event.type == SDL_MOUSEWHEEL) {
        scrollOffset -= event.wheel.y * 40;
        scrollOffset = std::clamp(scrollOffset, 0, maxScroll);
        return;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (GameManager::IsPointInsideRect(event.button.x, event.button.y, btnBack)) {
            SceneMenu *menu = new SceneMenu(gameManager);
            menu->Initialize(renderer);
            gameManager.ChangeScene(menu); // "this" é destruído aqui dentro
            return;
        }
    }
}

void SceneCollection::Update(float dt) {
    for (auto obj : objects)
        obj->Update(dt);
}

void SceneCollection::Render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 15, 15, 30, 255);
    SDL_RenderClear(renderer);

    if (fontTitle) {
        SDL_Color white = {255, 255, 255, 255};
        ui::UIRenderUtils::RenderText(renderer, "Colecao de Cartas", 220, 40, white, fontTitle);
    }

    // Desenha só as cartas visíveis, aplicando o scroll na hora do render
    for (Card *card : allCards) {
        int drawY = card->GetY() - scrollOffset;

        if (drawY + cardHeight < marginTop - 10 || drawY > screenH) continue;

        int originalY = card->GetY();
        card->SetPosition(card->GetX(), drawY);
        card->Render(renderer);
        card->SetPosition(card->GetX(), originalY); // restaura a posição "lógica"
    }

    const SDL_Color borderColor = {255, 255, 255, 255};
    const SDL_Color textColor = {255, 255, 255, 255};
    ui::UIRenderUtils::RenderButton(renderer, btnBack, "Voltar", font, false, {80, 80, 200, 255},
                                    {120, 120, 240, 255}, borderColor, textColor);
}