#pragma once
#include <string>

#include "../../objects/cards/types/CardEffectTypes.hpp"
#include "../../objects/cards/types/CardTypes.hpp"

EffectTrigger StringToEffectTrigger(const std::string &trigger);
EffectAction StringToEffectAction(const std::string &action);
EffectTarget StringToEffectTarget(const std::string &target);
EffectCondition StringToEffectCondition(const std::string &condition);
Race StringToRace(const std::string &raceStr);
Rarity StringToRarity(const std::string &rarityType);
CardType StringToCardType(const std::string &cardType);
SpellSpeed StringToSpellType(const std::string &spellType);
Ability StringToAbility(const std::string &abilityStr);