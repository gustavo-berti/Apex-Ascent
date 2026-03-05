#include "GameWorld.hpp"
#include "../objects/Card.hpp"
#include "../objects/CreatureCard.hpp"
#include "../objects/SpellCard.hpp"

GameWorld::GameWorld() {
    Card* carta1 = new CreatureCard("Guerreiro", 5, Rarity::COMMON, "", 3, 5, "", 0, 590, 65);
    Card* carta2 = new CreatureCard("Guerreiro", 5, Rarity::COMMON, "", 3, 5, "", 0, 590, 215);
    Card* carta3 = new CreatureCard("Guerreiro", 5, Rarity::COMMON, "", 5, 5, "", 0, 590, 365);
    Card* carta4 = new CreatureCard("Guerreiro", 5, Rarity::COMMON, "", 5, 5, "", 0, 590, 515);
    Card* carta5 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 100, 650);
    Card* carta6 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 200, 650);
    Card* carta7 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 300, 650);
    Card* carta8 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 400, 650);
    Card* carta9 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 500, 650);
    Card* carta10 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 600, 650);
    Card* carta11 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 700, 650);
    Card* carta12 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 800, 650);
    Card* carta13 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 900, 650);
    Card* carta14 = new SpellCard("Bola de Fogo", 5, Rarity::COMMON, "", "Cause 10 damage to the target enemy.", SpellType::FAST, 1000, 650);

    AddObject(carta1);
    AddObject(carta2);
    AddObject(carta3);
    AddObject(carta4);
    AddObject(carta5);
    AddObject(carta6);
    AddObject(carta7);
    AddObject(carta8);
    AddObject(carta9);
    AddObject(carta10);
    AddObject(carta11);
    AddObject(carta12);
    AddObject(carta13);
    AddObject(carta14);
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