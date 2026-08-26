#pragma once
#include <vector>

class Card;

// ── Resultado de uma resolucao de combate ──────────────────────────
struct CombatResult {
    int damageDealt = 0; // dano que passou direto para a entidade defensora
    bool opponentDied = false;
    std::vector<Card *> deadPlayerCards; // criaturas do jogador destruidas
    std::vector<Card *> deadEnemyCards;  // criaturas do oponente destruidas
};

// ── Bloqueio declarado ─────────────────────────────────────────────
// Quem defende escolhe qual atacante cada criatura vai defender (1 para 1:
// cada defensor bloqueia um atacante e cada atacante recebe no maximo um
// defensor).
struct DefenderAssignment {
    Card *defender = nullptr;
    Card *attacker = nullptr;
};
