#include "CardFactory.hpp"
#include "../objects/CreatureCard.hpp"
#include <iostream>

CreatureCard* CardFactory::CreateCreatureCard(const CardDatabase& database, int creatureId,
                                              const std::string& imageBasePath, int x, int y,
                                              int xpPoints) {
    const CreatureData* cardData = database.GetCreature(creatureId);
    if (!cardData || cardData->stages.empty()) {
        return nullptr;
    }

    std::cout << "Carta encontrada: " << cardData->name << std::endl;

    return new CreatureCard(
        cardData->name,
        cardData->manaCost,
        cardData->rarity,
        imageBasePath + cardData->name + ".png",
        cardData->stages[0].attack,
        cardData->stages[0].health,
        cardData->description,
        xpPoints,
        x,
        y
    );
}
