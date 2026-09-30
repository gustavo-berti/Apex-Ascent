#pragma once

#include <string>
#include <vector>

// Uma pessoa na tela de creditos. Tudo vem de assets/data/credits.json: trocar
// nome, frase ou foto nao exige recompilar o jogo.
struct CreditPerson {
    std::string name;
    std::string role;  // linha pequena embaixo do nome; vazia = nao aparece
    std::string quote; // frase de efeito
    std::string photo; // caminho da imagem; vazio = quadro com a inicial do nome
};

// Os creditos divididos em duas secoes: quem fez o jogo e as mençoes honrosas.
class Credits {
  private:
    std::vector<CreditPerson> creators;
    std::vector<CreditPerson> mentions;

  public:
    static constexpr const char *kDefaultPath = "assets/data/credits.json";

    // O arquivo aceita comentarios (// ...) pra ficar autoexplicativo.
    bool Load(const std::string &filepath = kDefaultPath);

    const std::vector<CreditPerson> &GetCreators() const { return creators; }
    const std::vector<CreditPerson> &GetMentions() const { return mentions; }
    bool IsEmpty() const { return creators.empty() && mentions.empty(); }
};
