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
    LocationDef& startLoc = manager.addLocation(LocationType::Start, "Start", 11, 0, 0);
    startLoc.descriptionPack.descriptions = {"Start!"};
    LocationDef& interLoc = manager.addLocation(LocationType::Regular, "Intermediate", 13, 1, 0);
    interLoc.descriptionPack.descriptions = {"Intermediate!"};
    LocationDef& finishLoc = manager.addLocation(LocationType::Finish, "Finish", 12, 2, 0);
    finishLoc.descriptionPack.descriptions = {"Finish!"};

    // Add edges: 2 from start to intermediate
    auto& edge1 = manager.addEdge(startLoc.id, interLoc.id, "Go to inter 1", "Path 1");
    auto& edge2 = manager.addEdge(startLoc.id, interLoc.id, "Go to inter 2", "Path 2");
    // 3 from intermediate to start
    auto& edge3 = manager.addEdge(interLoc.id, startLoc.id, "Back to start 1", "Return 1");
    auto& edge4 = manager.addEdge(interLoc.id, startLoc.id, "Back to start 2", "Return 2");
    auto& edge5 = manager.addEdge(interLoc.id, startLoc.id, "Back to start 3", "Return 3");
    // 1 from intermediate to finish
    auto& edge6 = manager.addEdge(interLoc.id, finishLoc.id, "To finish", "Final path");

    // Initialize game state
    GameInitializer initializer(game);
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
    // 1. Collect edge IDs from start to intermediate
    std::vector<int> startToInter = collectEdgeIds(step->options, startLoc.id, interLoc.id);
    if (startToInter.size() != 2) {
        std::cerr << "Test failed: Expected 2 edges from start to intermediate, got " << startToInter.size() << std::endl;
        return 1;
    }
    // 2. Take edge1: Start -> Intermediate
    tge::player::runtime::CurrentLocation inter1;
    if (!moveAndCheckCurrent(engine, *step, startToInter[0], "Test failed: Could not take edge1 or not at intermediate after edge1.", inter1)) return 1;
    // 3. Collect edge IDs from intermediate to start
    std::vector<int> interToStartIds = collectEdgeIds(inter1.options, interLoc.id, startLoc.id);
    if (interToStartIds.size() != 3) {
        std::cerr << "Test failed: Expected 3 edges from intermediate to start, got " << interToStartIds.size() << std::endl;
        return 1;
    }
    // 4. Take edge3: Intermediate -> Start
    tge::player::runtime::CurrentLocation start2;
    if (!moveAndCheckCurrent(engine, inter1, interToStartIds[0], "Test failed: Could not take edge3 or not at start after edge3.", start2)) return 1;
    // 5. Take edge2: Start -> Intermediate
    int edge2id = startToInter[1];
    tge::player::runtime::CurrentLocation inter2;
    if (!moveAndCheckCurrent(engine, start2, edge2id, "Test failed: Could not take edge2 or not at intermediate after edge2.", inter2)) return 1;
    // 6. Take edge4: Intermediate -> Start
    tge::player::runtime::CurrentLocation start3;
    if (!moveAndCheckCurrent(engine, inter2, interToStartIds[1], "Test failed: Could not take edge4 or not at start after edge4.", start3)) return 1;
    // 7. Take edge1 again: Start -> Intermediate
    tge::player::runtime::CurrentLocation inter3;
    if (!moveAndCheckCurrent(engine, start3, startToInter[0], "Test failed: Could not take edge1 again or not at intermediate after edge1 again.", inter3)) return 1;
    // 8. Take edge5: Intermediate -> Start
    tge::player::runtime::CurrentLocation start4;
    if (!moveAndCheckCurrent(engine, inter3, interToStartIds[2], "Test failed: Could not take edge5 or not at start after edge5.", start4)) return 1;
    // 9. Take edge2 again: Start -> Intermediate
    tge::player::runtime::CurrentLocation inter4;
    if (!moveAndCheckCurrent(engine, start4, edge2id, "Test failed: Could not take edge2 again or not at intermediate after edge2 again.", inter4)) return 1;
    // 10. Take edge from Intermediate to Finish
    std::vector<int> toFinish = collectEdgeIds(inter4.options, interLoc.id, finishLoc.id);
    if (toFinish.empty()) {
        std::cerr << "Test failed: Could not find edge from intermediate to finish." << std::endl;
        return 1;
    }
    auto t8 = engine.choose(inter4, toFinish[0]);
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
