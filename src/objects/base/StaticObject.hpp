#pragma once
#include "../../core/GameObject.hpp"
#include <my-lib/math-vector.h>

class StaticObject : public GameObject {
  protected:
    Mylib::Math::Vector2f pos;
    int width, height;

  public:
    StaticObject(float x, float y, int w, int h) : pos(x, y), width(w), height(h) {}
    virtual ~StaticObject() {}

    virtual void Initialize() override = 0;

    virtual void Update(float dt) override {}

    virtual void Render(SDL_Renderer *renderer) override = 0;
    int GetX() const { return static_cast<int>(pos.x); }
    int GetY() const { return static_cast<int>(pos.y); }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    const Mylib::Math::Vector2f &GetPosition() const { return pos; }
};