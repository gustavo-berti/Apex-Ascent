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
    fontPanel = ui::UIRenderUtils::LoadFont("assets/fonts/arial.ttf", 18);

    btnBack = {40, 30, 160, 50};
    btnClear = {panelX, 30, panelW, 50};

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

// ═══════════════════════════════════════════════════════════════════
//  Selecao de deck
// ═══════════════════════════════════════════════════════════════════

int SceneCollection::GetSelectedCount(int cardId) const {
    const auto &selection = gameManager.GetPlayerDeckSelection();
    auto it = selection.find(cardId);
    return it != selection.end() ? it->second : 0;
}

int SceneCollection::GetTotalSelected() const {
    int total = 0;
    for (const auto &[id, count] : gameManager.GetPlayerDeckSelection())
        total += count;
    return total;
}

void SceneCollection::AddCopy(int cardId) {
    if (GetSelectedCount(cardId) >= kMaxCopies) {
        std::cout << "[COLECAO] Limite de " << kMaxCopies << " copias atingido." << std::endl;
        return;
    }
    if (GetTotalSelected() >= kDeckTarget) {
        std::cout << "[COLECAO] Deck ja tem " << kDeckTarget << " cartas." << std::endl;
        return;
    }

    gameManager.GetPlayerDeckSelection()[cardId]++;
}

void SceneCollection::RemoveCopy(int cardId) {
    auto &selection = gameManager.GetPlayerDeckSelection();
    auto it = selection.find(cardId);
    if (it == selection.end() || it->second <= 0) return;

    if (--(it->second) <= 0) selection.erase(it);
}

Card *SceneCollection::FindCardAtPoint(int x, int y) const {
    for (Card *card : allCards) {
        const int drawY = card->GetY() - scrollOffset;
        if (drawY + cardHeight < marginTop - 10 || drawY > screenH) continue;

        SDL_Rect r = {card->GetX(), drawY, cardWidth, cardHeight};
        if (GameManager::IsPointInsideRect(x, y, r)) return card;
    }
    return nullptr;
}

void SceneCollection::HandleInput(SDL_Event &event) {
    if (event.type == SDL_MOUSEWHEEL) {
        scrollOffset -= event.wheel.y * 40;
        scrollOffset = std::clamp(scrollOffset, 0, maxScroll);
        return;
    }

    if (event.type != SDL_MOUSEBUTTONDOWN) return;

    if (event.button.button == SDL_BUTTON_LEFT) {
        if (GameManager::IsPointInsideRect(event.button.x, event.button.y, btnBack)) {
            SceneMenu *menu = new SceneMenu(gameManager);
            menu->Initialize(renderer);
            gameManager.ChangeScene(menu); // "this" é destruído aqui dentro
            return;
        }

        if (GameManager::IsPointInsideRect(event.button.x, event.button.y, btnClear)) {
            gameManager.GetPlayerDeckSelection().clear();
            std::cout << "[COLECAO] Selecao de deck limpa." << std::endl;
            return;
        }
    }

    if (event.button.button != SDL_BUTTON_LEFT && event.button.button != SDL_BUTTON_RIGHT) return;

    Card *clicked = FindCardAtPoint(event.button.x, event.button.y);
    if (!clicked) return;

    if (event.button.button == SDL_BUTTON_LEFT)
        AddCopy(clicked->GetId());
    else
        RemoveCopy(clicked->GetId());
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
        RenderSelectionBadge(renderer, card, GetSelectedCount(card->GetId()));
        card->SetPosition(card->GetX(), originalY); // restaura a posição "lógica"
    }

    const SDL_Color borderColor = {255, 255, 255, 255};
    const SDL_Color textColor = {255, 255, 255, 255};
    ui::UIRenderUtils::RenderButton(renderer, btnBack, "Voltar", font, false, {80, 80, 200, 255},
                                    {120, 120, 240, 255}, borderColor, textColor);
    ui::UIRenderUtils::RenderButton(renderer, btnClear, "Limpar selecao", font, false,
                                    {150, 60, 60, 255}, {190, 90, 90, 255}, borderColor, textColor);

    RenderDeckPanel(renderer);
}

