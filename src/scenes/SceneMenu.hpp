#pragma once
#include "../core/data/ScoreBoard.hpp"
#include "../objects/cards/types/CardTypes.hpp"
#include "SceneUI.hpp"

// Menu principal, a tela de escolha do oponente (depois do "Começar Jogo") e o
// placar das melhores pontuacoes. As tres sao a mesma cena: mudam so os botoes.
class SceneMenu : public SceneUI {
  private:
    enum class Screen { MAIN, OPPONENT_SETUP, SCORES };

    static constexpr int kRaceCount = 5;
    static constexpr int kLevelCount = 5;

    // Race::NONE fica de fora: e o baralho de teste, nao uma raça jogavel.
    static constexpr Race kRaces[kRaceCount] = {Race::HUMAN, Race::AUTOMAT, Race::PIXIE,
                                                Race::DRAGON, Race::DRYAD};

    // Viram deckType e deckPart do Opponent.
    Race selectedRace = Race::PIXIE;
    int selectedLevel = 1;

    Screen screen = Screen::MAIN;
    ScoreBoard scores; // recarregado a cada vez que o placar abre

    void ShowMainScreen();
    void ShowOpponentSetup(); // botoes: [0..4] raças, [5..9] niveis, Lutar, Voltar
    void ShowScores();
    void SelectRace(Race race);
    void SelectLevel(int level);
    void StartOpponentBattle(); // destroi a cena: nada pode rodar depois
    void RenderScoreTable(SDL_Renderer *renderer) const;

  protected:
    void RenderContent(SDL_Renderer *renderer) override;

  public:
    explicit SceneMenu(GameManager &manager);

    void Initialize(SDL_Renderer *renderer) override;
};
