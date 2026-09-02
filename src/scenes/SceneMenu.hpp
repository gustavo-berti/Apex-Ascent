#pragma once
#include "../core/GameWorld.hpp"
#include "../objects/cards/types/CardTypes.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

class GameManager;

struct MenuButton {
    SDL_Rect rect;
    std::string label;
};

// O menu tem duas telas: a principal e a escolha do oponente, que aparece
// depois do "Começar Jogo".
enum class MenuScreen { MAIN, OPPONENT_SELECT };

class SceneMenu : public GameWorld {
  private:
    GameManager &gameManager;
    SDL_Texture *background = nullptr;
    TTF_Font *font = nullptr;
    TTF_Font *fontTitle = nullptr;

    MenuScreen screen = MenuScreen::MAIN;
    std::vector<MenuButton> buttons;      // tela principal
    std::vector<MenuButton> setupButtons; // escolha do oponente
    int hoveredIndex = -1;

    // Layout de setupButtons: [0..4] raças, [5..9] níveis, depois Lutar e Voltar.
    static constexpr int kRaceCount = 5;
    static constexpr int kLevelCount = 5;
    static constexpr int kIdxFight = kRaceCount + kLevelCount;
    static constexpr int kIdxBack = kIdxFight + 1;

    // Race::NONE fica de fora: e o baralho de teste, nao uma raça jogavel.
    static constexpr Race kRaces[kRaceCount] = {Race::HUMAN, Race::AUTOMAT, Race::PIXIE,
                                                Race::DRAGON, Race::DRYAD};

    // Viram deckType e deckPart do Opponent.
    Race selectedRace = Race::PIXIE;
    int selectedLevel = 1;

    bool IsButtonClicked(const MenuButton &btn, const SDL_Event &event) const;
    const std::vector<MenuButton> &ActiveButtons() const;

    void BuildOpponentSetupButtons(int w, int h);
    void HandleMainInput(const SDL_Event &event);
    void HandleSetupInput(const SDL_Event &event);
    void StartOpponentBattle(); // destroi a cena: nada pode rodar depois
    void RenderOpponentSetup(SDL_Renderer *renderer, int w);

  public:
    SceneMenu(GameManager &manager);
    ~SceneMenu() override;

    void Initialize(SDL_Renderer *renderer) override;
    void HandleInput(SDL_Event &event) override;
    void Update(float dt) override;
    void Render(SDL_Renderer *renderer) override;
};
