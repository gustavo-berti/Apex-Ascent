#include "UITextField.hpp"
#include "UIRenderUtils.hpp"

namespace {

bool Contains(const SDL_Rect &rect, int x, int y) {
    return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

// Em UTF-8 so o primeiro byte de um caractere nao comeca com 10xxxxxx.
bool IsContinuationByte(char c) { return (static_cast<unsigned char>(c) & 0xC0) == 0x80; }

int CountCharacters(const std::string &text) {
    int count = 0;
    for (char c : text)
        if (!IsContinuationByte(c)) ++count;
    return count;
}

// Apaga o ultimo caractere inteiro: um acento ocupa dois bytes e sumir com so
// um deles deixaria a string invalida.
void PopCharacter(std::string &text) {
    while (!text.empty() && IsContinuationByte(text.back()))
        text.pop_back();
    if (!text.empty()) text.pop_back();
}

} // namespace

namespace ui {

bool UITextField::HandleEvent(const SDL_Event &event) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        const bool inside = Contains(rect, event.button.x, event.button.y);
        focused = inside;
        return inside;
    }

    if (!focused) return false;

    if (event.type == SDL_TEXTINPUT) {
        const std::string typed = event.text.text;

        // O SDL manda o texto ja composto; teclas de controle chegam como
        // SDL_KEYDOWN e nao devem virar caractere.
        if (typed.empty() || static_cast<unsigned char>(typed.front()) < 0x20) return true;
        if (CountCharacters(text) + CountCharacters(typed) > maxLength) return true;

        text += typed;
        return true;
    }

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_BACKSPACE:
            PopCharacter(text);
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_TAB:
            focused = false;
            return true;
        default:
            return false;
        }
    }

    return false;
}

std::string UITextField::Trimmed() const {
    const auto isSpace = [](char c) { return c == ' ' || c == '\t'; };

    size_t begin = 0;
    while (begin < text.size() && isSpace(text[begin]))
        ++begin;

    size_t end = text.size();
    while (end > begin && isSpace(text[end - 1]))
        --end;

    return text.substr(begin, end - begin);
}

void RenderTextField(SDL_Renderer *renderer, const UITextField &field, TTF_Font *font) {
    if (!renderer) return;

    SDL_SetRenderDrawColor(renderer, 25, 25, 45, 255);
    SDL_RenderFillRect(renderer, &field.rect);

    if (field.focused)
        SDL_SetRenderDrawColor(renderer, 160, 255, 190, 255);
    else
        SDL_SetRenderDrawColor(renderer, 180, 180, 255, 255);
    SDL_RenderDrawRect(renderer, &field.rect);

    if (!font) return;

    const bool empty = field.text.empty();
    const std::string shown = empty ? field.placeholder : field.text;
    const SDL_Color color = empty ? SDL_Color{150, 150, 170, 255} : SDL_Color{255, 255, 255, 255};

    int textW = 0;
    int textH = 0;
    TTF_SizeUTF8(font, shown.c_str(), &textW, &textH);

    const int padding = 12;
    const int textX = field.rect.x + padding;
    const int textY = field.rect.y + (field.rect.h - textH) / 2;
    UIRenderUtils::RenderText(renderer, shown, textX, textY, color, font);

    // Cursor piscando: so aparece com o campo em foco.
    if (!field.focused) return;
    if ((SDL_GetTicks() / 500) % 2 != 0) return;

    const int caretX = empty ? textX : textX + textW + 2;
    SDL_Rect caret = {caretX, field.rect.y + 10, 2, field.rect.h - 20};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &caret);
}

} // namespace ui
