#pragma once
#include "./base/DynamicObject.hpp"
#include "./cards/CardTypes.hpp"
#include <string>
#include <vector>

class Card : public DynamicObject {
    private:
        std::string name;
        int manaCost;
        std::string description;
        Rarity rarity;
        std::vector<Ability> abilities;
        Rarity rarity;
        std::string imagePath;
    public:
        Card(std::string name, int manaCost, Rarity rarity, std::string imagePath, int x, int y);
        virtual ~Card();

        virtual void Initialize() override;
        virtual void Update(float dt) override;
        virtual void Render(SDL_Renderer* renderer) override;

        bool hasAbility(Ability ability) const {
            for (const auto& a : abilities) {
                if (a == ability) {
                    return true;
                }
            }
            return false;
        }
};