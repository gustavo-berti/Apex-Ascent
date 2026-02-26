#pragma once
#include "./base/DynamicObject.hpp"
#include <string>

enum class CardType {
    CREATURE,
    SPELL
};

class Card : public DynamicObject {
    private:
        std::string name;
        CardType type;
        int manaCost;
    public:
        Card(std::string name, CardType type, int manaCost, int x, int y);
        virtual ~Card();

        virtual void Initialize() override;
        virtual void Update(float dt) override;
        virtual void Render(SDL_Renderer* renderer) override;
};