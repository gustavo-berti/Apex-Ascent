#pragma once
#include "StaticObject.hpp"

class DynamicObject : public StaticObject {
  protected:
    bool isHovered;

  public:
    DynamicObject(int x, int y, int w, int h) : StaticObject(x, y, w, h), isHovered(false) {}

    virtual ~DynamicObject() {}

    virtual void Initialize() override = 0;

    virtual void Update(float dt) override = 0;

    virtual void Render(SDL_Renderer *renderer) override = 0;

    bool IsHovered() const { return isHovered; }
    void SetHovered(bool hovered) { isHovered = hovered; }
};