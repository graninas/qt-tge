#include <QtCore/QCoreApplication>
#include <iostream>
#include "../src/tge/domain.h"
#include "../src/tge/editor/types.h"
#include "../src/tge/editor/runtime/manager.h"
#include "../src/tge/player/types.h"
#include "../src/tge/player/runtime/gameinitializer.h"

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
    auto& startLoc = manager.addLocation(LocationType::Start, "Start", 11, 0, 0); // Green
    // Add finish location
    auto& finishLoc = manager.addLocation(LocationType::Finish, "Finish", 12, 1, 0); // Deep Orange
    // Add edge from start to finish
    manager.addEdge(startLoc.id, finishLoc.id, "Go to finish", "You move to the finish.");

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
    const GameState& state_ = result.state.value();

    // Verify that dynamic locations were created
    if (state_.locations.size() != 2) {
        std::cerr << "Test failed: Expected 2 dynamic locations, got " << state_.locations.size() << std::endl;
        return 1;
    }
    // Check that the first is Start and the second is Finish, and they reference the correct static locations
    const LocationState& dynStart = state_.locations[0];
    const LocationState& dynFinish = state_.locations[1];
    if (!dynStart.def || !dynFinish.def) {
        std::cerr << "Test failed: Dynamic locations do not reference static definitions." << std::endl;
        return 1;
    }
    if (dynStart.def->type != LocationType::Start || dynFinish.def->type != LocationType::Finish) {
        std::cerr << "Test failed: Dynamic locations do not reference correct static types." << std::endl;
        return 1;
    }
    // Check that the start location has one outgoing edge
    if (dynStart.outgoingEdges.size() != 1) {
        std::cerr << "Test failed: Start location should have 1 outgoing edge, got " << dynStart.outgoingEdges.size() << std::endl;
        return 1;
    }
    // Check that the dynamic edge references the correct static edge
    const EdgeState& dynEdge = dynStart.outgoingEdges[0];
    if (!dynEdge.def) {
        std::cerr << "Test failed: Dynamic edge does not reference static definition." << std::endl;
        return 1;
    }
    if (dynEdge.def->fromLocation != dynStart.def->id || dynEdge.def->toLocation != dynFinish.def->id) {
        std::cerr << "Test failed: Dynamic edge does not reference correct static edge." << std::endl;
        return 1;
    }
    std::cout << "Player runtime test passed: " << state_.locations.size() << " locations created and interconnected correctly." << std::endl;
    return 0;
}
