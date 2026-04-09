#pragma once
#include <functional>
#include <string>

enum class TurnOwner { PLAYER, OPPONENT };

enum class BattlePhase {
    PREPARE, // Fase 1: Posicionar criaturas na zona de preparação
    BATTLE,  // Fase 2: Mover criaturas para a zona de ataque
    END      // Fase 3: Resolver combate e limpar
};

class TurnManager {
  private:
    TurnOwner   currentOwner;
    BattlePhase currentPhase;
    int         phaseIndex; // 0, 1, 2 → mapeado para as 3 fases

    std::function<void(TurnOwner, BattlePhase)> onPhaseChanged;
    std::function<void(TurnOwner)>              onTurnChanged;

    static BattlePhase PhaseFromIndex(int index) {
        switch (index) {
            case 0:  return BattlePhase::PREPARE;
            case 1:  return BattlePhase::BATTLE;
            default: return BattlePhase::END;
        }
    }

  public:
    TurnManager() : currentOwner(TurnOwner::PLAYER), currentPhase(BattlePhase::PREPARE), phaseIndex(0) {}

    // ── Inicialização ──────────────────────────────────────────────
    void RollForFirstTurn() {
        currentOwner = (rand() % 2 == 0) ? TurnOwner::PLAYER : TurnOwner::OPPONENT;
        currentPhase = BattlePhase::PREPARE;
        phaseIndex   = 0;
    }

    // ── Callbacks ──────────────────────────────────────────────────
    void SetOnPhaseChanged(std::function<void(TurnOwner, BattlePhase)> cb) { onPhaseChanged = cb; }
    void SetOnTurnChanged (std::function<void(TurnOwner)> cb)              { onTurnChanged  = cb; }

    // ── Avançar ────────────────────────────────────────────────────
    // Retorna true se apenas a fase avançou; false se o turno passou para o oponente.
    bool AdvancePhase() {
        phaseIndex++;

        if (phaseIndex > 2) {
            // Turno completo → passa para o outro lado
            phaseIndex   = 0;
            currentPhase = BattlePhase::PREPARE;
            currentOwner = (currentOwner == TurnOwner::PLAYER) ? TurnOwner::OPPONENT : TurnOwner::PLAYER;

            if (onTurnChanged)  onTurnChanged(currentOwner);
            if (onPhaseChanged) onPhaseChanged(currentOwner, currentPhase);
            return false;
        }

        currentPhase = PhaseFromIndex(phaseIndex);
        if (onPhaseChanged) onPhaseChanged(currentOwner, currentPhase);
        return true;
    }

    // ── Consulta ───────────────────────────────────────────────────
    TurnOwner   GetOwner()      const { return currentOwner; }
    BattlePhase GetPhase()      const { return currentPhase; }
    bool        IsPlayerTurn()  const { return currentOwner == TurnOwner::PLAYER; }

    std::string GetOwnerName()  const { return currentOwner == TurnOwner::PLAYER ? "Jogador" : "Oponente"; }

    std::string GetPhaseName()  const {
        switch (currentPhase) {
            case BattlePhase::PREPARE: return "Preparacao";
            case BattlePhase::BATTLE:  return "Batalha";
            case BattlePhase::END:     return "Fim";
        }
        return "";
    }
};