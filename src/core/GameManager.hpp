#pragma once
#include <SDL2/SDL.h>
#include "GameWorld.hpp"

class GameManager {
private:
    bool isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;
    GameWorld* currentWorld;
public:
    GameManager();
    ~GameManager();
    static bool IsPointInsideRect(int x, int y, const SDL_Rect& rect);
    bool Initialize(const char* title, int x, int y, int width, int height, bool fullscreen);
    void Run();
    void HandleEvents();
    void Update();
    void Render();
    void Clean();
    void ToggleFullscreen();
    bool Running() { return isRunning; }
};