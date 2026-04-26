#include "signal_heist.h"

#include <QDebug>

#include "tge/domain.h"
#include "tge/editor/runtime/manager.h"

namespace tge {
namespace demo {

namespace {

using namespace tge::domain;
using tge::editor::runtime::Manager;

EdgeDef* findEdgeByEndpoints(GameDef& game, int fromId, int toId) {
    for (auto it = game.edges.begin(); it != game.edges.end(); ++it) {
        if (it.value().fromLocation == fromId && it.value().toLocation == toId) {
            return &it.value();
        }
    }
    return nullptr;
}

void applyVisibilityOnEdge(EdgeDef* edge, int itemId, bool visible) {
    if (!edge) {
        return;
    }

    EdgeInfoDisplayItemSettingDef s;
    s.itemIndex = itemId;
    s.changePriority = false;
    s.newPriority = 0;
    s.changeVisibility = true;
    s.newVisibility = visible;
    s.changeShowValue = false;
    s.newShowValue = true;
    edge->infoDisplayItemSettings.append(s);
}

} // namespace

/*
 * Demo Quest: Signal Heist
 *
 * Story premise:
 * - The player is a courier-hacker trying to extract from a surveillance-heavy city
 *   after stealing a relay uplink protocol.
 * - The route starts at a Safehouse, moves through infiltration locations, and ends
 *   in one of two outcomes: clean extraction or capture.
 *
 * Graph structure:
 * - Start location: Safehouse.
 * - Regular locations: Black Market, Relay Tower, City Archive, Transit Hub.
 * - Service loop location: generated automatically by addLoopEdge from City Archive.
 * - Finish locations: Extraction Train (good ending), Containment Cell (bad ending).
 * - The graph contains explicit return paths (Tower <-> Market, Tower <-> Archive,
 *   Hub <-> Archive) so the player can revisit previous locations.
 *
 * Global variables used in edge logic:
 * - P1 (Credits): consumed for operations like bribes/tools.
 * - P2 (Suspicion): grows or shrinks based on risky/safe actions.
 * - P3 (Signal): represents mission progress and signal stability.
 *
 * Formula usage in this demo:
 * - Global edge conditions use logical expressions, e.g.
 *   (([P1] >= 1) and ([P2] <= 3)).
 * - Variable settings use local per-variable conditions and arithmetic updates,
 *   e.g. ([P1] - 1), ([P3] + 2), ([P2] - 2).
 * - The service loop itself is gated by arithmetic/logical condition
 *   (([P1] + [P3]) >= 3).
 *
 * Info display (HUD/tablo) behavior:
 * - Two items are defined: Suspicion ([P2]) and Signal ([P3]).
 * - Their visibility is changed by edge-level infoDisplayItemSettings so that in
 *   some parts of the route both are hidden, in others one is hidden, and in
 *   critical segments both are shown.
 *
 * Ending logic:
 * - Good ending requires strong signal and low suspicion:
 *   (([P3] >= 4) and ([P2] <= 2)).
 * - Bad ending triggers when suspicion is high or signal is too weak:
 *   (([P2] >= 4) or ([P3] <= 1)).
 */
UiModel makeSignalHeistGame() {
    UiModel model;
    Manager& mgr = model.manager;

    model.gameDef.name = "Demo Quest: Signal Heist";
    model.gameDef.description = "A branching city infiltration with return paths, loop service checks, and two endings.";

    model.gameDef.globalVariables.append({"P1", "Credits", "Spendable currency for bribes and tools.", VarType::Integer, "3"});
    model.gameDef.globalVariables.append({"P2", "Suspicion", "How much attention the player attracts.", VarType::Integer, "0"});
    model.gameDef.globalVariables.append({"P3", "Signal", "Stability of the stolen uplink signal.", VarType::Integer, "1"});

    model.gameDef.infoDisplayItems.append({1, "Suspicion", "[P2]", InfoDisplayItemMode::Actual, 10, false, true});
    model.gameDef.infoDisplayItems.append({2, "Signal", "[P3]", InfoDisplayItemMode::Actual, 20, false, true});

    auto& startLoc = mgr.addStartLocation("Safehouse", 0, 0);
    auto& marketLoc = mgr.addLocation("Black Market", 6, 2, 0);
    auto& towerLoc = mgr.addLocation("Relay Tower", 7, 4, 1);
    auto& archiveLoc = mgr.addLocation("City Archive", 8, 3, 3);
    auto& hubLoc = mgr.addLocation("Transit Hub", 9, 6, 2);
    auto& goodEndLoc = mgr.addFinishLocation("Extraction Train", 8, 8);
    auto& badEndLoc = mgr.addFinishLocation("Containment Cell", 10, 6);

    startLoc.description = "Night rain hides your movement. The only lead is the black market fixer.";
    marketLoc.description = "Vendors whisper about signal boosters and patrol schedules.";
    towerLoc.description = "The relay tower amplifies transmissions, but scanners are active.";
    archiveLoc.description = "Old records hold route keys and maintenance overrides.";
    hubLoc.description = "The transit hub is your launch point: extraction or capture.";
    goodEndLoc.description = "You board the extraction train with a clean signal and disappear into the outskirts.";
    badEndLoc.description = "Security teams triangulate your position and lock you in containment.";

    model.sceneModel.setSceneCenteredPoint(QPointF(startLoc.coordX, startLoc.coordY));

    auto* eStartToMarket = mgr.addEdge(
        startLoc.id,
        marketLoc.id,
        "Meet the fixer",
        "You enter the market under flickering neon."
    );
    if (!eStartToMarket || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    auto* eMarketToTower = mgr.addEdge(
        marketLoc.id,
        towerLoc.id,
        "Buy tower pass and move",
        "A fake badge gets you through the first checkpoint.",
        "(([P1] >= 1) and ([P2] <= 3))"
    );
    if (!eMarketToTower || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    auto* eTowerToMarket = mgr.addEdge(
        towerLoc.id,
        marketLoc.id,
        "Fallback to market",
        "You slip back into the crowd to cool things down."
    );
    if (!eTowerToMarket || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    auto* eTowerToArchive = mgr.addEdge(
        towerLoc.id,
        archiveLoc.id,
        "Route to archive",
        "You use a maintenance shaft to reach the archives.",
        "([P2] <= 4)"
    );
    if (!eTowerToArchive || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    auto* eArchiveToTower = mgr.addEdge(
        archiveLoc.id,
        towerLoc.id,
        "Return to tower",
        "You backtrack with copied route keys."
    );
    if (!eArchiveToTower || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    auto* eArchiveToHub = mgr.addEdge(
        archiveLoc.id,
        hubLoc.id,
        "Take cargo tunnel",
        "You emerge at the transit hub.",
        "(([P3] >= 2) and ([P2] <= 5))"
    );
    if (!eArchiveToHub || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    auto* eHubToArchive = mgr.addEdge(
        hubLoc.id,
        archiveLoc.id,
        "Return through tunnel",
        "You move back into archive maintenance corridors."
    );
    if (!eHubToArchive || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    auto* eHubToGood = mgr.addEdge(
        hubLoc.id,
        goodEndLoc.id,
        "Board extraction train",
        "Your forged token works and the train departs.",
        "(([P3] >= 4) and ([P2] <= 2))"
    );
    if (!eHubToGood || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    auto* eHubToBad = mgr.addEdge(
        hubLoc.id,
        badEndLoc.id,
        "Dash to platform under fire",
        "Sirens converge before you can clear the gate.",
        "(([P2] >= 4) or ([P3] <= 1))"
    );
    if (!eHubToBad || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    const int loopServiceId = mgr.addLoopEdge(
        archiveLoc.id,
        "Re-index archive manifests",
        "You run another archive consistency pass.",
        "(([P1] + [P3]) >= 3)"
    );
    if (loopServiceId < 0 || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

    if (eMarketToTower) {
        eMarketToTower->variableSettings.append({1, "([P1] >= 1)", "([P1] - 1)"});
        eMarketToTower->variableSettings.append({3, "([P2] <= 2)", "([P3] + 1)"});
    }

    if (eTowerToMarket) {
        eTowerToMarket->variableSettings.append({2, "([P2] >= 1)", "([P2] - 1)"});
    }

    if (eTowerToArchive) {
        eTowerToArchive->variableSettings.append({2, "([P3] < 3)", "([P2] + 2)"});
        eTowerToArchive->variableSettings.append({3, "([P3] >= 3)", "([P3] + 1)"});
    }

    if (eArchiveToTower) {
        eArchiveToTower->variableSettings.append({2, "([P2] >= 1)", "([P2] - 1)"});
    }

    if (eArchiveToHub) {
        eArchiveToHub->variableSettings.append({3, "([P3] >= 2)", "([P3] + 1)"});
        eArchiveToHub->variableSettings.append({2, "([P2] >= 2)", "([P2] - 2)"});
    }

    if (eHubToArchive) {
        eHubToArchive->variableSettings.append({2, "([P2] <= 4)", "([P2] + 1)"});
    }

    if (loopServiceId >= 0) {
        EdgeDef* forwardLoop = findEdgeByEndpoints(model.gameDef, archiveLoc.id, loopServiceId);
        if (forwardLoop) {
            forwardLoop->variableSettings.append({1, "([P1] >= 1)", "([P1] - 1)"});
            forwardLoop->variableSettings.append({3, "([P1] >= 1)", "([P3] + 2)"});
            forwardLoop->variableSettings.append({2, "([P1] <= 0)", "([P2] + 1)"});
        }
    }

    applyVisibilityOnEdge(eStartToMarket, 1, false);
    applyVisibilityOnEdge(eStartToMarket, 2, false);

    applyVisibilityOnEdge(eMarketToTower, 1, true);
    applyVisibilityOnEdge(eMarketToTower, 2, true);

    applyVisibilityOnEdge(eTowerToMarket, 2, false);

    applyVisibilityOnEdge(eTowerToArchive, 1, true);
    applyVisibilityOnEdge(eTowerToArchive, 2, false);

    applyVisibilityOnEdge(eArchiveToTower, 1, true);
    applyVisibilityOnEdge(eArchiveToTower, 2, true);

    applyVisibilityOnEdge(eArchiveToHub, 1, true);
    applyVisibilityOnEdge(eArchiveToHub, 2, true);

    applyVisibilityOnEdge(eHubToArchive, 2, false);

    return model;
}

} // namespace demo
} // namespace tge
