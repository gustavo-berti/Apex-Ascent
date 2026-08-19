#include "SceneBattle.hpp"
#include "../core/GameManager.hpp"
#include "../logic/CardFactory.hpp"
#include "../objects/cards/SpellCard.hpp"
#include "../objects/ui/UIRenderUtils.hpp"
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <iostream>
#include <random>

// ═══════════════════════════════════════════════════════════════════
//  Construtor / Destrutor
// ═══════════════════════════════════════════════════════════════════

SceneBattle::SceneBattle() : board(turnManager), draggedCard(nullptr) {}

SceneBattle::~SceneBattle() {
    if (background) {
        SDL_DestroyTexture(background);
        background = nullptr;
    }
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (fontSmall) {
        TTF_CloseFont(fontSmall);
        fontSmall = nullptr;
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Initialize
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::Initialize(SDL_Renderer *renderer) {
    std::cout << "Inicializando SceneBattle..." << std::endl;
    SDL_Surface *surface = IMG_Load("assets/images/arena.png");
    if (!surface) {
        std::cerr << "Erro ao carregar fundo: " << IMG_GetError() << std::endl;
    } else {
        background = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    if (!cardDatabase.LoadFromJson("assets/data/cards.json"))
        std::cerr << "Falha ao carregar cards.json" << std::endl;

    font = ui::UIRenderUtils::LoadFont("./assets/fonts/arial.ttf", 72);
    fontSmall = ui::UIRenderUtils::LoadFont("./assets/fonts/arial.ttf", 28);

    // O campo tem exatamente a largura das 6 cartas que cabem nele (Board::kZoneWidth).
    // As quatro zonas usam o mesmo kZoneGap entre si; na horizontal o bloco fica
    // centralizado na tela e na vertical ele desce ate encostar na mao (kHandGap),
    // que e o limite inferior real do tabuleiro.
    constexpr int kScreenW = 1600;
    constexpr int kScreenH = 900;
    constexpr int kZoneGap = 8;
    constexpr int kHandGap = 8; // respiro entre a preparacao do jogador e a mao
    // A mao usa a mesma altura das zonas (carta + respiro), o mais justo possivel,
    // para o tabuleiro poder descer o maximo.
    constexpr int kHandH = Board::kZoneHeight;

    constexpr int boardWidth = Board::kZoneWidth;
    constexpr int zoneH = Board::kZoneHeight;
    constexpr int boardX = (kScreenW - boardWidth) / 2;

    constexpr int handY = kScreenH - kHandH;
    constexpr int stackH = 4 * zoneH + 3 * kZoneGap;
    constexpr int topY = handY - kHandGap - stackH;
    static_assert(topY >= 0, "o tabuleiro nao cabe acima da mao");

    enemyPreparationZone = {boardX, topY, boardWidth, zoneH};
    enemyBattleZone = {boardX, enemyPreparationZone.y + zoneH + kZoneGap, boardWidth, zoneH};
    playerBattleZone = {boardX, enemyBattleZone.y + zoneH + kZoneGap, boardWidth, zoneH};
    playerPreparationZone = {boardX, playerBattleZone.y + zoneH + kZoneGap, boardWidth, zoneH};
    playerHandZone = {0, handY, kScreenW, kHandH};

    btnCancel = {1420, 300, 150, 50};
    btnNextPhase = {1420, 360, 150, 50};
    btnAttack = {1420, 420, 150, 50};

    board.SetZoneRects(playerPreparationZone, playerBattleZone, enemyPreparationZone,
                       enemyBattleZone);

    turnManager.SetOnPhaseChanged([this](TurnOwner o, BattlePhase p) { OnPhaseChanged(o, p); });
    turnManager.SetOnTurnChanged([this](TurnOwner o) { OnTurnChanged(o); });
    turnManager.SetOnCombatStepChanged([this](CombatStep s) { OnCombatStepChanged(s); });
}

// ═══════════════════════════════════════════════════════════════════
//  StartBattle
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::StartBattle(Player *playerState, Opponent *opp, SDL_Renderer *sdlRenderer) {
    if (!SetCurrentPlayerState(playerState)) return;
    if (!opp) return;

    opponent = opp;
    this->renderer = sdlRenderer;
    outcome = BattleOutcome::ONGOING;
    summonPending.Clear();
    opponent->SetDeck(Race::PIXIE, 1);
    ResetBattleState();

    matchStartPending = true;
}

void SceneBattle::StartMatchFlow() {
    BuildDrawPile(currentState);
    BuildDrawPile(opponent);

    ShuffleDrawPile(currentState);
    ShuffleDrawPile(opponent);

    dealCount = 0;
    scriptedState = ScriptedState::DealPlayerHand;
}

// ═══════════════════════════════════════════════════════════════════
//  Callbacks do TurnManager
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::OnPhaseChanged(TurnOwner owner, BattlePhase phase) {
    std::cout << "[" << turnManager.GetOwnerName() << "] " << turnManager.GetPhaseName()
              << std::endl;
    if (phase == BattlePhase::TURN_START && owner == TurnOwner::PLAYER) HandleTurnStart();
}

void SceneBattle::OnTurnChanged(TurnOwner owner) {
    std::cout << "=== TURNO: " << turnManager.GetOwnerName() << " ===" << std::endl;
    if (owner == TurnOwner::OPPONENT) RunOpponentTurn();
}

void SceneBattle::OnCombatStepChanged(CombatStep step) {
    std::cout << "[COMBATE] " << turnManager.GetCombatStepName() << std::endl;

    if (step == CombatStep::DECLARE_DEFENDERS) {
        BeginDefenderDeclaration();
        return;
    }

    if (step == CombatStep::RESOLUTION) {
        const bool opponentWasAttacking = !turnManager.IsPlayerTurn();
        int &healthTarget =
            turnManager.IsPlayerTurn() ? opponent->currentHealth : currentState->currentHealth;
        CombatResult result = board.ResolveCombat(healthTarget, cardObjects);
        pendingDefender = nullptr;
        SendDeadCardsToDiscard(result);

        if (result.damageDealt > 0) {
            std::string targetName = turnManager.IsPlayerTurn() ? "Oponente" : "Jogador";
            std::cout << "[BATALHA] " << targetName << " recebeu " << result.damageDealt
                      << " de dano!" << std::endl;
        }
        CheckBattleOutcome(result);

        if (!IsBattleOver()) {
            turnManager.AdvancePhase(); // COMBATE → Fase Secundaria
            // A IA nao tem input: precisa de um passo pausado para encerrar o turno
            if (opponentWasAttacking) scriptedState = ScriptedState::AIEndTurn;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Vitória / derrota
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::CheckBattleOutcome(const CombatResult &result) {
    (void)result;

    if (opponent->IsDefeated()) {
        outcome = BattleOutcome::PLAYER_WIN;
        std::cout << "=== JOGADOR VENCEU! ===" << std::endl;
        return;
    }

    if (currentState->IsDefeated()) {
        outcome = BattleOutcome::PLAYER_LOSE;
        std::cout << "=== JOGADOR PERDEU! ===" << std::endl;
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Lógica de turno
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleTurnStart() {
    bool gainMana = turnManager.ShouldGainManaThisTurn();
    currentState->mana.OnTurnStart(gainMana);
    std::cout << "[INICIO DE TURNO] Mana: " << currentState->mana.current << "/"
              << currentState->mana.total << (gainMana ? "" : " (1o turno)") << std::endl;

    if (turnManager.ShouldDrawThisTurn()) {
        DrawCards(currentState, currentState->GetTurnStartDrawCount());
    }

    turnManager.AdvancePhase();
}

void SceneBattle::RunOpponentTurn() {
    std::cout << "\n===================================" << std::endl;
    std::cout << "[OPONENTE] Iniciando turno da IA..." << std::endl;
    scriptedState = ScriptedState::AITurnStart;
}

void SceneBattle::RunAITurnStart() {
    bool gainMana = turnManager.ShouldGainManaThisTurn();
    opponent->mana.OnTurnStart(gainMana);
    std::cout << "[IA] Mana atual: " << opponent->mana.current << "/" << opponent->mana.total
              << std::endl;

    if (turnManager.ShouldDrawThisTurn()) {
        DrawCards(opponent, opponent->GetTurnStartDrawCount());
    }

    turnManager.AdvancePhase();
    scriptedState = ScriptedState::AIEvaluateHand;
}

void SceneBattle::RunAIEvaluateHand() {
    auto &hand = opponentPiles.hand;
    const int freeSlots = board.GetFreePreparationSlots(TurnOwner::OPPONENT);

    std::cout << "[IA] Avaliando " << hand.size() << " carta(s) na mao (Ordem FIFO)..."
              << std::endl;

    aiCardsToPlay = opponent->ChooseCreaturesToPlay(hand, freeSlots);
    aiCardsToPlayIndex = 0;

    if (aiCardsToPlay.empty()) {
        turnManager.AdvancePhase();
        scriptedState = ScriptedState::AIDecideAttack;
    } else {
        scriptedState = ScriptedState::AISummoning;
    }
}

void SceneBattle::RunAISummonStep() {
    Card *card = aiCardsToPlay[aiCardsToPlayIndex++];

    // Mesmo caminho do jogador: o Board valida o limite, organiza e toca o som
    if (board.AddToEnemyPreparation(card)) {
        SpendMana(opponent, card->GetManaCost(), card->GetName());

        auto &hand = opponentPiles.hand;
        hand.erase(std::remove(hand.begin(), hand.end(), card), hand.end());

        std::cout << "[IA] >>> INVOCOU: " << card->GetName() << " (Visível no campo!)" << std::endl;
    }

    if (aiCardsToPlayIndex >= aiCardsToPlay.size()) {
        aiCardsToPlay.clear();
        turnManager.AdvancePhase();
        scriptedState = ScriptedState::AIDecideAttack;
    }
}

void SceneBattle::RunAIDecideAttack() {
    const int enemyCount = board.GetFieldCount(TurnOwner::OPPONENT);
    const int playerCount = board.GetFieldCount(TurnOwner::PLAYER);

    // Mesma regra do jogador: ninguem ataca no proprio primeiro turno
    const bool canAttack = turnManager.CanAttackThisTurn();
    if (!canAttack)
        std::cout << "[IA] Primeiro turno: nao pode atacar." << std::endl;

    // O Board declara os atacantes pelas mesmas regras usadas pelo jogador
    if (canAttack && opponent->ShouldAttack(enemyCount, playerCount) &&
        board.DeclareAllAttackers() > 0) {
        std::cout << "[IA] Vantagem numerica. ATACANDO com " << board.GetAttackers().size()
                  << " criatura(s)!" << std::endl;

        scriptedState = ScriptedState::AIConfirmAttack;
        turnManager.AdvanceCombatStep(); // Declarar Atacantes → Magias do Atacante
    } else {
        std::cout << "[IA] Sem vantagem (" << enemyCount << " vs " << playerCount
                  << "). Abortou o ataque." << std::endl;
        std::cout << "===================================\n" << std::endl;
        scriptedState = ScriptedState::Idle;
        turnManager.AdvancePhase();
        turnManager.AdvancePhase();
    }
}

void SceneBattle::RunAIConfirmAttack() {
    std::cout << "[IA] Confirmou o ataque com " << board.GetAttackers().size() << " criatura(s)."
              << std::endl;
    scriptedState = ScriptedState::Idle;
    turnManager.AdvanceCombatStep(); // Magias do Atacante → Declarar Defensores (jogador defende)
}

// ── Defesa da IA (quando o jogador ataca) ─────────────────────────

void SceneBattle::RunAIDeclareDefenders() {
    aiDefensePlan = opponent->ChooseDefenders(board.GetAttackers(), board.GetAvailableDefenders());
    aiDefensePlanIndex = 0;

    if (aiDefensePlan.empty()) {
        std::cout << "[IA] Nenhum bloqueio compensa. Passou a defesa." << std::endl;
        scriptedState = ScriptedState::Idle;
        turnManager.AdvanceCombatStep(); // → Resolucao
        return;
    }

    std::cout << "[IA] Vai defender com " << aiDefensePlan.size() << " criatura(s)." << std::endl;
    scriptedState = ScriptedState::AIDefendStep;
}

void SceneBattle::RunAIDefendStep() {
    const DefenderAssignment &assignment = aiDefensePlan[aiDefensePlanIndex++];
    board.AssignDefender(assignment.defender, assignment.attacker);

    if (aiDefensePlanIndex >= aiDefensePlan.size()) {
        aiDefensePlan.clear();
        aiDefensePlanIndex = 0;
        scriptedState = ScriptedState::AIConfirmDefense;
    }
}

void SceneBattle::RunAIConfirmDefense() {
    scriptedState = ScriptedState::Idle;
    turnManager.AdvanceCombatStep(); // Declarar Defensores → Resolucao
}

void SceneBattle::RunAIEndTurn() {
    std::cout << "===================================\n" << std::endl;
    scriptedState = ScriptedState::Idle;
    turnManager.AdvancePhase(); // Fase Secundaria → turno do jogador
}

void SceneBattle::AdvanceScriptedState() {
    switch (scriptedState) {
    case ScriptedState::DealPlayerHand:
        DrawCards(currentState, 1);
        if (++dealCount >= 5) {
            dealCount = 0;
            scriptedState = ScriptedState::DealOpponentHand;
        }
        break;

    case ScriptedState::DealOpponentHand:
        DrawCards(opponent, 1);
        if (++dealCount >= 5) {
            dealCount = 0;
            scriptedState = ScriptedState::Idle;

            srand(static_cast<unsigned>(SDL_GetTicks()));
            turnManager.RollForFirstTurn();

            if (turnManager.IsPlayerTurn())
                HandleTurnStart();
            else
                RunOpponentTurn();
        }
        break;

    case ScriptedState::AITurnStart:
        RunAITurnStart();
        break;

    case ScriptedState::AIEvaluateHand:
        RunAIEvaluateHand();
        break;

    case ScriptedState::AISummoning:
        RunAISummonStep();
        break;

    case ScriptedState::AIDecideAttack:
        RunAIDecideAttack();
        break;

    case ScriptedState::AIConfirmAttack:
        RunAIConfirmAttack();
        break;

    case ScriptedState::AIDeclareDefenders:
        RunAIDeclareDefenders();
        break;

    case ScriptedState::AIDefendStep:
        RunAIDefendStep();
        break;

    case ScriptedState::AIConfirmDefense:
        RunAIConfirmDefense();
        break;

    case ScriptedState::AIEndTurn:
        RunAIEndTurn();
        break;

    case ScriptedState::Idle:
        break;
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Mana
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::SpendMana(Entity *entity, int cost, const std::string &cardName) {
    if (!entity->mana.CanAfford(cost)) {
        std::cout << "[MANA] Insuficiente para " << cardName << " (custo: " << cost
                  << ", disponivel: " << entity->mana.current << ")" << std::endl;
        return false;
    }
    entity->mana.Spend(cost);
    std::cout << "[MANA] Gastou " << cost << " em " << cardName << " (" << entity->mana.current
              << "/" << entity->mana.total << ")" << std::endl;
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  Invocação e sacrifício
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::TrySummonCard(Card *card, std::vector<Card *>::reverse_iterator handIt) {
    if (!board.IsPreparationFull(TurnOwner::PLAYER)) {
        if (!SpendMana(currentState, card->GetManaCost(), card->GetName())) return;

        if (board.AddToPlayerPreparation(card, cardObjects)) {
            playerPiles.hand.erase(std::next(handIt).base());
            RearrangeHand();
        } else {
            currentState->mana.current += card->GetManaCost();
        }
        return;
    }

    std::cout << "[INVOCACAO] Campo cheio. Selecione uma carta para sacrificar." << std::endl;
    summonPending.Begin(card);
}

void SceneBattle::ConfirmSummon() {
    if (!summonPending.active || !summonPending.cardToSummon) {
        CancelSummon();
        return;
    }

    if (!summonPending.cardToSacrifice) {
        std::cout << "[INVOCACAO] Selecione uma carta para sacrificar primeiro." << std::endl;
        return;
    }

    Card *toSummon = summonPending.cardToSummon;
    Card *toSacrifice = summonPending.cardToSacrifice;

    if (!SpendMana(currentState, toSummon->GetManaCost(), toSummon->GetName())) {
        CancelSummon();
        return;
    }

    if (board.RemoveFromPreparation(toSacrifice, TurnOwner::PLAYER)) {
        toSacrifice->SetPosition(-200, -200);
        playerPiles.discardPile.push_back(toSacrifice);
        GameManager::PlaySFX("card_death");
        std::cout << "[SACRIFICIO] " << toSacrifice->GetName() << " foi enviada ao cemiterio."
                  << std::endl;
    }

    auto handIt = std::find(playerPiles.hand.begin(), playerPiles.hand.end(), toSummon);
    if (handIt != playerPiles.hand.end()) playerPiles.hand.erase(handIt);

    board.AddToPlayerPreparation(toSummon, cardObjects);
    RearrangeHand();
    summonPending.Clear();
    std::cout << "[INVOCACAO] " << toSummon->GetName() << " invocada com sucesso." << std::endl;
}

void SceneBattle::CancelSummon() {
    if (!summonPending.active) return;
    std::cout << "[INVOCACAO] Invocacao cancelada. Carta permanece na mao." << std::endl;
    summonPending.Clear();
}

void SceneBattle::RearrangeHand() {
    const int n = static_cast<int>(playerPiles.hand.size());
    const int totalW = n * Board::kCardWidth + (n - 1) * Board::kCardGap;
    const int startX = playerHandZone.x + (playerHandZone.w - totalW) / 2;
    const int y = playerHandZone.y + (playerHandZone.h - Board::kCardHeight) / 2;
    for (int i = 0; i < n; ++i)
        playerPiles.hand[i]->SetPosition(startX + i * (Board::kCardWidth + Board::kCardGap), y);
}

// ═══════════════════════════════════════════════════════════════════
//  Helpers de estado
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::CanPlayCreature() const {
    if (!turnManager.IsPlayerTurn()) return false;
    auto p = turnManager.GetPhase();
    return p == BattlePhase::MAIN || p == BattlePhase::SECOND_MAIN;
}

bool SceneBattle::CanPlaySpell() const {
    if (!turnManager.IsPlayerTurn()) return false;
    auto p = turnManager.GetPhase();
    auto s = turnManager.GetCombatStep();
    return p == BattlePhase::MAIN || p == BattlePhase::SECOND_MAIN ||
           s == CombatStep::ATTACK_MAGIC || s == CombatStep::DECLARE_DEFENDERS;
}

// ═══════════════════════════════════════════════════════════════════
//  Combate
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleAttackButton() {
    if (!board.CanDeclareAttack()) return;
    turnManager.AdvanceCombatStep();
}

void SceneBattle::HandleCancelAttack() {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return;
    board.ReturnAllAttackersToPreparation(cardObjects);
    std::cout << "[COMBATE] Ataque cancelado." << std::endl;
}

void SceneBattle::HandleConfirmAttack() {
    if (!board.ConfirmAttack()) {
        turnManager.AdvancePhase();
        return;
    }
    turnManager.AdvanceCombatStep();
}

// ═══════════════════════════════════════════════════════════════════
//  Defesa
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::IsPlayerDeclaringDefenders() const {
    return !turnManager.IsPlayerTurn() && turnManager.GetPhase() == BattlePhase::COMBAT &&
           turnManager.GetCombatStep() == CombatStep::DECLARE_DEFENDERS;
}

void SceneBattle::BeginDefenderDeclaration() {
    pendingDefender = nullptr;
    board.ClearDefenders();

    if (board.GetAttackers().empty()) {
        std::cout << "[DEFESA] Nenhum atacante declarado." << std::endl;
        turnManager.AdvanceCombatStep();
        return;
    }

    if (board.GetAvailableDefenders().empty()) {
        std::cout << "[DEFESA] Lado defensor sem criaturas para defender." << std::endl;
        if (turnManager.IsPlayerTurn())
            scriptedState = ScriptedState::AIConfirmDefense; // pausa pro render antes do dano
        else
            turnManager.AdvanceCombatStep();
        return;
    }

    if (turnManager.IsPlayerTurn()) {
        // O jogador atacou: a IA escolhe os defensores dela
        scriptedState = ScriptedState::AIDeclareDefenders;
        return;
    }

    // A IA atacou: o jogador escolhe quem defende e contra quem
    std::cout << "[DEFESA] Clique numa criatura da preparacao e depois no atacante que ela "
                 "vai defender. Confirme quando terminar."
              << std::endl;
}

void SceneBattle::ConfirmDefense() {
    pendingDefender = nullptr;
    std::cout << "[DEFESA] Defesa confirmada com " << board.GetDefenderAssignments().size()
              << " bloqueio(s)." << std::endl;
    turnManager.AdvanceCombatStep(); // → Resolucao
}

void SceneBattle::ClearDefense() {
    pendingDefender = nullptr;
    board.ClearDefenders();
    std::cout << "[DEFESA] Bloqueios limpos." << std::endl;
}

void SceneBattle::SendDeadCardsToDiscard(const CombatResult &result) {
    auto bury = [](std::vector<Card *> &discardPile, const std::vector<Card *> &deadCards,
                   const char *ownerName) {
        for (Card *card : deadCards) {
            if (!card) continue;
            card->SetPosition(-200, -200);
            discardPile.push_back(card);
            std::cout << "[COMBATE] " << card->GetName() << " (" << ownerName
                      << ") foi destruida e enviada ao cemiterio." << std::endl;
        }
    };

    bury(playerPiles.discardPile, result.deadPlayerCards, "jogador");
    bury(opponentPiles.discardPile, result.deadEnemyCards, "oponente");

    if (!result.deadPlayerCards.empty() || !result.deadEnemyCards.empty())
        GameManager::PlaySFX("card_death");
}

// ═══════════════════════════════════════════════════════════════════
//  HandleInput
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::HandleInput(SDL_Event &event) {
    if (IsBattleOver()) return;
    if (event.type != SDL_MOUSEBUTTONDOWN) return;
    if (summonPending.active) {
        HandleSummonPendingInput(event);
        return;
    }

    if (IsPlayerDeclaringDefenders()) {
        HandleDefenseInput(event);
        return;
    }

    if (HandleCancelClick(event)) return;
    if (HandleAttackClick(event)) return;
    if (HandleNextPhaseClick(event)) return;
    if (!turnManager.IsPlayerTurn()) return;
    if (HandleBattleCardClick(event)) return;
    HandleHandCardClick(event);
}

// ── Input da declaração de defensores ─────────────────────────────

bool SceneBattle::HandleDefenseInput(const SDL_Event &e) {
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;

    auto hits = [&](const Card *card) {
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        return GameManager::IsPointInsideRect(e.button.x, e.button.y, r);
    };

    if (GameManager::IsPointInsideRect(e.button.x, e.button.y, btnNextPhase)) {
        ConfirmDefense();
        return true;
    }

    if (GameManager::IsPointInsideRect(e.button.x, e.button.y, btnCancel)) {
        ClearDefense();
        return true;
    }

    // Criatura da preparacao: escolhe (ou desmarca) quem vai defender
    for (Card *card : board.GetPlayerPreparationCards()) {
        if (!card || !hits(card)) continue;
        if (!dynamic_cast<CreatureCard *>(card)) return true;

        if (pendingDefender == card) {
            pendingDefender = nullptr;
            std::cout << "[DEFESA] Selecao removida." << std::endl;
        } else {
            pendingDefender = card;
            std::cout << "[DEFESA] " << card->GetName()
                      << " selecionada. Clique no atacante que ela vai defender." << std::endl;
        }
        return true;
    }

    // Defensor ja declarado: clicar nele desfaz o bloqueio
    for (Card *card : board.GetPlayerBattleCards()) {
        if (!card || !hits(card)) continue;
        if (board.UnassignDefender(card) && pendingDefender == card) pendingDefender = nullptr;
        return true;
    }

    // Atacante inimigo: recebe o defensor selecionado (ou libera o bloqueio dele)
    for (Card *card : board.GetEnemyBattleCards()) {
        if (!card || !hits(card)) continue;

        if (pendingDefender) {
            if (board.AssignDefender(pendingDefender, card)) pendingDefender = nullptr;
            return true;
        }

        if (Card *blocker = board.GetDefenderOf(card)) board.UnassignDefender(blocker);
        return true;
    }

    return false;
}

// ── Input do modo sacrifício ──────────────────────────────────────

bool SceneBattle::HandleSummonPendingInput(const SDL_Event &e) {
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;

    if (GameManager::IsPointInsideRect(e.button.x, e.button.y, btnNextPhase)) {
        ConfirmSummon();
        return true;
    }

    if (GameManager::IsPointInsideRect(e.button.x, e.button.y, btnCancel)) {
        CancelSummon();
        return true;
    }

    for (auto *card : board.GetPlayerPreparationCards()) {
        if (!card) continue;
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;
        if (summonPending.cardToSacrifice == card) {
            summonPending.cardToSacrifice = nullptr;
            std::cout << "[SACRIFICIO] Selecao removida." << std::endl;
        } else {
            summonPending.cardToSacrifice = card;
            std::cout << "[SACRIFICIO] " << card->GetName() << " selecionada." << std::endl;
        }
        return true;
    }

    return false;
}

bool SceneBattle::HandleNextPhaseClick(const SDL_Event &e) {
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;
    if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, btnNextPhase)) return false;
    if (!turnManager.IsPlayerTurn()) return true;

    auto phase = turnManager.GetPhase();
    auto step = turnManager.GetCombatStep();

    if (phase == BattlePhase::COMBAT && step == CombatStep::ATTACK_MAGIC) {
        HandleConfirmAttack();
        return true;
    }
    if (phase == BattlePhase::COMBAT && step == CombatStep::DECLARE_ATTACKERS) {
        turnManager.AdvancePhase();
        return true;
    }
    turnManager.AdvancePhase();
    return true;
}

bool SceneBattle::HandleAttackClick(const SDL_Event &e) {
    if (!board.ShouldShowAttackButton()) return false;
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;
    if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, btnAttack)) return false;
    HandleAttackButton();
    return true;
}

bool SceneBattle::HandleCancelClick(const SDL_Event &e) {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return false;
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;
    if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, btnCancel)) return false;
    HandleCancelAttack();
    return true;
}

bool SceneBattle::HandleHandCardClick(const SDL_Event &e) {
    for (auto it = playerPiles.hand.rbegin(); it != playerPiles.hand.rend(); ++it) {
        Card *card = *it;
        if (!card) continue;

        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;

        bool isCreature = dynamic_cast<CreatureCard *>(card) != nullptr;

        if (isCreature && CanPlayCreature()) {
            TrySummonCard(card, it);
        } else if (!isCreature && CanPlaySpell()) {
            if (!SpendMana(currentState, card->GetManaCost(), card->GetName())) return true;
            std::cout << "[MAGIA] " << card->GetName() << " jogada (efeito pendente)." << std::endl;
        } else {
            std::cout << "[AVISO] Nao e possivel jogar essa carta agora." << std::endl;
        }
        return true;
    }
    return false;
}

bool SceneBattle::HandleBattleCardClick(const SDL_Event &e) {
    if (turnManager.GetCombatStep() != CombatStep::ATTACK_MAGIC) return false;

    for (auto *card : board.GetPlayerBattleCards()) {
        if (!card) continue;
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;
        board.ToggleAttackerSelection(card, cardObjects);
        return true;
    }
    for (auto *card : board.GetPlayerPreparationCards()) {
        if (!card) continue;
        SDL_Rect r = {card->GetX(), card->GetY(), card->GetWidth(), card->GetHeight()};
        if (!GameManager::IsPointInsideRect(e.button.x, e.button.y, r)) continue;
        board.ToggleAttackerSelection(card, cardObjects);
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════
//  Update / Render
// ═══════════════════════════════════════════════════════════════════

void SceneBattle::Update(float dt) {
    if (matchStartPending && hasRendered) {
        matchStartPending = false;
        StartMatchFlow();
    }

    if (scriptedState != ScriptedState::Idle) {
        scriptedTimer += dt;
        if (scriptedTimer >= kScriptedDelay) {
            scriptedTimer = 0.f;
            AdvanceScriptedState();
        }
    }

    for (auto obj : objects)
        obj->Update(dt);
}

void SceneBattle::Render(SDL_Renderer *renderer) {
    hasRendered = true;

    // O fundo vem primeiro: qualquer coisa desenhada antes dele seria apagada.
    int w, h;
    SDL_RenderGetLogicalSize(renderer, &w, &h);
    if (w == 0 || h == 0) SDL_GetRendererOutputSize(renderer, &w, &h);

    if (background) {
        SDL_Rect dst = {0, 0, w, h};
        SDL_RenderCopy(renderer, background, nullptr, &dst);
    } else {
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_RenderClear(renderer);
    }

    board.Render(renderer);
    RenderDefensePhase(renderer);
    RenderHand(renderer);
    RenderButtons(renderer);
    RenderMana(renderer);
    RenderHealthBars(renderer);
    RenderHUD(renderer);
    if (summonPending.active) RenderSummonPending(renderer);
    if (IsBattleOver()) RenderOutcome(renderer);
}

void SceneBattle::RenderSummonPending(SDL_Renderer *renderer) const {
    if (!summonPending.active || !summonPending.cardToSummon) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
    SDL_Rect full = {0, 0, 1600, 900};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    {
        Card *c = summonPending.cardToSummon;
        int origX = c->GetX(), origY = c->GetY();
        c->SetPosition(660, 380);
        c->Render(renderer);
        c->SetPosition(origX, origY);
    }

    if (fontSmall) {
        ui::UIRenderUtils::RenderText(renderer, "Campo cheio!", 800, 300,
                                      SDL_Color{255, 80, 80, 255}, fontSmall);
        ui::UIRenderUtils::RenderText(renderer, "Escolha uma carta para enviar", 800, 380,
                                      SDL_Color{255, 220, 80, 255}, fontSmall);
        ui::UIRenderUtils::RenderText(renderer, "ao cemiterio", 800, 425,
                                      SDL_Color{255, 220, 80, 255}, fontSmall);

        if (summonPending.cardToSacrifice) {
            std::string confirmMsg = "Sacrificar " + summonPending.cardToSacrifice->GetName() + "?";
            ui::UIRenderUtils::RenderText(renderer, confirmMsg, 800, 535,
                                          SDL_Color{255, 80, 80, 255}, fontSmall);
        }
    }

    for (const Card *card : board.GetPlayerPreparationCards()) {
        if (!card) continue;

        bool isSelected = (summonPending.cardToSacrifice == card);
        SDL_Rect highlight = {card->GetX() - 3, card->GetY() - 3, card->GetWidth() + 6,
                              card->GetHeight() + 6};

        if (isSelected) {
            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255); // vermelho = selecionada
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255); // amarelo = disponível
        }
        SDL_RenderDrawRect(renderer, &highlight);
        SDL_Rect inner = {highlight.x + 1, highlight.y + 1, highlight.w - 2, highlight.h - 2};
        SDL_RenderDrawRect(renderer, &inner);
    }

    const SDL_Color borderColor = {255, 255, 255, 255};
    const SDL_Color textColor = {255, 255, 255, 255};
    ui::UIRenderUtils::RenderButton(renderer, btnNextPhase, "Confirmar", fontSmall,
                                    summonPending.cardToSacrifice != nullptr, {50, 180, 50, 255},
                                    {80, 220, 80, 255}, borderColor, textColor);
    ui::UIRenderUtils::RenderButton(renderer, btnCancel, "Cancelar", fontSmall, true,
                                    {80, 80, 200, 255}, {120, 120, 240, 255}, borderColor,
                                    textColor);
}

void SceneBattle::RenderHand(SDL_Renderer *renderer) const {
    for (auto *c : playerPiles.hand)
        c->Render(renderer);
}

void SceneBattle::RenderButtons(SDL_Renderer *renderer) const {
    if (IsBattleOver() || summonPending.active) return;

    bool isPlayer = turnManager.IsPlayerTurn();
    auto step = turnManager.GetCombatStep();
    const bool defending = IsPlayerDeclaringDefenders();

    const SDL_Color borderColor = {255, 255, 255, 255};
    const SDL_Color textColor = {255, 255, 255, 255};

    const bool confirmLabel =
        defending || (turnManager.GetPhase() == BattlePhase::COMBAT && isPlayer &&
                      step == CombatStep::ATTACK_MAGIC);
    const std::string nextPhaseLabel = confirmLabel ? "Confirmar" : "Próximo";

    ui::UIRenderUtils::RenderButton(renderer, btnNextPhase, nextPhaseLabel, fontSmall,
                                    isPlayer || defending, {220, 160, 0, 255}, {250, 200, 40, 255},
                                    borderColor, textColor);

    if (board.ShouldShowAttackButton())
        ui::UIRenderUtils::RenderButton(renderer, btnAttack, "Atacar", fontSmall,
                                        board.CanDeclareAttack(), {200, 50, 50, 255},
                                        {230, 80, 80, 255}, borderColor, textColor);

    if (step == CombatStep::ATTACK_MAGIC && isPlayer)
        ui::UIRenderUtils::RenderButton(renderer, btnCancel, "Cancelar", fontSmall, true,
                                        {80, 80, 200, 255}, {120, 120, 240, 255}, borderColor,
                                        textColor);

    if (defending)
        ui::UIRenderUtils::RenderButton(renderer, btnCancel, "Limpar", fontSmall, true,
                                        {80, 80, 200, 255}, {120, 120, 240, 255}, borderColor,
                                        textColor);
}

// ── Destaque dos bloqueios ────────────────────────────────────────

void SceneBattle::RenderCardOutline(SDL_Renderer *renderer, const Card *card,
                                    SDL_Color color) const {
    if (!card) return;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int i = 0; i < 3; ++i) {
        SDL_Rect outline = {card->GetX() - 3 + i, card->GetY() - 3 + i, card->GetWidth() + 6 - i * 2,
                            card->GetHeight() + 6 - i * 2};
        SDL_RenderDrawRect(renderer, &outline);
    }
}

void SceneBattle::RenderLink(SDL_Renderer *renderer, const Card *from, const Card *to,
                             SDL_Color color) const {
    if (!from || !to) return;

    const int x1 = from->GetX() + from->GetWidth() / 2;
    const int y1 = from->GetY() + from->GetHeight() / 2;
    const int x2 = to->GetX() + to->GetWidth() / 2;
    const int y2 = to->GetY() + to->GetHeight() / 2;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int offset = -1; offset <= 1; ++offset)
        SDL_RenderDrawLine(renderer, x1 + offset, y1, x2 + offset, y2);
}

void SceneBattle::RenderDefensePhase(SDL_Renderer *renderer) const {
    if (turnManager.GetPhase() != BattlePhase::COMBAT) return;
    if (turnManager.GetCombatStep() != CombatStep::DECLARE_DEFENDERS) return;

    const SDL_Color blocked = {80, 220, 120, 255};
    const SDL_Color unblocked = {235, 70, 70, 255};
    const SDL_Color selected = {255, 220, 80, 255};

    // Atacantes: verde = bloqueado, vermelho = vai passar direto pra vida
    for (const Card *attacker : board.GetAttackers())
        RenderCardOutline(renderer, attacker,
                          board.GetDefenderOf(attacker) ? blocked : unblocked);

    for (const DefenderAssignment &assignment : board.GetDefenderAssignments()) {
        RenderCardOutline(renderer, assignment.defender, blocked);
        RenderLink(renderer, assignment.defender, assignment.attacker, blocked);
    }

    if (pendingDefender) RenderCardOutline(renderer, pendingDefender, selected);

    if (!fontSmall || !IsPlayerDeclaringDefenders()) return;

    const std::string hint =
        pendingDefender ? "Clique no atacante que " + pendingDefender->GetName() + " vai defender"
                        : "Escolha uma criatura para defender e depois o atacante";

    int textW = 0;
    int textH = 0;
    TTF_SizeUTF8(fontSmall, hint.c_str(), &textW, &textH);
    ui::UIRenderUtils::RenderText(renderer, hint, (1600 - textW) / 2, 370, selected, fontSmall);
}

void SceneBattle::RenderHealthBars(SDL_Renderer *renderer) const {
    if (!currentState || !opponent || !fontSmall) return;

    {
        char hpText[64];
        snprintf(hpText, sizeof(hpText), "Oponente: %d/%d", opponent->currentHealth,
                 opponent->maxHealth);
        SDL_Color color =
            opponent->isGuardian ? SDL_Color{220, 130, 30, 255} : SDL_Color{200, 50, 50, 255};
        ui::UIRenderUtils::RenderText(renderer, hpText, 10, 90, color, fontSmall);
    }

    {
        char hpText[64];
        snprintf(hpText, sizeof(hpText), "Você: %d/%d", currentState->currentHealth,
                 currentState->maxHealth);
        SDL_Color color{50, 200, 80, 255};
        ui::UIRenderUtils::RenderText(renderer, hpText, 10, 870, color, fontSmall);
    }
}

void SceneBattle::RenderMana(SDL_Renderer *renderer) const {
    if (!currentState || !opponent) return;

    const ManaState &pm = currentState->mana;
    const ManaState &om = opponent->mana;

    auto clampMana = [](int value) { return value < 0 ? 0 : value > 99 ? 99 : value; };

    auto drawManaLabel = [&](int current, int total, int x, int y, Uint8 bgR, Uint8 bgG, Uint8 bgB,
                             SDL_Color textColor) {
        const int safeCurrent = clampMana(current);
        const int safeTotal = clampMana(total);
        const std::string text = std::to_string(safeCurrent) + "/" + std::to_string(safeTotal);

        int textW = 0;
        int textH = 0;
        TTF_SizeUTF8(fontSmall, text.c_str(), &textW, &textH);

        const int paddingX = 10;
        const int paddingY = 6;
        SDL_Rect panel = {x - paddingX, y - paddingY, textW + paddingX * 2, textH + paddingY * 2};

        SDL_SetRenderDrawColor(renderer, bgR, bgG, bgB, 210);
        SDL_RenderFillRect(renderer, &panel);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &panel);

        const int textX = panel.x + (panel.w - textW) / 2;
        const int textY = panel.y + (panel.h - textH) / 2;
        ui::UIRenderUtils::RenderText(renderer, text, textX, textY, textColor, fontSmall);
    };

    drawManaLabel(pm.current, pm.total, 15, 760, 10, 24, 70, SDL_Color{100, 200, 255, 255});
    drawManaLabel(om.current, om.total, 1500, 40, 70, 12, 12, SDL_Color{255, 120, 120, 255});
}

void SceneBattle::RenderHUD(SDL_Renderer *renderer) const {
    SDL_Rect panel = {10, 10, 185, 70};
    bool isPlayer = turnManager.IsPlayerTurn();

    SDL_SetRenderDrawColor(renderer, isPlayer ? 30 : 180, isPlayer ? 80 : 30, isPlayer ? 180 : 30,
                           210);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &panel);

    int phaseIdx = static_cast<int>(turnManager.GetPhase());
    for (int i = 0; i < 4; ++i) {
        SDL_Rect pip = {15 + i * 44, 60, 36, 10};
        SDL_SetRenderDrawColor(renderer, i == phaseIdx ? 255 : 60, i == phaseIdx ? 220 : 60,
                               i == phaseIdx ? 50 : 60, 255);
        SDL_RenderFillRect(renderer, &pip);
    }

    if (turnManager.GetPhase() == BattlePhase::COMBAT) {
        int stepIdx = static_cast<int>(turnManager.GetCombatStep());
        for (int i = 1; i <= 4; ++i) {
            SDL_Rect pip = {15 + (i - 1) * 44, 75, 36, 6};
            SDL_SetRenderDrawColor(renderer, i <= stepIdx ? 255 : 40, i <= stepIdx ? 100 : 40, 40,
                                   255);
            SDL_RenderFillRect(renderer, &pip);
        }
    }
}

void SceneBattle::RenderOutcome(SDL_Renderer *renderer) const {
    if (!font) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, 1600, 900};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    const char *text = (outcome == BattleOutcome::PLAYER_WIN) ? "VITORIA!" : "DERROTA!";
    SDL_Color color = (outcome == BattleOutcome::PLAYER_WIN) ? SDL_Color{30, 255, 60, 255}
                                                             : SDL_Color{255, 50, 50, 255};

    int textW = 0;
    int textH = 0;
    TTF_SizeUTF8(font, text, &textW, &textH);
    const int textX = (1600 - textW) / 2;
    const int textY = (900 - textH) / 2;
    ui::UIRenderUtils::RenderText(renderer, text, textX, textY, color, font);
}