void SceneCollection::RenderSelectionBadge(SDL_Renderer *renderer, const Card *card,
                                           int count) const {
    if (count <= 0) return;

    constexpr int badgeSize = 24;
    SDL_Rect badge = {card->GetX() + card->GetWidth() - badgeSize - 4, card->GetY() + 4, badgeSize,
                      badgeSize};

    SDL_SetRenderDrawColor(renderer, 40, 160, 90, 255);
    SDL_RenderFillRect(renderer, &badge);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &badge);

    if (!font) return;
    std::string text = "x" + std::to_string(count);
    int textW = 0, textH = 0;
    TTF_SizeUTF8(font, text.c_str(), &textW, &textH);
    ui::UIRenderUtils::RenderText(renderer, text, badge.x + (badge.w - textW) / 2,
                                  badge.y + (badge.h - textH) / 2, {255, 255, 255, 255}, font);
}

// Painel lateral: mostra o deck sendo montado (nome + copias) e o total X/30.
// Fica sempre visivel, sem scroll proprio — se a lista nao couber, mostra
// "+N outra(s)..." no final em vez de estourar o painel.
void SceneCollection::RenderDeckPanel(SDL_Renderer *renderer) const {
    SDL_Rect panel = {panelX, marginTop, panelW, screenH - marginTop - 40};

    SDL_SetRenderDrawColor(renderer, 25, 25, 45, 230);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &panel);

    if (!fontPanel) return;

    const int total = GetTotalSelected();
    const std::string header =
        "Meu Deck: " + std::to_string(total) + "/" + std::to_string(kDeckTarget);
    const SDL_Color headerColor =
        (total >= kDeckTarget) ? SDL_Color{120, 230, 140, 255} : SDL_Color{255, 220, 120, 255};
    ui::UIRenderUtils::RenderText(renderer, header, panel.x + 12, panel.y + 10, headerColor,
                                  fontPanel);

    if (total == 0) {
        const SDL_Color hint = {180, 180, 200, 255};
        ui::UIRenderUtils::RenderText(renderer, "Nenhuma carta escolhida.", panel.x + 12,
                                      panel.y + 44, hint, fontPanel);
        ui::UIRenderUtils::RenderText(renderer, "O deck sera sorteado", panel.x + 12,
                                      panel.y + 66, hint, fontPanel);
        ui::UIRenderUtils::RenderText(renderer, "automaticamente.", panel.x + 12, panel.y + 88,
                                      hint, fontPanel);
    }

    struct Entry {
        std::string name;
        int count;
    };
    std::vector<Entry> entries;

    for (const auto &[id, count] : gameManager.GetPlayerDeckSelection()) {
        if (count <= 0) continue;
        std::string name;
        if (const CreatureData *c = cardDatabase.GetCreature(id))
            name = c->name;
        else if (const SpellData *s = cardDatabase.GetSpell(id))
            name = s->name;
        else
            continue;
        entries.push_back({name, count});
    }

    std::sort(entries.begin(), entries.end(),
             [](const Entry &a, const Entry &b) { return a.name < b.name; });

    const int lineHeight = 22;
    const int listTop = panel.y + 46;
    const int maxLines = std::max(1, (panel.y + panel.h - listTop - 8) / lineHeight);
    const bool willOverflow = static_cast<int>(entries.size()) > maxLines;
    const int linesForEntries = willOverflow ? maxLines - 1 : maxLines;

    int shown = 0;
    for (const Entry &entry : entries) {
        if (shown >= linesForEntries) break;
        std::string line = entry.name + "  x" + std::to_string(entry.count);
        ui::UIRenderUtils::RenderText(renderer, line, panel.x + 12, listTop + shown * lineHeight,
                                      SDL_Color{230, 230, 240, 255}, fontPanel);
        ++shown;
    }

    if (static_cast<int>(entries.size()) > shown) {
        std::string more = "+" + std::to_string(entries.size() - shown) + " outra(s)...";
        ui::UIRenderUtils::RenderText(renderer, more, panel.x + 12, listTop + shown * lineHeight,
                                      SDL_Color{160, 160, 180, 255}, fontPanel);
    }
}
