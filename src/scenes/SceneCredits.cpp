#include "SceneCredits.hpp"
#include "../core/GameManager.hpp"
#include "../objects/ui/UIRenderUtils.hpp"
#include "SceneMenu.hpp"
#include <algorithm>
#include <iostream>

namespace {

// ── Layout (tela de 1600x900) ─────────────────────────────────────
constexpr int kCreatorsLabelY = 140;
constexpr int kCreatorsY = 180;
constexpr int kCreatorCardW = 360;
constexpr int kCreatorCardH = 330;
constexpr int kMentionsLabelY = 545;
constexpr int kMentionsY = 580;
constexpr int kMentionCardW = 560;
constexpr int kMentionCardH = 170;
constexpr int kCardGap = 50;
constexpr int kCreatorPhoto = 150;
constexpr int kMentionPhoto = 100;
constexpr int kQuoteMaxLines = 3;

constexpr SDL_Color kWhite = {255, 255, 255, 255};
constexpr SDL_Color kLabel = {255, 220, 80, 255};
constexpr SDL_Color kRole = {180, 190, 220, 255};
constexpr SDL_Color kQuote = {200, 215, 255, 255};

int TextWidth(TTF_Font *font, const std::string &text) {
    int w = 0;
    int h = 0;
    if (font) TTF_SizeUTF8(font, text.c_str(), &w, &h);
    return w;
}

int TextHeight(TTF_Font *font) { return font ? TTF_FontHeight(font) : 0; }

void RenderCenteredIn(SDL_Renderer *renderer, TTF_Font *font, const std::string &text, int x,
                      int width, int y, SDL_Color color) {
    if (!font || text.empty()) return;
    ui::UIRenderUtils::RenderText(renderer, text, x + (width - TextWidth(font, text)) / 2, y, color,
                                  font);
}

// Quebra a frase em linhas que cabem na largura pedida. Palavra sozinha maior
// que a linha fica na propria linha e vaza um pouco: e melhor que sumir.
std::vector<std::string> WrapText(TTF_Font *font, const std::string &text, int maxWidth) {
    std::vector<std::string> lines;
    if (!font || text.empty()) return lines;

    std::string line;
    size_t pos = 0;

    while (pos <= text.size()) {
        const size_t space = text.find(' ', pos);
        const std::string word = text.substr(pos, space - pos);

        const std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && TextWidth(font, candidate) > maxWidth) {
            lines.push_back(line);
            line = word;
        } else {
            line = candidate;
        }

        if (space == std::string::npos) break;
        pos = space + 1;
    }

    if (!line.empty()) lines.push_back(line);

    // Frase comprida demais: corta e avisa com reticencias.
    if (static_cast<int>(lines.size()) > kQuoteMaxLines) {
        lines.resize(kQuoteMaxLines);
        lines.back() += "...";
    }

    return lines;
}

// Primeiro caractere do nome, inteiro (um acento ocupa mais de um byte).
std::string FirstCharacter(const std::string &text) {
    if (text.empty()) return "?";

    size_t length = 1;
    while (length < text.size() && (static_cast<unsigned char>(text[length]) & 0xC0) == 0x80)
        ++length;

    return text.substr(0, length);
}

} // namespace

SceneCredits::SceneCredits(GameManager &manager) : SceneUI(manager) {}

void SceneCredits::Initialize(SDL_Renderer *renderer) {
    SceneUI::Initialize(renderer);
    LoadBackground(renderer, "assets/images/start_menu.png");
    fontSmall = ui::UIRenderUtils::LoadFont("assets/fonts/arial.ttf", 18);
    title = "Créditos";

    if (!credits.Load())
        std::cerr << "[CREDITOS] Tela aberta sem nenhum credito pra mostrar." << std::endl;

    creatorPhotos = LoadPhotos(renderer, credits.GetCreators());
    mentionPhotos = LoadPhotos(renderer, credits.GetMentions());

    const int btnW = 200, btnH = 50;
    buttons.push_back({{(screenWidth - btnW) / 2, screenHeight - 110, btnW, btnH}, "Voltar",
                       [this] {
                           SceneMenu *menu = new SceneMenu(gameManager);
                           menu->Initialize(gameManager.GetRenderer());
                           gameManager.ChangeScene(menu); // "this" e destruido aqui dentro
                       }});
}

std::vector<SDL_Texture *> SceneCredits::LoadPhotos(SDL_Renderer *renderer,
                                                    const std::vector<CreditPerson> &people) const {
    std::vector<SDL_Texture *> photos;
    photos.reserve(people.size());

    for (const CreditPerson &person : people)
        photos.push_back(person.photo.empty()
                             ? nullptr
                             : ui::UIRenderUtils::LoadTexture(renderer, person.photo));

    return photos;
}

// ── Render ────────────────────────────────────────────────────────

