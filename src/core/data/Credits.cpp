#include "Credits.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace {

// Uma secao do arquivo. Campo ausente vira string vazia: um credito so com o
// nome preenchido continua valendo.
std::vector<CreditPerson> ReadSection(const json &j, const char *key) {
    std::vector<CreditPerson> people;
    if (!j.contains(key) || !j[key].is_array()) return people;

    for (const auto &item : j[key]) {
        if (!item.is_object()) continue;

        CreditPerson person;
        person.name = item.value("nome", std::string());
        person.role = item.value("papel", std::string());
        person.quote = item.value("frase", std::string());
        person.photo = item.value("foto", std::string());

        if (person.name.empty()) continue; // entrada sem nome nao tem o que mostrar
        people.push_back(person);
    }

    return people;
}

} // namespace

bool Credits::Load(const std::string &filepath) {
    creators.clear();
    mentions.clear();

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[CREDITOS] Nao encontrei " << filepath << std::endl;
        return false;
    }

    // ignore_comments = true: o arquivo explica cada campo em // comentarios.
    const json j = json::parse(file, nullptr, false, true);
    if (j.is_discarded() || !j.is_object()) {
        std::cerr << "[CREDITOS] " << filepath << " invalido (erro de JSON)." << std::endl;
        return false;
    }

    creators = ReadSection(j, "criadores");
    mentions = ReadSection(j, "mencoes_honrosas");

    return !IsEmpty();
}
