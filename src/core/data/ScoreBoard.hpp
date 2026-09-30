#pragma once

#include "../../objects/cards/types/CardTypes.hpp"
#include <string>
#include <vector>

// Uma partida encerrada: o nome escolhido no menu mais como a run terminou.
// Placar antigo (gravado antes do nome existir) carrega com o nome vazio.
struct ScoreEntry {
    std::string name;
    int score = 0;
    Race opponentRace = Race::NONE;
    int difficulty = 1; // nivel do oponente escolhido no menu (1 a 5)
    int health = 0;     // vida que sobrou para o jogador
    int cardsLeft = 0;  // cartas que ainda estavam com o jogador
};

// Top 10 do jogo, salvo em assets/data/scores.json. O arquivo e progresso
// local: ele fica fora do git e e criado na primeira partida.
class ScoreBoard {
  private:
    std::vector<ScoreEntry> entries; // sempre ordenada, maior pontuacao primeiro

  public:
    static constexpr int kMaxEntries = 10;
    static constexpr const char *kDefaultPath = "assets/data/scores.json";

    // (cartas restantes + vida) * dificuldade
    static int ComputeScore(int cardsLeft, int health, int difficulty);

    // Arquivo ausente nao e erro: o placar so ainda esta vazio.
    bool Load(const std::string &filepath = kDefaultPath);
    bool Save(const std::string &filepath = kDefaultPath) const;

    // Insere na ordem e corta no top 10. Devolve a posicao (0 = melhor) ou -1
    // se a pontuacao nao entrou na lista.
    int Add(const ScoreEntry &entry);

    const std::vector<ScoreEntry> &GetEntries() const { return entries; }
};
