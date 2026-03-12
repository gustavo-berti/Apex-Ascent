#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include "GameObject.hpp"
class GameWorld {
protected:
    std::vector<GameObject*> objects;

public:
    virtual ~GameWorld() {
        for (auto obj : objects) {
            delete obj;
        }
        objects.clear();
    }

    virtual void Initialize() = 0;
    virtual void HandleInput(SDL_Event& event) = 0;
    virtual void Update(float dt) = 0;
    virtual void Render(SDL_Renderer* renderer) = 0;
};