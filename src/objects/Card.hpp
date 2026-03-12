#pragma once
#include "./base/DynamicObject.hpp"
#include <string>

enum class Rarity {
    COMMON,
    UNCOMMON,
    RARE,
    EPIC,
    LEGENDARY
};

class Card : public DynamicObject {
    private:
        std::string name;
        std::string description;
        int manaCost;
        Rarity rarity;
        std::string imagePath;
    public:
        Card(std::string name, int manaCost, Rarity rarity, std::string imagePath, int x, int y);
        virtual ~Card();

        virtual void Initialize() override;
        virtual void Update(float dt) override;
        virtual void Render(SDL_Renderer* renderer) override;

        void SetPosition(int newX, int newY) {
            x = newX;
            y = newY;
        }
};