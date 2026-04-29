#pragma once
#include <functional>
#include <string>

enum class TurnOwner { PLAYER, OPPONENT };

enum class BattlePhase {
    TURN_START,  // Automático: restaura/incrementa mana, compra carta
    MAIN,        // Fase Principal: baixar criaturas e magias
    COMBAT,      // Fase de Combate: atacar com criaturas
    SECOND_MAIN, // Fase Secundária: igual à principal, sem compra
};

enum class CombatStep {
    NONE,
    DECLARE_ATTACKERS, // Selecionar criaturas atacantes (ou passar)
    ATTACK_MAGIC,      // Magia do atacante; pode cancelar o ataque aqui
    DECLARE_DEFENDERS, // Oponente defende (automático por enquanto)
    RESOLUTION,        // Cálculo do combate (implementar depois)
};

class TurnManager {
  private:
    TurnOwner currentOwner;
    BattlePhase currentPhase;
    CombatStep combatStep;
    TurnOwner firstOwner;
    int playerTurnCount = 0;
    int opponentTurnCount = 0;

    std::function<void(TurnOwner, BattlePhase)> onPhaseChanged;
    std::function<void(TurnOwner)> onTurnChanged;
    std::function<void(CombatStep)> onCombatStepChanged;

    void SetPhase(BattlePhase phase) {
        currentPhase = phase;

        if (phase == BattlePhase::TURN_START) {
            if (currentOwner == TurnOwner::PLAYER)
                ++playerTurnCount;
            else
                ++opponentTurnCount;
        }
        if (onPhaseChanged) onPhaseChanged(currentOwner, currentPhase);
    }

    void SetCombatStep(CombatStep step) {
        combatStep = step;
        if (onCombatStepChanged) onCombatStepChanged(combatStep);
    }

  public:
    TurnManager()
        : currentOwner(TurnOwner::PLAYER), currentPhase(BattlePhase::TURN_START),
          combatStep(CombatStep::NONE), firstOwner(TurnOwner::PLAYER) {}

    // ── Inicialização ─────────────────────────────────────────────
    void RollForFirstTurn() {
        firstOwner = (rand() % 2 == 0) ? TurnOwner::PLAYER : TurnOwner::OPPONENT;
        currentOwner = firstOwner;
        currentPhase = BattlePhase::TURN_START;
        combatStep = CombatStep::NONE;
        playerTurnCount = (firstOwner == TurnOwner::PLAYER) ? 1 : 0;
        opponentTurnCount = (firstOwner == TurnOwner::OPPONENT) ? 1 : 0;
    }

    // ── Callbacks ─────────────────────────────────────────────────
    void SetOnPhaseChanged(std::function<void(TurnOwner, BattlePhase)> cb) { onPhaseChanged = cb; }
    void SetOnTurnChanged(std::function<void(TurnOwner)> cb) { onTurnChanged = cb; }
    void SetOnCombatStepChanged(std::function<void(CombatStep)> cb) { onCombatStepChanged = cb; }

    // ── Avançar fase ─────────────────────────────────────────────
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
            SetPhase(BattlePhase::SECOND_MAIN);
            SetCombatStep(CombatStep::NONE);
            return true;

        case BattlePhase::SECOND_MAIN:
            currentOwner =
                (currentOwner == TurnOwner::PLAYER) ? TurnOwner::OPPONENT : TurnOwner::PLAYER;
            SetPhase(BattlePhase::TURN_START); // já incrementa o contador
            SetCombatStep(CombatStep::NONE);
            if (onTurnChanged) onTurnChanged(currentOwner);
            return false;
        }
        return true;
    }

    // ── Avançar sub-passo de combate ──────────────────────────────
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
            return false;
        default:
            return false;
        }
    }

    // ── Consulta ─────────────────────────────────────────────────
    TurnOwner GetOwner() const { return currentOwner; }
    BattlePhase GetPhase() const { return currentPhase; }
    CombatStep GetCombatStep() const { return combatStep; }
    TurnOwner GetFirstOwner() const { return firstOwner; }
    bool IsPlayerTurn() const { return currentOwner == TurnOwner::PLAYER; }
    int GetPlayerTurnCount() const { return playerTurnCount; }
    int GetOpponentTurnCount() const { return opponentTurnCount; }

    bool IsFirstTurnOf(TurnOwner owner) const {
        return owner == TurnOwner::PLAYER ? playerTurnCount == 1 : opponentTurnCount == 1;
    }

    bool ShouldGainManaThisTurn() const {
        if (currentOwner == firstOwner && IsFirstTurnOf(currentOwner)) return false;
        return true;
    }

    bool ShouldDrawThisTurn() const { return ShouldGainManaThisTurn(); }

    bool CanAttackThisTurn() const {
        return currentOwner == TurnOwner::PLAYER ? playerTurnCount >= 2 : opponentTurnCount >= 2;
    }

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