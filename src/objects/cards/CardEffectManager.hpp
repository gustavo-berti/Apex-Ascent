#pragma once
#include "../scenes/SceneBattle.hpp"
#include "Card.hpp"
#include "CreatureCard.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

struct EffectContext {
    Card *source = nullptr;
    CreatureCard *target = nullptr;

    SceneBattle *scene = nullptr;

    std::vector<CreatureCard *> *allies = nullptr;
    std::vector<CreatureCard *> *enemies = nullptr;

    int kills_count = 0;
};

class EffectManager {
  public:
    using EffectFn = std::function<void(EffectData &, EffectContext &)>;

    EffectManager();

    void execute(EffectData &effect, EffectContext &ctx);
    void processPassives(std::vector<CreatureCard *> &field, SceneBattle *scene);

  private:
    std::unordered_map<EffectAction, EffectFn> dispatch_;

    void registerAll();
};