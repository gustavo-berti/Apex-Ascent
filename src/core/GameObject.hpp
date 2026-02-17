#pragma once
#include <SDL2/SDL.h>

class GameObject {
    public:
        virtual ~GameObject() {}
        virtual void Initialize() = 0;
        virtual void Update(float dt) = 0;
        virtual void Render(SDL_Renderer* renderer) = 0;
};