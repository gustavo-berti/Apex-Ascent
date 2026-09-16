#include "DeckBuilder.hpp"
#include <algorithm>
#include <random>

namespace {

std::vector<int> CollectAllIds(const CardDatabase &database) {
    std::vector<int> ids;
    ids.reserve(database.GetAllCreatures().size() + database.GetAllSpells().size());

    for (const auto &[id, data] : database.GetAllCreatures())
        ids.push_back(id);
    for (const auto &[id, data] : database.GetAllSpells())
        ids.push_back(id);

    return ids;
}

} // namespace

std::vector<std::string> DeckBuilder::Build(const std::unordered_map<int, int> &selection,
                                            const CardDatabase &database) {
    std::unordered_map<int, int> used;
    std::vector<std::string> deck;
    deck.reserve(kDeckSize);

    // 1) A escolha do jogador prevalece, respeitando os limites de copia e de tamanho.
    for (const auto &[id, requestedCopies] : selection) {
        if (static_cast<int>(deck.size()) >= kDeckSize) break;
        if (requestedCopies <= 0) continue;
        if (!database.GetCreature(id) && !database.GetSpell(id)) continue; // id invalido/removido

        const int copies = std::min(requestedCopies, kMaxCopies);
        const int allowed = std::min(copies, kDeckSize - static_cast<int>(deck.size()));

        for (int i = 0; i < allowed; ++i)
            deck.push_back(std::to_string(id));

        used[id] = allowed;
    }

    // 2) O que faltar (parcial ou totalmente) e preenchido aleatoriamente.
    const std::vector<int> pool = CollectAllIds(database);

    if (!pool.empty()) {
        std::random_device rd;
        std::mt19937 rng(rd());

        // Guarda contra loop infinito quando a colecao e pequena demais para
        // fechar kDeckSize cartas mesmo usando as kMaxCopies copias de tudo.
        const int maxAttempts = static_cast<int>(pool.size()) * kMaxCopies * 4 + 16;
        int attempts = 0;

        while (static_cast<int>(deck.size()) < kDeckSize && attempts++ < maxAttempts) {
            const int id = pool[rng() % pool.size()];
            if (used[id] >= kMaxCopies) continue;

            deck.push_back(std::to_string(id));
            used[id]++;
        }
    }

    return deck;
}
