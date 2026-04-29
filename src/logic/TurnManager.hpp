#pragma once
#include <functional>
#include <string>

enum class TurnOwner { PLAYER, OPPONENT };

enum class BattlePhase {
    TURN_START,
    MAIN,
    COMBAT,
    SECOND_MAIN,
};

enum class CombatStep {
    NONE,
    DECLARE_ATTACKERS,
    ATTACK_MAGIC,
    DECLARE_DEFENDERS,
    RESOLUTION,
};

class TurnManager {
  private:
    TurnOwner currentOwner;
    BattlePhase currentPhase;
    CombatStep combatStep;

    std::function<void(TurnOwner, BattlePhase)> onPhaseChanged;
    std::function<void(TurnOwner)> onTurnChanged;
    std::function<void(CombatStep)> onCombatStepChanged;

    void SetPhase(BattlePhase phase) {
        currentPhase = phase;
        if (onPhaseChanged) onPhaseChanged(currentOwner, currentPhase);
    }

    void SetCombatStep(CombatStep step) {
        combatStep = step;
        if (onCombatStepChanged) onCombatStepChanged(combatStep);
    }

  public:
    TurnManager()
        : currentOwner(TurnOwner::PLAYER), currentPhase(BattlePhase::TURN_START),
          combatStep(CombatStep::NONE) {}

    // ── Inicialização ─────────────────────────────────────────────
    void RollForFirstTurn() {
        currentOwner = (rand() % 2 == 0) ? TurnOwner::PLAYER : TurnOwner::OPPONENT;
        currentPhase = BattlePhase::TURN_START;
        combatStep = CombatStep::NONE;
    }

    void SetOnPhaseChanged(std::function<void(TurnOwner, BattlePhase)> cb) { onPhaseChanged = cb; }
    void SetOnTurnChanged(std::function<void(TurnOwner)> cb) { onTurnChanged = cb; }
    void SetOnCombatStepChanged(std::function<void(CombatStep)> cb) { onCombatStepChanged = cb; }

    // ── Avançar fase (botão "Passar Fase / Turno") ────────────────
    // Retorna false quando o turno passou para o outro lado.
    bool AdvancePhase() {
        switch (currentPhase) {

        case BattlePhase::TURN_START:
            SetPhase(BattlePhase::MAIN);
            return true;

        case BattlePhase::MAIN:
            SetPhase(BattlePhase::COMBAT);
            SetCombatStep(CombatStep::DECLARE_ATTACKERS);
            return true;

        case BattlePhase::COMBAT:
            // A fase de combate avança internamente via AdvanceCombatStep().
            // AdvancePhase() só é chamado aqui quando o combate já terminou
            // (CombatStep::NONE) — o jogador escolheu não atacar ou a resolução acabou.
            SetPhase(BattlePhase::SECOND_MAIN);
            SetCombatStep(CombatStep::NONE);
            return true;

        case BattlePhase::SECOND_MAIN:
            currentOwner =
                (currentOwner == TurnOwner::PLAYER) ? TurnOwner::OPPONENT : TurnOwner::PLAYER;
            SetPhase(BattlePhase::TURN_START);
            SetCombatStep(CombatStep::NONE);
            if (onTurnChanged) onTurnChanged(currentOwner);
            return false;
        }
        return true;
    }

    // ── Avançar sub-passo de combate ──────────────────────────────
    // Retorna false quando o combate terminou — SceneBattle deve chamar AdvancePhase().
    bool AdvanceCombatStep() {
        switch (combatStep) {
        case CombatStep::DECLARE_ATTACKERS:
            SetCombatStep(CombatStep::ATTACK_MAGIC);
            return true;

        case CombatStep::ATTACK_MAGIC:
            SetCombatStep(CombatStep::DECLARE_DEFENDERS);
            return true;

        case CombatStep::DECLARE_DEFENDERS:
            SetCombatStep(CombatStep::RESOLUTION);
            return true;

        case CombatStep::RESOLUTION:
            SetCombatStep(CombatStep::NONE);
            return false; // combate encerrado

        default:
            return false;
        }
    }

    // ── Consulta ─────────────────────────────────────────────────
    TurnOwner GetOwner() const { return currentOwner; }
    BattlePhase GetPhase() const { return currentPhase; }
    CombatStep GetCombatStep() const { return combatStep; }
    bool IsPlayerTurn() const { return currentOwner == TurnOwner::PLAYER; }

    std::string GetOwnerName() const {
        return currentOwner == TurnOwner::PLAYER ? "Jogador" : "Oponente";
    }

    std::string GetPhaseName() const {
        switch (currentPhase) {
        case BattlePhase::TURN_START:
            return "Inicio de Turno";
        case BattlePhase::MAIN:
            return "Fase Principal";
        case BattlePhase::COMBAT:
            return "Fase de Combate";
        case BattlePhase::SECOND_MAIN:
            return "Fase Secundaria";
        }
        return "";
    }

    std::string GetCombatStepName() const {
        switch (combatStep) {
        case CombatStep::NONE:
            return "";
        case CombatStep::DECLARE_ATTACKERS:
            return "Declarar Atacantes";
        case CombatStep::ATTACK_MAGIC:
            return "Magias do Atacante";
        case CombatStep::DECLARE_DEFENDERS:
            return "Declarar Defensores";
        case CombatStep::RESOLUTION:
            return "Resolucao";
        }
        return "";
    }
};