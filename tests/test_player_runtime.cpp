#include <QtCore/QCoreApplication>
#include <iostream>
#include <variant>
#include "../src/tge/domain.h"
#include "../src/tge/editor/types.h"
#include "../src/tge/editor/runtime/manager.h"
#include "../src/tge/player/types.h"
#include "../src/tge/player/runtime/gameinitializer.h"
#include "../src/tge/player/runtime/engine.h"

using namespace tge::domain;
using namespace tge::editor;
using namespace tge::editor::runtime;
using namespace tge::player;
using namespace tge::player::runtime;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    GameDef game;
    EditorState state;
    IdGenerator idGen;
    Manager manager(game, state, idGen);

    // Add start location
    auto& startLoc = manager.addStartLocation("Start", 0, 0); // Green
    // Add finish location
    auto& finishLoc = manager.addFinishLocation("Finish", 1, 0); // Deep Orange
    // Add edge from start to finish
    auto* edge = manager.addEdge(startLoc.id, finishLoc.id, "Go to finish", "You move to the finish.");
    if (!edge) {
        std::cerr << "Test failed: Edge creation error: " << manager.lastError().toStdString() << std::endl;
        return 1;
    }

    game.globalVariables.append({"P1", "Health", "Player health", VarType::Integer, "100"});
    game.globalVariables.append({"P2", "Gold", "Player gold", VarType::Integer, "7"});
    game.infoDisplayItems.append({1, "Health", "[P1]", InfoDisplayItemMode::Actual, 10, true, true});
    game.infoDisplayItems.append({2, "Debug Gold", "[P2]", InfoDisplayItemMode::Debug, 20, true, true});
    game.infoDisplayItems.append({3, "Summary", "([P1] + [P2])", InfoDisplayItemMode::Actual, 30, true, true});
    edge->variableSettings.append({1, "([P1] >= 100)", "([P1] - 10)"});
    edge->infoDisplayItemSettings.append({1, true, 5, true, false, true, false});

    // Initialize game state
    GameInitializer initializer(game, GameMode::Normal);
    GameInitResult result = initializer.initialize();
    if (!result.state.has_value()) {
        std::cerr << "Test failed: Initialization error: ";
        if (result.error.has_value()) {
            std::cerr << result.error.value().toStdString();
        } else {
            std::cerr << "Unknown error";
        }
        std::cerr << std::endl;
        return 1;
    }
    GameState state_ = std::move(result.state.value());

    // Verify that dynamic locations were created
    if (state_.locations.size() != 2) {
        std::cerr << "Test failed: Expected 2 dynamic locations, got " << state_.locations.size() << std::endl;
        return 1;
    }
    // Find start and finish locations by type
    const LocationState* dynStart = nullptr;
    const LocationState* dynFinish = nullptr;
    for (const auto& locPair : state_.locations) {
        const LocationState* loc = locPair.second.get();
        if (loc->def && loc->def->type == LocationType::Start) dynStart = loc;
        if (loc->def && loc->def->type == LocationType::Finish) dynFinish = loc;
    }
    if (!dynStart || !dynFinish) {
        std::cerr << "Test failed: Could not find start or finish location by type." << std::endl;
        return 1;
    }
    if (state_.variables.size() != 2) {
        std::cerr << "Test failed: Expected 2 initialized variables, got " << state_.variables.size() << std::endl;
        return 1;
    }
    if (!state_.variables[0].def || state_.variables[0].value != "100") {
        std::cerr << "Test failed: Variable P1 was not initialized from defaultValue." << std::endl;
        return 1;
    }
    if (!state_.variables[1].def || state_.variables[1].value != "7") {
        std::cerr << "Test failed: Variable P2 was not initialized from defaultValue." << std::endl;
        return 1;
    }

    if (state_.infoDisplayItems.size() != 3) {
        std::cerr << "Test failed: Expected 3 initialized info display items, got " << state_.infoDisplayItems.size() << std::endl;
        return 1;
    }
    if (!state_.infoDisplayItems[0].def || !state_.infoDisplayItems[0].visible || !state_.infoDisplayItems[0].allowVisibilityChanges) {
        std::cerr << "Test failed: Actual info item visibility in Normal mode is incorrect." << std::endl;
        return 1;
    }
    if (!state_.infoDisplayItems[1].def || state_.infoDisplayItems[1].visible || state_.infoDisplayItems[1].allowVisibilityChanges) {
        std::cerr << "Test failed: Debug info item must be hidden and locked in Normal mode." << std::endl;
        return 1;
    }
    if (!state_.infoDisplayItems[2].def || !state_.infoDisplayItems[2].visible || !state_.infoDisplayItems[2].allowVisibilityChanges) {
        std::cerr << "Test failed: Formula-driven actual info item initialization is incorrect." << std::endl;
        return 1;
    }

    GameInitializer debugInitializer(game, GameMode::Debug);
    GameInitResult debugResult = debugInitializer.initialize();
    if (!debugResult.state.has_value()) {
        std::cerr << "Test failed: Debug initialization error: "
                  << debugResult.error.value_or("Unknown error").toStdString() << std::endl;
        return 1;
    }
    GameState debugState = std::move(debugResult.state.value());
    if (debugState.infoDisplayItems.size() != 3) {
        std::cerr << "Test failed: Expected 3 debug info display items, got " << debugState.infoDisplayItems.size() << std::endl;
        return 1;
    }
    if (!debugState.infoDisplayItems[0].visible || debugState.infoDisplayItems[0].allowVisibilityChanges ||
        !debugState.infoDisplayItems[1].visible || debugState.infoDisplayItems[1].allowVisibilityChanges ||
        !debugState.infoDisplayItems[2].visible || debugState.infoDisplayItems[2].allowVisibilityChanges) {
        std::cerr << "Test failed: In Debug mode all info items must be visible and visibility-locked." << std::endl;
        return 1;
    }

    Engine engine(game, GameMode::Normal);
    if (engine.hasError()) {
        std::cerr << "Test failed: Engine initialization error: " << engine.error().toStdString() << std::endl;
        return 1;
    }
    const auto current = engine.start();
    if (!current.has_value()) {
        std::cerr << "Test failed: Engine could not return start location." << std::endl;
        return 1;
    }
    if (current->options.size() != 1 || !current->options[0].isAvailable || !current->options[0].edge || !current->options[0].edge->def) {
        std::cerr << "Test failed: Expected one available transition from start." << std::endl;
        return 1;
    }
    auto transition = engine.choose(*current, current->options[0].edge->def->id);
    if (!transition.has_value()) {
        std::cerr << "Test failed: Engine could not choose available edge." << std::endl;
        return 1;
    }
    if (transition->pendingVariableChanges.size() != 1 || transition->pendingVariableChanges[0].newValue != "90") {
        std::cerr << "Test failed: Pending variable changes were computed incorrectly." << std::endl;
        return 1;
    }
    if (transition->pendingInfoDisplayItemChanges.size() != 2) {
        std::cerr << "Test failed: Expected explicit and formula-driven pending info display changes, got "
                  << transition->pendingInfoDisplayItemChanges.size() << std::endl;
        return 1;
    }
    const PendingInfoDisplayItemChange* pendingItemChange = nullptr;
    const PendingInfoDisplayItemChange* pendingSummaryChange = nullptr;
    for (const auto& change : transition->pendingInfoDisplayItemChanges) {
        if (change.itemIndex == 1) {
            pendingItemChange = &change;
        }
        if (change.itemIndex == 3) {
            pendingSummaryChange = &change;
        }
    }
    if (!pendingItemChange ||
        !pendingItemChange->changePriority || pendingItemChange->newPriority != 5 ||
        !pendingItemChange->changeVisibility || pendingItemChange->newVisibility ||
        !pendingItemChange->changeShowValue || pendingItemChange->newShowValue ||
        !pendingItemChange->changeValue || pendingItemChange->newValue != "90") {
        std::cerr << "Test failed: Pending info display changes were computed incorrectly." << std::endl;
        return 1;
    }
    if (!pendingSummaryChange || pendingSummaryChange->changePriority || pendingSummaryChange->changeVisibility ||
        pendingSummaryChange->changeShowValue || !pendingSummaryChange->changeValue ||
        pendingSummaryChange->newValue != "97") {
        std::cerr << "Test failed: Formula-driven pending info display change was not prepared correctly." << std::endl;
        return 1;
    }
    auto stepResult = engine.step(*transition);
    if (!std::holds_alternative<FinishLocation>(stepResult)) {
        std::cerr << "Test failed: Expected step to reach finish location." << std::endl;
        return 1;
    }
    if (engine.state().variables[0].value != "90") {
        std::cerr << "Test failed: Variable update was not applied on step." << std::endl;
        return 1;
    }
    if (engine.state().infoDisplayItems[0].priority != 5 || engine.state().infoDisplayItems[0].visible ||
        engine.state().infoDisplayItems[0].showFormulaValue || engine.state().infoDisplayItems[0].value != "90") {
        std::cerr << "Test failed: Info display update was not applied on step." << std::endl;
        return 1;
    }
    if (engine.state().infoDisplayItems[2].value != "97") {
        std::cerr << "Test failed: Formula-driven info display item was not recalculated after step." << std::endl;
        return 1;
    }

    std::cout << "Player runtime test passed: dynamic states, modes, and HUD recalculation are correct." << std::endl;
    return 0;
}
