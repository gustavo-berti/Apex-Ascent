#include "CardEffectManager.hpp"

EffectManager::EffectManager() { registerAll(); }

void EffectManager::execute(EffectData &effect, EffectContext &ctx) {
    if (effect.params.max_activations != -1 &&
        effect.activations_used >= effect.params.max_activations)
        return;

    auto it = dispatch_.find(effect.action);
    if (it != dispatch_.end()) {
        it->second(effect, ctx);
        effect.activations_used++;
    }
}

void EffectManager::registerAll() {}