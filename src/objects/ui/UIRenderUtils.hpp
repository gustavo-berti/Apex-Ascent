#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

namespace ui {

class UIRenderUtils {
  public:
    static TTF_Font *LoadFont(const std::string &path, int size);

    static void RenderText(SDL_Renderer *renderer, const std::string &text, int x, int y,
                           SDL_Color color, TTF_Font *font);

    static void RenderButton(SDL_Renderer *renderer, const SDL_Rect &rect, const std::string &label,
                             TTF_Font *font, bool hovered,
                             SDL_Color normalColor = {60, 60, 100, 255},
                             SDL_Color hoverColor = {100, 100, 180, 255},
                             SDL_Color borderColor = {180, 180, 255, 255},
                             SDL_Color textColor = {255, 255, 255, 255});
};

} // namespace ui
