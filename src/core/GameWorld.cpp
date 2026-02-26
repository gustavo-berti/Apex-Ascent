#include "GameWorld.hpp"
#include "../objects/Card.hpp"

GameWorld::GameWorld() {
    Card* carta1 = new Card("Guerreiro", CardType::CREATURE, 3, 100, 200);
    Card* carta2 = new Card("Bola de Fogo", CardType::SPELL, 5, 250, 200);

    AddObject(carta1);
    AddObject(carta2);
}

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