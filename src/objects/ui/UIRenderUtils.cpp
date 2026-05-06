#include "UIRenderUtils.hpp"
#include <iostream>
#include <memory>
#include <unordered_map>

namespace {

using FontPtr = std::shared_ptr<TTF_Font>;

std::unordered_map<std::string, FontPtr> &GetFontCache() {
    static std::unordered_map<std::string, FontPtr> cache;
    return cache;
}

std::string MakeKey(const std::string &path, int size) { return path + "#" + std::to_string(size); }

} // namespace

namespace ui {

TTF_Font *UIRenderUtils::LoadFont(const std::string &path, int size) {
    if (path.empty() || size <= 0) return nullptr;

    auto &cache = GetFontCache();
    const std::string key = MakeKey(path, size);

    const auto found = cache.find(key);
    if (found != cache.end()) return found->second.get();

    TTF_Font *rawFont = TTF_OpenFont(path.c_str(), size);
    if (!rawFont) {
        std::cerr << "Falha ao carregar fonte '" << path << "' (" << size << "): " << TTF_GetError()
                  << std::endl;
        return nullptr;
    }

    cache[key] = FontPtr(rawFont, TTF_CloseFont);
    return cache[key].get();
}

void UIRenderUtils::RenderText(SDL_Renderer *renderer, const std::string &text, int x, int y,
                               SDL_Color color, TTF_Font *font) {
    if (!renderer || !font || text.empty()) return;

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int w = 0;
    int h = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void UIRenderUtils::RenderButton(SDL_Renderer *renderer, const SDL_Rect &rect,
                                 const std::string &label, TTF_Font *font, bool hovered,
                                 SDL_Color normalColor, SDL_Color hoverColor, SDL_Color borderColor,
                                 SDL_Color textColor) {
    if (!renderer) return;

    const SDL_Color fillColor = hovered ? hoverColor : normalColor;
    SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &rect);

    if (!font || label.empty()) return;

    int textW = 0;
    int textH = 0;
    TTF_SizeUTF8(font, label.c_str(), &textW, &textH);

    const int textX = rect.x + (rect.w - textW) / 2;
    const int textY = rect.y + (rect.h - textH) / 2;
    RenderText(renderer, label, textX, textY, textColor, font);
}

} // namespace ui
