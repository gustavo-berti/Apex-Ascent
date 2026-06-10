#pragma once
#include "../../core/GameObject.hpp"

class StaticObject : public GameObject {
  protected:
    int x, y;
    int width, height;

  public:
    StaticObject(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
    virtual ~StaticObject() {}

    virtual void Initialize() override = 0;

    virtual void Update(float dt) override {}

    virtual void Render(SDL_Renderer *renderer) override = 0;
    int GetX() const { return x; }
    int GetY() const { return y; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
};