#pragma once
#include "SceneUI.hpp"
#include <functional>

// Menu de pausa da batalha. Diferente das outras telas de menu, esta nao e a
// cena atual do GameManager: ela fica por cima da SceneBattle, que continua
// viva (so congelada) e e quem repassa o input e o render para ca.
class ScenePause : public SceneUI {
  private:
    std::function<void()> onResume;
    bool confirmingExit = false;

    void ShowMainScreen();
    void ShowExitConfirmation(); // "Voltar ao menu" so sai depois de confirmar
    void ReturnToMenu();         // destroi a batalha, dona desta cena: nada pode rodar depois

  protected:
    void RenderBackground(SDL_Renderer *renderer) override;
    void RenderContent(SDL_Renderer *renderer) override;

  public:
    ScenePause(GameManager &manager, std::function<void()> resumeCallback);

    void Initialize(SDL_Renderer *renderer) override;
};
