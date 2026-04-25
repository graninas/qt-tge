#include "gui_model.h"
#include <QDebug>

using namespace tge::domain;
using tge::editor::runtime::Manager;

UiModel UiModel::makeTestGraph() {
    UiModel model;
    Manager& mgr = model.manager;
    // Location 1: Start
    auto& loc1 = mgr.addStartLocation("Start", 0, 0);
    // Location 2: Intermediate 1 (Regular)
    auto& loc2 = mgr.addLocation("Intermediate1", 1, 2, 0);
    // Location 3: Intermediate 2 (Regular)
    auto& loc3 = mgr.addLocation("Intermediate2", 2, 3, 0);
    // Location 4: Finish
    auto& loc4 = mgr.addFinishLocation("Finish", 4, 0);
    model.sceneModel.setSceneCenteredPoint(QPointF(loc1.coordX, loc1.coordY));
    // Edge: start -> inter1
    mgr.addEdge(loc1.id, loc2.id, "", "");
    if (mgr.hasError()) qWarning() << "Demo graph error:" << mgr.lastError();
    // Edge: inter1 -> inter2
    mgr.addEdge(loc2.id, loc3.id, "", "");
    if (mgr.hasError()) qWarning() << "Demo graph error:" << mgr.lastError();
    // Edge: inter2 -> inter1 (cycle)
    mgr.addEdge(loc3.id, loc2.id, "", "");
    if (mgr.hasError()) qWarning() << "Demo graph error:" << mgr.lastError();
    // Edge: inter2 -> finish
    mgr.addEdge(loc3.id, loc4.id, "", "");
    if (mgr.hasError()) qWarning() << "Demo graph error:" << mgr.lastError();
    return model;
}

UiModel UiModel::makeTestGame()
{
  UiModel model;
  Manager &mgr = model.manager;

  model.gameDef.name = "Demo Quest: Locked Lab";
  model.gameDef.description = "Small demo graph with conditions, variables and a service-loop.";

  // Global integer parameters used in edge conditions.
  model.gameDef.globalVariables.append({"P1", "HasAccessCard", "1 when player has an access card.", VarType::Integer, "0"});
  model.gameDef.globalVariables.append({"P2", "Energy", "Current station energy level.", VarType::Integer, "3"});

  auto &startLoc = mgr.addStartLocation("Airlock", 0, 0);
  auto &hallLoc = mgr.addLocation("Security Hall", 2, 2, 0);
  auto &armoryLoc = mgr.addLocation("Armory", 4, 4, 1);
  auto &labLoc = mgr.addLocation("Locked Lab", 6, 2, 2);
  auto &reactorLoc = mgr.addLocation("Reactor Room", 8, 4, 2);
  auto &finishLoc = mgr.addFinishLocation("Escape Shuttle", 10, 1);

  startLoc.descriptionPack.descriptions = {
    "You dock at a damaged station. The only way forward is the security hall."
  };
  hallLoc.descriptionPack.descriptions = {
    "A silent corridor: armory to the east, lab door to the north, reactor warnings to the south."
  };
  armoryLoc.descriptionPack.descriptions = {
    "Old crates and a functional access card terminal."
  };
  labLoc.descriptionPack.descriptions = {
    "The lab contains emergency override controls."
  };
  reactorLoc.descriptionPack.descriptions = {
    "The reactor is unstable. You can try to stabilize it if you have enough energy."
  };
  finishLoc.descriptionPack.descriptions = {
    "You launch the shuttle and escape."
  };

  model.sceneModel.setSceneCenteredPoint(QPointF(startLoc.coordX, startLoc.coordY));

  // Main progression edges.
  mgr.addEdge(startLoc.id, hallLoc.id,
        "Enter the station",
        "You step into Security Hall.");
  if (mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

  mgr.addEdge(hallLoc.id, armoryLoc.id,
        "Search the armory",
        "You move to the Armory.");
  if (mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

  mgr.addEdge(armoryLoc.id, hallLoc.id,
        "Return to security hall",
        "You return to the hall.");
  if (mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

  // Condition uses parameter P1 (access card acquired).
  mgr.addEdge(hallLoc.id, labLoc.id,
        "Open the lab door",
        "The card reader accepts your credential.",
        "([P1] >= 1)");
  if (mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

  mgr.addEdge(labLoc.id, reactorLoc.id,
        "Take the service lift to reactor",
        "You descend into the reactor core.");
  if (mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

  // Condition uses both parameters: card and sufficient energy.
  mgr.addEdge(reactorLoc.id, finishLoc.id,
        "Board the escape shuttle",
        "Systems sync and the shuttle departs.",
        "(([P1] >= 1) and ([P2] >= 2))");
  if (mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

  // Feedback loop via an automatically created service location.
  // This models "re-check reactor diagnostics" and returns to the same location.
  const int loopServiceId = mgr.addLoopEdge(
    reactorLoc.id,
    "Re-check reactor diagnostics",
    "You run another diagnostics cycle.",
    "([P2] >= 1)");
  if (loopServiceId < 0 || mgr.hasError()) qWarning() << "Demo game error:" << mgr.lastError();

  return model;
}
