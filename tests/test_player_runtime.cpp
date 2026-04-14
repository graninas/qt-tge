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
    auto& startLoc = manager.addStartLocation("Start", 0, 0); // Green
    // Add finish location
    auto& finishLoc = manager.addFinishLocation("Finish", 1, 0); // Deep Orange
    // Add edge from start to finish
    auto* edge = manager.addEdge(startLoc.id, finishLoc.id, "Go to finish", "You move to the finish.");
    if (!edge) {
        std::cerr << "Test failed: Edge creation error: " << manager.lastError().toStdString() << std::endl;
        return 1;
    }

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
    // Check that the start location has one outgoing edge
    if (dynStart->outgoingEdges.size() != 1) {
        std::cerr << "Test failed: Start location should have 1 outgoing edge, got " << dynStart->outgoingEdges.size() << std::endl;
        return 1;
    }
    const EdgeState* dynEdge = dynStart->outgoingEdges[0];
    if (!dynEdge) {
        std::cerr << "Test failed: Dynamic edge pointer is nullptr." << std::endl;
        return 1;
    }
    if (!dynEdge->def) {
        std::cerr << "Test failed: Dynamic edge does not reference static definition (def is nullptr)." << std::endl;
        return 1;
    }
    if (dynEdge->def->fromLocation != dynStart->def->id || dynEdge->def->toLocation != dynFinish->def->id) {
        std::cerr << "Test failed: Dynamic edge does not reference correct static edge." << std::endl;
        return 1;
    }
    std::cout << "Player runtime test passed: " << state_.locations.size() << " locations created and interconnected correctly." << std::endl;
    return 0;
}
