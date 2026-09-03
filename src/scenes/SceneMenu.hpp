#pragma once
#include "../objects/cards/types/CardTypes.hpp"
#include "SceneUI.hpp"

// Menu principal e a tela de escolha do oponente, que aparece depois do
// "Começar Jogo". As duas telas sao a mesma cena: mudam so os botoes.
class SceneMenu : public SceneUI {
  private:
    static constexpr int kRaceCount = 5;
    static constexpr int kLevelCount = 5;

    // Race::NONE fica de fora: e o baralho de teste, nao uma raça jogavel.
    static constexpr Race kRaces[kRaceCount] = {Race::HUMAN, Race::AUTOMAT, Race::PIXIE,
                                                Race::DRAGON, Race::DRYAD};

    // Viram deckType e deckPart do Opponent.
    Race selectedRace = Race::PIXIE;
    int selectedLevel = 1;

    bool showingOpponentSetup = false;

    void ShowMainScreen();
    void ShowOpponentSetup(); // botoes: [0..4] raças, [5..9] niveis, Lutar, Voltar
    void SelectRace(Race race);
    void SelectLevel(int level);
    void StartOpponentBattle(); // destroi a cena: nada pode rodar depois

  protected:
    void RenderContent(SDL_Renderer *renderer) override;

  public:
    explicit SceneMenu(GameManager &manager);

    void Initialize(SDL_Renderer *renderer) override;
};
