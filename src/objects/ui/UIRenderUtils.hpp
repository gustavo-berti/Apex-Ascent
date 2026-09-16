#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

namespace ui {

class UIRenderUtils {
  public:
    static TTF_Font *LoadFont(const std::string &path, int size);

    // Cache global de texturas por caminho de arquivo: a primeira chamada decodifica
    // a imagem e cria a textura; chamadas seguintes (mesmo de outra cena) reusam o
    // mesmo SDL_Texture*. O chamador NUNCA deve dar SDL_DestroyTexture nela —
    // o dono e o cache, liberado via ClearTextureCache().
    static SDL_Texture *LoadTexture(SDL_Renderer *renderer, const std::string &path);
    static void ClearTextureCache();

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
