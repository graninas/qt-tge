#include <QtCore/QCoreApplication>
#include <iostream>
#include "../src/tge/domain.h"
#include "../src/tge/editor/types.h"
#include "../src/tge/editor/runtime/manager.h"

using namespace tge::domain;
using namespace tge::editor;
using namespace tge::editor::runtime;

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

    bool edgeConnected = manager.connectEdge(startLoc.id, finishLoc.id, edge->id);
    if (!edgeConnected) {
        std::cerr << "Test failed: Edge connection error: " << manager.lastError().toStdString() << std::endl;
        return 1;
    }

      // Verify structure
    if (manager.game().locations.size() != 2) {
        std::cerr << "Test failed: Expected 2 locations." << std::endl;
        return 1;

    }
    if (manager.game().edges.size() != 1) {
        std::cerr << "Test failed: Expected 1 edge." << std::endl;
        return 1;
    }
    if (manager.game().locations[startLoc.id].type != LocationType::Start || manager.game().locations[finishLoc.id].type != LocationType::Finish) {
        std::cerr << "Test failed: Incorrect location types." << std::endl;
        return 1;
    }
    if (manager.game().locations[startLoc.id].outgoingEdges.size() != 1 || manager.game().locations[finishLoc.id].incomingEdges.size() != 1) {
        std::cerr << "Test failed: Incorrect edge connections." << std::endl;
        return 1;
    }

    // Verify edge details
    auto& createdEdge = manager.game().edges[edge->id];
    if (createdEdge.fromLocation != startLoc.id || createdEdge.toLocation != finishLoc.id || createdEdge.optionText != "Go to finish" || createdEdge.transitionText != "You move to the finish.") {
        std::cerr << "Test failed: Edge details do not match." << std::endl;
        return 1;
    }

    std::cout << "Editor manager test completed." << std::endl;
    return 0;
}
