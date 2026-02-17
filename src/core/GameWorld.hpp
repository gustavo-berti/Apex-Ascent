#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include "GameObject.hpp"

class GameWorld {
    private:
        std::vector<GameObject*> objects; 
    public:
        GameWorld();
        ~GameWorld();
        void AddObject(GameObject* obj);
        void Update(float dt);
        void Render(SDL_Renderer* renderer);
};