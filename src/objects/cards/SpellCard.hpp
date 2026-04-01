#include "../../logic/CardDatabase.hpp"
#include "./Card.hpp"

enum class SpellType { FAST, SLOW };

class SpellCard : public Card {
  private:
    const SpellData *dataRef;

    SpellType spellType;

  public:
    SpellCard(const SpellData *data, int x, int y);
    virtual ~SpellCard();

    virtual void Initialize() override;
    virtual void Update(float dt) override;
    virtual void Render(SDL_Renderer *renderer) override;
    void ActivateEffect();
    bool isFast() const;
};