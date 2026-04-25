#include <QtCore/QCoreApplication>
#include <iostream>
#include "../src/tge/domain.h"
#include "../src/tge/editor/types.h"
#include "../src/tge/editor/runtime/manager.h"

using namespace tge::domain;
using namespace tge::editor;
using namespace tge::editor::runtime;

static const EdgeDef* findEdge(const GameDef& game, int fromId, int toId) {
    for (auto it = game.edges.constBegin(); it != game.edges.constEnd(); ++it) {
        const auto& edge = it.value();
        if (edge.fromLocation == fromId && edge.toLocation == toId) {
            return &edge;
        }
    }
    return nullptr;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    GameDef game;
    EditorState state;
    IdGenerator idGen;
    Manager manager(game, state, idGen);

    // Base regular location
    auto& regular = manager.addLocation("Regular", 3, 0, 0);

    // 1) Regular loop must create a nearby service location and two edges.
    const int firstServiceId = manager.addLoopEdge(regular.id, "Loop option", "Loop transition", "x > 0");
    if (firstServiceId == -1) {
        std::cerr << "Test failed: loop on regular location was rejected: "
                  << manager.lastError().toStdString() << std::endl;
        return 1;
    }
    if (!game.locations.contains(firstServiceId)) {
        std::cerr << "Test failed: created service location is missing." << std::endl;
        return 1;
    }

    const auto& firstService = game.locations[firstServiceId];
    if (firstService.type != LocationType::Service) {
        std::cerr << "Test failed: loop helper location must be Service." << std::endl;
        return 1;
    }
    if (firstService.coordX == regular.coordX && firstService.coordY == regular.coordY) {
        std::cerr << "Test failed: loop helper location must be placed near source, not on source." << std::endl;
        return 1;
    }

    const EdgeDef* eRegToSrv = findEdge(game, regular.id, firstServiceId);
    const EdgeDef* eSrvToReg = findEdge(game, firstServiceId, regular.id);
    if (!eRegToSrv || !eSrvToReg) {
        std::cerr << "Test failed: regular loop must create both edges (src->service and service->src)." << std::endl;
        return 1;
    }
    if (eRegToSrv->optionText != "Loop option" ||
        eRegToSrv->transitionText != "Loop transition" ||
        eRegToSrv->condition != "x > 0") {
        std::cerr << "Test failed: forward loop edge payload mismatch." << std::endl;
        return 1;
    }

    // 2) Service locations are also allowed to have loops (same scheme).
    const int secondServiceId = manager.addLoopEdge(firstServiceId, "Service loop", "Service transition", "flag == true");
    if (secondServiceId == -1) {
        std::cerr << "Test failed: loop on service location was rejected: "
                  << manager.lastError().toStdString() << std::endl;
        return 1;
    }
    if (!game.locations.contains(secondServiceId) || game.locations[secondServiceId].type != LocationType::Service) {
        std::cerr << "Test failed: service->loop must create an additional service location." << std::endl;
        return 1;
    }

    const EdgeDef* eSrvToSrv2 = findEdge(game, firstServiceId, secondServiceId);
    const EdgeDef* eSrv2ToSrv = findEdge(game, secondServiceId, firstServiceId);
    if (!eSrvToSrv2 || !eSrv2ToSrv) {
        std::cerr << "Test failed: service loop must create both edges (service->service and back)." << std::endl;
        return 1;
    }
    if (eSrvToSrv2->optionText != "Service loop" ||
        eSrvToSrv2->transitionText != "Service transition" ||
        eSrvToSrv2->condition != "flag == true") {
        std::cerr << "Test failed: service forward loop edge payload mismatch." << std::endl;
        return 1;
    }

    if (game.edges.size() != 4) {
        std::cerr << "Test failed: expected 4 edges after two loop creations, got "
                  << game.edges.size() << std::endl;
        return 1;
    }

    std::cout << "Editor loop edge test passed." << std::endl;
    return 0;
}
