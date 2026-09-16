#pragma once
#include "../core/data/ScoreBoard.hpp"
#include "../objects/cards/types/CardTypes.hpp"
#include "../objects/ui/UITextField.hpp"
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

    static constexpr int kNameMaxLength = 12;

    // Viram deckType e deckPart do Opponent.
    Race selectedRace = Race::PIXIE;
    int selectedLevel = 1;

    // Nome do jogador: vai para o Player e de la para o placar. Sobrevive a um
    // "Voltar" para o menu principal, entao quem joga de novo nao redigita.
    ui::UITextField nameField;
    bool nameMissing = false; // "Lutar" clicado com o campo vazio

    Screen screen = Screen::MAIN;
    ScoreBoard scores; // recarregado a cada vez que o placar abre

    void ShowMainScreen();
    void ShowOpponentSetup(); // botoes: [0..4] raças, [5..9] niveis, Lutar, Voltar
    void ShowScores();
    void SelectRace(Race race);
    void SelectLevel(int level);
    void StartOpponentBattle(); // destroi a cena: nada pode rodar depois
    void RenderScoreTable(SDL_Renderer *renderer) const;

    // O SDL_TEXTINPUT so chega enquanto a tela do oponente esta aberta.
    void SetTextInputActive(bool active);

  protected:
    void RenderContent(SDL_Renderer *renderer) override;

  public:
    explicit SceneMenu(GameManager &manager);
    ~SceneMenu() override;

    void Initialize(SDL_Renderer *renderer) override;
    void HandleInput(SDL_Event &event) override;
};
