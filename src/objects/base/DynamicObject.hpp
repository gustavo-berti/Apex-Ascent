#pragma once
#include "../../core/GameObject.hpp"

class DynamicObject : public GameObject {
    protected:
        int x, y;
        int width, height;
        bool isHovered;
    public:
        DynamicObject(int x, int y, int w, int h) : x(x), y(y), width(w), height(h), isHovered(false) {}
        virtual ~DynamicObject() {}

        virtual void Initialize() = 0;
        virtual void Update(float dt) = 0;
        virtual void Render(SDL_Renderer* renderer) = 0;

        int GetX() const { return x; }
        int GetY() const { return y; }
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
};