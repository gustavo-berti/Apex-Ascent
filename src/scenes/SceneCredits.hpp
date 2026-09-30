#pragma once
#include "../core/data/Credits.hpp"
#include "SceneUI.hpp"
#include <vector>

// Tela de creditos, aberta pelo menu principal. Nao tem nada fixo no codigo:
// os nomes, as frases e as fotos vem de assets/data/credits.json, e a tela se
// ajusta a quantas pessoas estiverem la.
class SceneCredits : public SceneUI {
  private:
    Credits credits;
    TTF_Font *fontSmall = nullptr; // papel e frase de efeito

    // Uma textura por pessoa, na mesma ordem do JSON (nullptr = sem foto ou
    // caminho errado). Carregadas uma unica vez, no Initialize: assim um
    // caminho invalido reclama uma vez so, e nao a cada frame. Quem destroi as
    // texturas e o cache de UIRenderUtils, no fim do jogo.
    std::vector<SDL_Texture *> creatorPhotos;
    std::vector<SDL_Texture *> mentionPhotos;

    std::vector<SDL_Texture *> LoadPhotos(SDL_Renderer *renderer,
                                          const std::vector<CreditPerson> &people) const;

    // Cartao vertical (foto em cima, textos embaixo) — usado nos criadores.
    void RenderCreatorCard(SDL_Renderer *renderer, const CreditPerson &person, SDL_Texture *photo,
                           const SDL_Rect &card) const;

    // Cartao deitado (foto a esquerda, textos a direita) — usado nas mençoes.
    void RenderMentionCard(SDL_Renderer *renderer, const CreditPerson &person, SDL_Texture *photo,
                           const SDL_Rect &card) const;

    // Foto cortada no centro pra caber no quadrado. Sem textura, desenha a
    // inicial do nome no lugar.
    void RenderPhoto(SDL_Renderer *renderer, const CreditPerson &person, SDL_Texture *photo,
                     const SDL_Rect &dst) const;

    // Uma fileira de cartoes centralizada na tela.
    void RenderRow(SDL_Renderer *renderer, const std::vector<CreditPerson> &people,
                   const std::vector<SDL_Texture *> &photos, int y, int cardW, int cardH,
                   bool vertical) const;

    void RenderPanel(SDL_Renderer *renderer, const SDL_Rect &rect) const;

  protected:
    void RenderContent(SDL_Renderer *renderer) override;

  public:
    explicit SceneCredits(GameManager &manager);

    void Initialize(SDL_Renderer *renderer) override;
};
