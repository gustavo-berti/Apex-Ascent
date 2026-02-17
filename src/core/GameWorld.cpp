#include "GameWorld.hpp"

GameWorld::GameWorld() {}

GameWorld::~GameWorld() {
    for (auto obj : objects) {
        delete obj;
    }
    objects.clear();
}

void GameWorld::AddObject(GameObject* obj) {
    objects.push_back(obj);
    obj->Initialize();
}

void GameWorld::Update(float dt) {
    for (auto obj : objects) {
        obj->Update(dt);
    }
}

void GameWorld::Render(SDL_Renderer* renderer) {
    for (auto obj : objects) {
        obj->Render(renderer);
    }
}