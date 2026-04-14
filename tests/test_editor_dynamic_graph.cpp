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

    std::cout << "Editor manager test completed." << std::endl;
    return 0;
}
