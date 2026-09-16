#include "UIButton.hpp"
#include "UIRenderUtils.hpp"

namespace {

bool Contains(const SDL_Rect &rect, int x, int y) {
    return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

} // namespace

namespace ui {

int FindButtonAt(const std::vector<UIButton> &buttons, int x, int y) {
    for (int i = 0; i < static_cast<int>(buttons.size()); ++i)
        if (Contains(buttons[i].rect, x, y)) return i;
    return -1;
}

bool DispatchClick(const std::vector<UIButton> &buttons, int x, int y) {
    const int index = FindButtonAt(buttons, x, y);
    if (index < 0 || !buttons[index].onClick) return false;

    // A copia mantem o callback vivo mesmo quando ele limpa a lista de botoes
    // (troca de tela) ou destroi a cena inteira (troca de cena).
    const std::function<void()> callback = buttons[index].onClick;
    callback();
    return true;
}

void RenderButtons(SDL_Renderer *renderer, const std::vector<UIButton> &buttons, TTF_Font *font,
                   int hoveredIndex) {
    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        const UIButton &button = buttons[i];
        UIRenderUtils::RenderButton(renderer, button.rect, button.label, font, i == hoveredIndex,
                                    button.style.normal, button.style.hover, button.style.border,
                                    button.style.text);
    }
}

} // namespace ui
