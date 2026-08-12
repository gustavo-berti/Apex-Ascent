#include "Opponent.hpp"
#include "../objects/cards/Card.hpp"
#include "../objects/cards/CreatureCard.hpp"

std::vector<Card *> Opponent::ChooseCreaturesToPlay(const std::vector<Card *> &hand,
                                                     int freeFieldSlots) const {
    std::vector<Card *> chosen;
    int manaLeft = mana.current;

    for (Card *card : hand) {
        if (static_cast<int>(chosen.size()) >= freeFieldSlots) break;
        if (!dynamic_cast<CreatureCard *>(card)) continue;
        if (card->GetManaCost() > manaLeft) continue;

        chosen.push_back(card);
        manaLeft -= card->GetManaCost();
    }

    return chosen;
}

bool Opponent::ShouldAttack(int enemyFieldCount, int playerFieldCount) const {
    return enemyFieldCount > playerFieldCount && enemyFieldCount > 0;
}