// ═══════════════════════════════════════════════════════════════════
//  Deck
// ═══════════════════════════════════════════════════════════════════

bool SceneBattle::SetCurrentPlayerState(Player *p) {
    if (!p) {
        std::cout << "Player invalido." << std::endl;
        return false;
    }
    currentState = p;
    return true;
}

void SceneBattle::ResetBattleState() {
    for (auto *obj : objects)
        delete obj;
    objects.clear();
    playerPiles.drawPile.clear();
    playerPiles.hand.clear();
    playerPiles.discardPile.clear();
    opponentPiles.drawPile.clear();
    opponentPiles.hand.clear();
    opponentPiles.discardPile.clear();
    cardObjects.clear();
    aiCardsToPlay.clear();
    aiCardsToPlayIndex = 0;
    aiDefensePlan.clear();
    aiDefensePlanIndex = 0;
    pendingDefender = nullptr;
    board.Reset();
    summonPending.Clear();
    if (currentState) currentState->Reset();
    if (opponent) opponent->Reset();
}

void SceneBattle::AddDeckCardToDrawPile(Entity *owner, const std::string &cardId) {
    int id = 0;
    try {
        id = std::stoi(cardId);
    } catch (...) {
        std::cout << "ID invalido: " << cardId << std::endl;
        return;
    }

    Card *card = nullptr;
    if (cardDatabase.GetCreature(id)) {
        card = CardFactory::CreateCreatureCard(cardDatabase, id);
    } else if (const SpellData *sd = cardDatabase.GetSpell(id)) {
        card = new SpellCard(sd, -200, -200);
    } else {
        std::cout << "Carta ID " << id << " nao encontrada." << std::endl;
        return;
    }
    if (!card) return;
    GetPilesFor(owner).drawPile.push_back(card);
    objects.push_back(card);
    cardObjects.push_back(card);
}

void SceneBattle::BuildDrawPile(Entity *entity) {
    if (!entity) return;
    for (const auto &id : entity->GetMasterDeck()) {
        AddDeckCardToDrawPile(entity, id);
    }
}

void SceneBattle::ShuffleDrawPile(Entity* entity) {
    auto& piles = GetPilesFor(entity);
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::shuffle(piles.drawPile.begin(), piles.drawPile.end(), rng);
}

void SceneBattle::DrawCards(Entity *entity, int amount) {
    if (!entity || amount <= 0) return;
    auto &piles = GetPilesFor(entity);

    for (int i = 0; i < amount; ++i) {
        if (piles.drawPile.empty()) break;
        Card *card = piles.drawPile.back();
        piles.drawPile.pop_back();
        GameManager::PlaySFX("card_pull");

        if (entity->IsHandFull(static_cast<int>(piles.hand.size()))) {
            objects.erase(std::remove(objects.begin(), objects.end(), card), objects.end());
            cardObjects.erase(std::remove(cardObjects.begin(), cardObjects.end(), card),
                              cardObjects.end());
            delete card;
        } else {
            piles.hand.push_back(card);
            if (renderer) card->LoadTexture(renderer);
        }
    }

    if (entity == currentState) RearrangeHand();
}