void SceneCredits::RenderContent(SDL_Renderer *renderer) {
    if (credits.IsEmpty()) {
        RenderCenteredText(renderer, font, "Nao consegui ler assets/data/credits.json.", 300,
                           kWhite);
        return;
    }

    const auto &creators = credits.GetCreators();
    const auto &mentions = credits.GetMentions();

    if (!creators.empty()) {
        RenderCenteredText(renderer, font, "Criadores", kCreatorsLabelY, kLabel);
        RenderRow(renderer, creators, creatorPhotos, kCreatorsY, kCreatorCardW, kCreatorCardH,
                  true);
    }

    if (mentions.empty()) return;

    const char *label = mentions.size() > 1 ? "Menções honrosas" : "Menção honrosa";
    RenderCenteredText(renderer, font, label, kMentionsLabelY, kLabel);
    RenderRow(renderer, mentions, mentionPhotos, kMentionsY, kMentionCardW, kMentionCardH, false);
}

void SceneCredits::RenderRow(SDL_Renderer *renderer, const std::vector<CreditPerson> &people,
                             const std::vector<SDL_Texture *> &photos, int y, int cardW, int cardH,
                             bool vertical) const {
    const int count = static_cast<int>(people.size());
    if (count <= 0) return;

    // Muita gente numa fileira so: encolhe o cartao ate a fileira caber.
    const int margin = 40;
    const int available = screenWidth - margin * 2 - (count - 1) * kCardGap;
    const int width = std::max(160, std::min(cardW, available / count));
    const int rowW = count * width + (count - 1) * kCardGap;
    const int x = (screenWidth - rowW) / 2;

    for (int i = 0; i < count; ++i) {
        const SDL_Rect card = {x + i * (width + kCardGap), y, width, cardH};
        SDL_Texture *photo = i < static_cast<int>(photos.size()) ? photos[i] : nullptr;

        if (vertical)
            RenderCreatorCard(renderer, people[i], photo, card);
        else
            RenderMentionCard(renderer, people[i], photo, card);
    }
}

void SceneCredits::RenderPanel(SDL_Renderer *renderer, const SDL_Rect &rect) const {
    // Painel escuro: o fundo do menu deixaria os textos ilegiveis.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 180, 180, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

void SceneCredits::RenderCreatorCard(SDL_Renderer *renderer, const CreditPerson &person,
                                     SDL_Texture *photoTexture, const SDL_Rect &card) const {
    RenderPanel(renderer, card);

    const SDL_Rect photo = {card.x + (card.w - kCreatorPhoto) / 2, card.y + 20, kCreatorPhoto,
                            kCreatorPhoto};
    RenderPhoto(renderer, person, photoTexture, photo);

    int y = photo.y + photo.h + 18;
    RenderCenteredIn(renderer, font, person.name, card.x, card.w, y, kWhite);
    y += TextHeight(font) + 4;

    for (const std::string &line : WrapText(fontSmall, person.role, card.w - 40)) {
        RenderCenteredIn(renderer, fontSmall, line, card.x, card.w, y, kRole);
        y += TextHeight(fontSmall) + 2;
    }
    if (!person.role.empty()) y += 6;

    for (const std::string &line : WrapText(fontSmall, person.quote, card.w - 40)) {
        RenderCenteredIn(renderer, fontSmall, line, card.x, card.w, y, kQuote);
        y += TextHeight(fontSmall) + 2;
    }
}

void SceneCredits::RenderMentionCard(SDL_Renderer *renderer, const CreditPerson &person,
                                     SDL_Texture *photoTexture, const SDL_Rect &card) const {
    RenderPanel(renderer, card);

    const SDL_Rect photo = {card.x + 20, card.y + (card.h - kMentionPhoto) / 2, kMentionPhoto,
                            kMentionPhoto};
    RenderPhoto(renderer, person, photoTexture, photo);

    const int textX = photo.x + photo.w + 20;
    const int textW = card.x + card.w - 20 - textX;
    int y = card.y + 22;

    ui::UIRenderUtils::RenderText(renderer, person.name, textX, y, kWhite, font);
    y += TextHeight(font) + 4;

    for (const std::string &line : WrapText(fontSmall, person.role, textW)) {
        ui::UIRenderUtils::RenderText(renderer, line, textX, y, kRole, fontSmall);
        y += TextHeight(fontSmall) + 2;
    }
    if (!person.role.empty()) y += 6;

    for (const std::string &line : WrapText(fontSmall, person.quote, textW)) {
        ui::UIRenderUtils::RenderText(renderer, line, textX, y, kQuote, fontSmall);
        y += TextHeight(fontSmall) + 2;
    }
}

void SceneCredits::RenderPhoto(SDL_Renderer *renderer, const CreditPerson &person,
                               SDL_Texture *texture, const SDL_Rect &dst) const {
    if (texture) {
        int w = 0;
        int h = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

        // Recorte quadrado no centro: foto retangular entra sem esticar.
        const int side = std::min(w, h);
        const SDL_Rect src = {(w - side) / 2, (h - side) / 2, side, side};
        SDL_RenderCopy(renderer, texture, &src, &dst);
    } else {
        // Sem foto: quadro com a inicial do nome.
        SDL_SetRenderDrawColor(renderer, 45, 45, 80, 255);
        SDL_RenderFillRect(renderer, &dst);
        RenderCenteredIn(renderer, fontTitle, FirstCharacter(person.name), dst.x, dst.w,
                         dst.y + (dst.h - TextHeight(fontTitle)) / 2, kRole);
    }

    SDL_SetRenderDrawColor(renderer, 180, 180, 255, 255);
    SDL_RenderDrawRect(renderer, &dst);
}
