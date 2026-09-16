#pragma once

#include "../core/data/CardDatabase.hpp"
#include <string>
#include <unordered_map>
#include <vector>

// ── DeckBuilder ──────────────────────────────────────────────────
// Converte a selecao feita pelo jogador na tela de Colecao (id da carta ->
// quantidade de copias) num deck valido de exatamente kDeckSize cartas.
//
// Regras:
//  - No maximo kMaxCopies copias da mesma carta.
//  - O que o jogador escolheu sempre prevalece (ate os limites acima).
//  - Os slots que sobrarem (parcial ou totalmente) sao preenchidos com
//    cartas aleatorias da colecao inteira, respeitando o limite de copias.
class DeckBuilder {
  public:
    static constexpr int kDeckSize = 30;
    static constexpr int kMaxCopies = 3;

    // selection: id da carta -> quantidade escolhida (pode vir com valores
    // fora do range 0..kMaxCopies; sao normalizados aqui).
    // Retorna a lista de ids em formato string, no mesmo formato usado por
    // Entity::masterDeck.
    static std::vector<std::string> Build(const std::unordered_map<int, int> &selection,
                                          const CardDatabase &database);
};
