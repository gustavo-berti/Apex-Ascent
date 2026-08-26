#include "Opponent.hpp"
#include "../objects/cards/Card.hpp"
#include "../objects/cards/CreatureCard.hpp"
#include <algorithm>

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

std::vector<DefenderAssignment>
Opponent::ChooseDefenders(const std::vector<Card *> &attackers,
                          const std::vector<Card *> &availableDefenders) const {
    std::vector<DefenderAssignment> plan;

    // Ameacas em ordem decrescente de ataque: o mais perigoso escolhe primeiro
    std::vector<CreatureCard *> threats;
    for (Card *card : attackers)
        if (auto *creature = dynamic_cast<CreatureCard *>(card)) threats.push_back(creature);

    std::sort(threats.begin(), threats.end(),
              [](const CreatureCard *a, const CreatureCard *b) {
                  if (a->GetAttack() != b->GetAttack()) return a->GetAttack() > b->GetAttack();
                  return a->GetHealth() > b->GetHealth();
              });

    std::vector<CreatureCard *> freeDefenders;
    for (Card *card : availableDefenders)
        if (auto *creature = dynamic_cast<CreatureCard *>(card)) freeDefenders.push_back(creature);

    int incomingDamage = 0;
    for (const CreatureCard *threat : threats)
        incomingDamage += threat->GetAttack();

    for (CreatureCard *threat : threats) {
        if (freeDefenders.empty()) break;

        // So vale morrer sem matar se o dano que ainda passaria for letal
        const bool lethalIncoming = incomingDamage >= currentHealth;

        auto best = freeDefenders.end();
        int bestScore = 0;
        int bestValue = 0;

        for (auto it = freeDefenders.begin(); it != freeDefenders.end(); ++it) {
            CreatureCard *defender = *it;

            const bool kills = defender->GetAttack() >= threat->GetHealth();
            const bool survives = threat->GetAttack() < defender->GetHealth();
            const int defenderValue = defender->GetAttack() + defender->GetHealth();
            const int threatValue = threat->GetAttack() + threat->GetHealth();

            int score = 0;
            if (kills && survives)
                score = 100; // mata sem morrer
            else if (kills)
                score = (threatValue >= defenderValue) ? 70 : 30; // troca (boa ou ruim)
            else if (survives)
                score = 50; // segura o dano e sobrevive
            else if (lethalIncoming)
                score = 10; // bloqueio-sacrificio para nao morrer

            if (score == 0) continue;

            // Empate: guarda a criatura mais valiosa para outro bloqueio
            if (score > bestScore || (score == bestScore && defenderValue < bestValue)) {
                best = it;
                bestScore = score;
                bestValue = defenderValue;
            }
        }

        if (best == freeDefenders.end()) continue;

        plan.push_back({*best, threat});
        incomingDamage -= threat->GetAttack();
        freeDefenders.erase(best);
    }

    return plan;
}
