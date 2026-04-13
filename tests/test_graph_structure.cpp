#include <QtCore/QCoreApplication>
#include <iostream>
#include "../src/tge/domain.h"

using namespace tge::domain;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Create start location
    LocationDef startLoc;
    startLoc.id = 0;
    startLoc.type = LocationType::Start;
    startLoc.label = "Start";
    startLoc.color = 11; // Green
    startLoc.coordX = 0;
    startLoc.coordY = 0;

    // Create finish location
    LocationDef finishLoc;
    finishLoc.id = 1;
    finishLoc.type = LocationType::Finish;
    finishLoc.label = "Finish";
    finishLoc.color = 12; // Deep Orange
    finishLoc.coordX = 1;
    finishLoc.coordY = 0;

    // Add locations to game definition
    GameDef game;
    game.name = "TestGame";
    game.description = "A test game with two nodes.";
    game.locations[startLoc.id] = startLoc;
    game.locations[finishLoc.id] = finishLoc;

    // Verify structure
    if (game.locations.size() != 2) {
        std::cerr << "Test failed: Expected 2 locations." << std::endl;
        return 1;
    }
    if (game.locations[startLoc.id].type != LocationType::Start || game.locations[finishLoc.id].type != LocationType::Finish) {
        std::cerr << "Test failed: Incorrect location types." << std::endl;
        return 1;
    }
    std::cout << "Graph structure test passed!" << std::endl;
    return 0;
}
