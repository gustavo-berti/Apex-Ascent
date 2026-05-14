#pragma once
#include "../logic/Opponent.hpp"
#include "../logic/Player.hpp"
#include "GameWorld.hpp"
#include <SDL2/SDL.h>

class GameManager {
  private:
    bool isRunning;
    Player player;
    Opponent opponent;
    SDL_Window *window;
    SDL_Renderer *renderer;
    GameWorld *currentWorld;

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
};