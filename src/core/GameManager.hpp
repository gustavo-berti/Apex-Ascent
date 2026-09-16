#pragma once
#include "../logic/Opponent.hpp"
#include "../logic/Player.hpp"
#include "GameWorld.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

class GameManager {
  private:
    bool isRunning;
    Player player;
    Opponent opponent;
    SDL_Window *window;
    SDL_Renderer *renderer;
    GameWorld *currentWorld;
    Mix_Music* backgroundMusic;
    static std::unordered_map<std::string, Mix_Chunk*> soundEffects;

    // Selecao de deck feita pelo jogador na tela de Colecao (id da carta ->
    // quantidade de copias, 0..3). Vive so durante a sessao atual: nao e
    // salva em disco. DeckBuilder::Build usa isso pra montar o deck real
    // quando a partida comeca.
    std::unordered_map<int, int> playerDeckSelection;

  public:
    GameManager();
    ~GameManager();
    static bool IsPointInsideRect(int x, int y, const SDL_Rect &rect);
    bool Initialize(const char *title, int x, int y, int width, int height, bool fullscreen);
    void ChangeScene(GameWorld *newWorld);
    void Run();
    void HandleEvents();
    void Update();
    void Render();
    void Clean();
    void ToggleFullscreen();
    bool Running() { return isRunning; }
    SDL_Renderer *GetRenderer() const { return renderer; }
    Player &GetPlayer() { return player; }
    Opponent &GetOpponent() { return opponent; }
    void SetOpponentDeck(Race type, int part) { opponent.SetDeck(type, part); }
    std::unordered_map<int, int> &GetPlayerDeckSelection() { return playerDeckSelection; }
    void ChangeMusic(const std::string &filePath);
    static void LoadSFX(const std::string& name, const std::string& filepath);
    static void PlaySFX(const std::string& name);
    static void ClearSFX();
};