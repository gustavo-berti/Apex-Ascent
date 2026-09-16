#include "ScoreBoard.hpp"
#include "../enums/EnumConverter.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

int ScoreBoard::ComputeScore(int cardsLeft, int health, int difficulty) {
    return (std::max(0, cardsLeft) + std::max(0, health)) * std::max(1, difficulty);
}

bool ScoreBoard::Load(const std::string &filepath) {
    entries.clear();

    std::ifstream file(filepath);
    if (!file.is_open()) return false; // primeira execucao: ainda nao existe placar

    // parse tolerante: um arquivo truncado ou editado a mao zera o placar em vez
    // de derrubar o jogo.
    const json j = json::parse(file, nullptr, false);
    if (j.is_discarded() || !j.contains("scores") || !j["scores"].is_array()) {
        std::cerr << "[PLACAR] " << filepath << " invalido. Começando um placar novo." << std::endl;
        return false;
    }

    for (const auto &item : j["scores"]) {
        ScoreEntry entry;
        entry.score = item.value("score", 0);
        entry.opponentRace = StringToRace(item.value("race", std::string("NONE")));
        entry.difficulty = item.value("difficulty", 1);
        entry.health = item.value("health", 0);
        entry.cardsLeft = item.value("cardsLeft", 0);
        entries.push_back(entry);
    }

    std::stable_sort(entries.begin(), entries.end(),
                     [](const ScoreEntry &a, const ScoreEntry &b) { return a.score > b.score; });
    if (static_cast<int>(entries.size()) > kMaxEntries) entries.resize(kMaxEntries);

    return true;
}

bool ScoreBoard::Save(const std::string &filepath) const {
    json j;
    j["scores"] = json::array();

    for (const ScoreEntry &entry : entries) {
        j["scores"].push_back({{"score", entry.score},
                               {"race", RaceToString(entry.opponentRace)},
                               {"difficulty", entry.difficulty},
                               {"health", entry.health},
                               {"cardsLeft", entry.cardsLeft}});
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[PLACAR] Falha ao salvar " << filepath << std::endl;
        return false;
    }

    file << j.dump(2) << std::endl;
    return true;
}

int ScoreBoard::Add(const ScoreEntry &entry) {
    // Empate mantem quem chegou antes: o novo entra depois dos iguais.
    const auto at = std::upper_bound(
        entries.begin(), entries.end(), entry,
        [](const ScoreEntry &a, const ScoreEntry &b) { return a.score > b.score; });

    const int position = static_cast<int>(at - entries.begin());
    if (position >= kMaxEntries) return -1;

    entries.insert(at, entry);
    if (static_cast<int>(entries.size()) > kMaxEntries) entries.resize(kMaxEntries);

    return position;
}
