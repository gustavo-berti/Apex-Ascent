#pragma once

#include "../core/data/CardDatabase.hpp"

class CreatureCard;

class CardFactory {
  public:
    static CreatureCard *CreateCreatureCard(const CardDatabase &database, int creatureId,
                                            const std::string &imageBasePath = "assets/images/",
                                            int x = 0, int y = 0, int xpPoints = 0);
};
