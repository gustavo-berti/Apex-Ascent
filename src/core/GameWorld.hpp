#pragma once
#include "GameObject.hpp"
#include <SDL2/SDL.h>
#include <vector>

class GameWorld {
  protected:
    std::vector<GameObject *> objects;

  public:
    virtual ~GameWorld() {
        for (auto obj : objects) {
            delete obj;
        }
        objects.clear();
    }

    virtual void Initialize() = 0;
    virtual void HandleInput(SDL_Event &event) = 0;
    virtual void Update(float dt) = 0;
    virtual void Render(SDL_Renderer *renderer) = 0;
};