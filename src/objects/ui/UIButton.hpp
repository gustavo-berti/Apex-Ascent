#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <functional>
#include <string>
#include <vector>

namespace ui {

struct UIButtonStyle {
    SDL_Color normal{60, 60, 100, 255};
    SDL_Color hover{100, 100, 180, 255};
    SDL_Color border{180, 180, 255, 255};
    SDL_Color text{255, 255, 255, 255};
};

namespace styles {
inline constexpr UIButtonStyle kDefault{};
inline constexpr UIButtonStyle kSelected{
    {40, 130, 70, 255}, {60, 180, 100, 255}, {160, 255, 190, 255}, {255, 255, 255, 255}};
inline constexpr UIButtonStyle kPrimary{
    {220, 160, 0, 255}, {250, 200, 40, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}};
inline constexpr UIButtonStyle kSecondary{
    {80, 80, 200, 255}, {120, 120, 240, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}};
} // namespace styles

struct UIButton {
    SDL_Rect rect{};
    std::string label;
    std::function<void()> onClick;
    UIButtonStyle style{};
};

// Indice do botao sob o ponto, ou -1.
int FindButtonAt(const std::vector<UIButton> &buttons, int x, int y);

// Dispara o callback do botao sob o ponto. O callback pode refazer a lista de
// botoes ou ate destruir a cena dona dela, entao nada aqui toca em `buttons`
// depois da chamada.
bool DispatchClick(const std::vector<UIButton> &buttons, int x, int y);

void RenderButtons(SDL_Renderer *renderer, const std::vector<UIButton> &buttons, TTF_Font *font,
                   int hoveredIndex);

} // namespace ui
