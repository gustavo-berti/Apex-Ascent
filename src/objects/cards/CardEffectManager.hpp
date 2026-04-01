#pragma once
#include "types/CardEffectTypes.hpp"
#include <functional>
#include <unordered_map>

struct EffectContext {
    class Card *source;
    class Card *enemy;
    class SceneBattle *scene;
    int kills_count;
};

class EffectManager {
  public:
    using EffectFn = std::function<void(EffectData &, EffectContext &)>;

    EffectManager();

    void execute(EffectData &effect, EffectContext &ctx);
    void processPassives(std::vector<Card *> &field, SceneBattle *scene);

  private:
    std::unordered_map<EffectAction, EffectFn> dispatch_;

    void registerAll();
};