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
    auto& startLoc = manager.addLocation(LocationType::Start, "Start", "green", 0, 0);
    // Add finish location
    auto& finishLoc = manager.addLocation(LocationType::Finish, "Finish", "red", 1, 0);
    // Add edge from start to finish
    manager.addEdge(startLoc.id, finishLoc.id, "Go to finish", "You move to the finish.");

    std::cout << "Editor manager test completed." << std::endl;
    return 0;
}
