#pragma once
#include "../../core/GameObject.hpp"

class StaticObject : public GameObject {
    protected:
        int x, y;
        int width, height;
    public:
        StaticObject(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
        virtual ~StaticObject() {}

        virtual void Initialize() = 0;
        virtual void Render(SDL_Renderer* renderer) = 0;
};