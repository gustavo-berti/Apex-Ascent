#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

namespace ui {

// Campo de texto de uma linha. A cena guarda um destes, repassa os eventos e le
// `text` quando precisar. Quem mostra o campo tem que ligar o SDL_StartTextInput
// (e desligar ao sair da tela): sem isso o SDL nao manda SDL_TEXTINPUT.
struct UITextField {
    SDL_Rect rect{};
    std::string text;
    std::string placeholder; // cinza, enquanto o campo esta vazio
    int maxLength = 12;      // em caracteres, nao em bytes: um acento conta 1
    bool focused = false;

    // true quando o evento era do campo (clique dentro, digitacao, backspace).
    // O clique de fora so tira o foco e segue para os botoes.
    bool HandleEvent(const SDL_Event &event);

    // Sem espacos nas pontas: e este valor que vai para o placar.
    std::string Trimmed() const;
};

void RenderTextField(SDL_Renderer *renderer, const UITextField &field, TTF_Font *font);

} // namespace ui
