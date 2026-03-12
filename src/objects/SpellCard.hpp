#include "./Card.hpp"

enum class SpellType {
    FAST,
    SLOW
};

class SpellCard : public Card {
    private:
        std::string effectDescription;
        SpellType spellType;
    public:
        SpellCard(std::string name, int manaCost, Rarity rarity, std::string imagePath, std::string effectDescription, SpellType spellType, int x, int y);
        virtual ~SpellCard();

        virtual void Initialize() override;
        virtual void Update(float dt) override;
        virtual void Render(SDL_Renderer* renderer) override;
        void ActivateEffect();
        bool isFast() const;
};