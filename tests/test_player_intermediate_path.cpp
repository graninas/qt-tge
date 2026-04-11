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

    // Start location with description
    LocationDef& startLoc = manager.addLocation(LocationType::Start, "Start", "green", 0, 0);
    startLoc.descriptionPack.descriptions = {"Hello!"};

    // Regular location with two descriptions
    LocationDef& regularLoc = manager.addLocation(LocationType::Regular, "Intermediate", "blue", 1, 0);
    regularLoc.descriptionPack.descriptions = {"Intermediate 1", "Intermediate 2"};

    // Finish location with description
    LocationDef& finishLoc = manager.addLocation(LocationType::Finish, "Finish", "red", 2, 0);
    finishLoc.descriptionPack.descriptions = {"Bye!"};

    // Edge 1: Start -> Regular
    manager.addEdge(startLoc.id, regularLoc.id, "To Intermediate", "");
    // Edge 2: Regular -> Finish
    manager.addEdge(regularLoc.id, finishLoc.id, "To Finish", "");

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

    // Use the engine to step through the game
    Engine engine(game);
    if (engine.hasError()) {
        std::cerr << "Test failed: Engine error: " << engine.error().toStdString() << std::endl;
        return 1;
    }
    auto startLocStep = engine.start();
    if (!startLocStep.has_value()) {
        std::cerr << "Test failed: Could not get start location." << std::endl;
        return 1;
    }
    // Check start location description and debug
    if (startLocStep->description != "Hello!") {
        std::cerr << "Test failed: Start location description mismatch: '" << startLocStep->description.toStdString() << "'" << std::endl;
        return 1;
    }
    if (startLocStep->debugMessages.isEmpty() || !startLocStep->debugMessages[0].contains("Selector: not implemented")) {
        std::cerr << "Test failed: Start location debug message missing or incorrect." << std::endl;
        return 1;
    }
    // Choose edge to intermediate
    int edgeToIntermediate = startLocStep->options[0]->def->id;
    auto transition1 = engine.choose(*startLocStep, edgeToIntermediate);
    if (!transition1.has_value()) {
        std::cerr << "Test failed: Could not transition to intermediate." << std::endl;
        return 1;
    }
    // Step to intermediate
    auto nextStep = engine.step(*transition1);
    if (!(std::holds_alternative<tge::player::runtime::CurrentLocation>(nextStep))) {
        std::cerr << "Test failed: Expected CurrentLocation after first step." << std::endl;
        return 1;
    }
    const auto& interLoc = std::get<tge::player::runtime::CurrentLocation>(nextStep);
    if (interLoc.description != "Intermediate 1") {
        std::cerr << "Test failed: Intermediate location description mismatch: '" << interLoc.description.toStdString() << "'" << std::endl;
        return 1;
    }
    // Choose edge to finish
    int edgeToFinish = interLoc.options[0]->def->id;
    auto transition2 = engine.choose(interLoc, edgeToFinish);
    if (!transition2.has_value()) {
        std::cerr << "Test failed: Could not transition to finish." << std::endl;
        return 1;
    }
    // Step to finish
    auto finishStep = engine.step(*transition2);
    if (!(std::holds_alternative<tge::player::runtime::FinishLocation>(finishStep))) {
        std::cerr << "Test failed: Expected FinishLocation after final step." << std::endl;
        return 1;
    }
    const auto& finishLocStep = std::get<tge::player::runtime::FinishLocation>(finishStep);
    if (finishLocStep.description != "Bye!") {
        std::cerr << "Test failed: Finish location description mismatch: '" << finishLocStep.description.toStdString() << "'" << std::endl;
        return 1;
    }
    std::cout << "Player intermediate path test passed: all steps and descriptions correct." << std::endl;
    return 0;
}
