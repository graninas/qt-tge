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

// Helper: collect edge IDs between two locations
std::vector<int> collectEdgeIds(const QVector<const EdgeState*>& options, int fromId, int toId) {
    std::vector<int> ids;
    for (const auto& opt : options) {
        if (opt && opt->def && opt->def->fromLocation == fromId && opt->def->toLocation == toId)
            ids.push_back(opt->def->id);
    }
    return ids;
}

// Helper: perform a move and check result type
bool moveAndCheckCurrent(Engine& engine, const tge::player::runtime::CurrentLocation& from, int edgeId, const char* failMsg, tge::player::runtime::CurrentLocation& out) {
    auto t = engine.choose(from, edgeId);
    if (!t.has_value()) { std::cerr << failMsg << std::endl; return false; }
    auto s = engine.step(*t);
    if (!std::holds_alternative<tge::player::runtime::CurrentLocation>(s)) { std::cerr << failMsg << std::endl; return false; }
    out = std::get<tge::player::runtime::CurrentLocation>(s);
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    GameDef game;
    EditorState state;
    IdGenerator idGen;
    Manager manager(game, state, idGen);

    // Add locations
    LocationDef& startLoc = manager.addStartLocation("Start", 0, 0);
    startLoc.description = "Start!";
    LocationDef& interLoc1 = manager.addLocation("Intermediate1", 13, 1, 0);
    interLoc1.description = "Intermediate1!";
    LocationDef& interLoc2 = manager.addLocation("Intermediate2", 14, 2, 0);
    interLoc2.description = "Intermediate2!";
    LocationDef& finishLoc = manager.addFinishLocation("Finish", 3, 0);
    finishLoc.description = "Finish!";

    // Edges: start -> inter1, inter1 <-> inter2 (cycle), inter2 -> finish
    auto* edge1 = manager.addEdge(startLoc.id, interLoc1.id, "Go to inter1", "Path 1");
    auto* edge2 = manager.addEdge(interLoc1.id, interLoc2.id, "To inter2", "Path 2");
    auto* edge3 = manager.addEdge(interLoc2.id, interLoc1.id, "Back to inter1", "Return 1");
    auto* edge4 = manager.addEdge(interLoc2.id, finishLoc.id, "To finish", "Final path");
    if (!edge1 || !edge2 || !edge3 || !edge4) {
        std::cerr << "Test failed: Edge creation error: " << manager.lastError().toStdString() << std::endl;
        return 1;
    }

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

    // Use the engine to step through the game
    Engine engine(game);
    if (engine.hasError()) {
        std::cerr << "Test failed: Engine error: " << engine.error().toStdString() << std::endl;
        return 1;
    }
    auto step = engine.start();
    if (!step.has_value()) {
        std::cerr << "Test failed: Could not get start location." << std::endl;
        return 1;
    }
    // 1. Collect edge IDs from start to inter1
    std::vector<int> startToInter1 = collectEdgeIds(step->options, startLoc.id, interLoc1.id);
    if (startToInter1.size() != 1) {
        std::cerr << "Test failed: Expected 1 edge from start to inter1, got " << startToInter1.size() << std::endl;
        return 1;
    }
    // 2. Take edge: Start -> Inter1
    tge::player::runtime::CurrentLocation inter1;
    if (!moveAndCheckCurrent(engine, *step, startToInter1[0], "Test failed: Could not take edge1 or not at inter1 after edge1.", inter1)) return 1;
    // 3. Collect edge IDs from inter1 to inter2
    std::vector<int> inter1ToInter2 = collectEdgeIds(inter1.options, interLoc1.id, interLoc2.id);
    if (inter1ToInter2.size() != 1) {
        std::cerr << "Test failed: Expected 1 edge from inter1 to inter2, got " << inter1ToInter2.size() << std::endl;
        return 1;
    }
    // 4. Take edge: Inter1 -> Inter2
    tge::player::runtime::CurrentLocation inter2;
    if (!moveAndCheckCurrent(engine, inter1, inter1ToInter2[0], "Test failed: Could not take edge2 or not at inter2 after edge2.", inter2)) return 1;
    // 5. Collect edge IDs from inter2 to inter1 and finish
    std::vector<int> inter2ToInter1 = collectEdgeIds(inter2.options, interLoc2.id, interLoc1.id);
    std::vector<int> inter2ToFinish = collectEdgeIds(inter2.options, interLoc2.id, finishLoc.id);
    if (inter2ToInter1.size() != 1 || inter2ToFinish.size() != 1) {
        std::cerr << "Test failed: Expected 1 edge from inter2 to inter1 and 1 to finish, got " << inter2ToInter1.size() << " and " << inter2ToFinish.size() << std::endl;
        return 1;
    }
    // 6. Take edge: Inter2 -> Inter1 (cycle)
    tge::player::runtime::CurrentLocation inter1b;
    if (!moveAndCheckCurrent(engine, inter2, inter2ToInter1[0], "Test failed: Could not take cycle edge or not at inter1 after cycle.", inter1b)) return 1;
    // 7. Take edge: Inter1 -> Inter2 again
    tge::player::runtime::CurrentLocation inter2b;
    if (!moveAndCheckCurrent(engine, inter1b, inter1ToInter2[0], "Test failed: Could not take edge2 again or not at inter2 after edge2 again.", inter2b)) return 1;
    // 8. Take edge: Inter2 -> Finish
    auto t8 = engine.choose(inter2b, inter2ToFinish[0]);
    if (!t8.has_value()) { std::cerr << "Test failed: Could not take edge to finish." << std::endl; return 1; }
    auto s9 = engine.step(*t8);
    if (!std::holds_alternative<tge::player::runtime::FinishLocation>(s9)) {
        std::cerr << "Test failed: Expected FinishLocation after final step." << std::endl;
        return 1;
    }
    const auto& finishLocStep = std::get<tge::player::runtime::FinishLocation>(s9);
    if (finishLocStep.description != "Finish!") {
        std::cerr << "Test failed: Finish location description mismatch: '" << finishLocStep.description.toStdString() << "'" << std::endl;
        return 1;
    }
    std::cout << "Player intermediate path test passed: all edges traversed and player reached finish." << std::endl;
    return 0;
}
