#include "gui_model.h"
#include <QDebug>

using namespace tge::domain;
using tge::editor::runtime::Manager;

UiModel UiModel::makeTestGraph() {
    UiModel model;
    Manager& mgr = model.manager;
    // Location 1: Start
    auto& loc1 = mgr.addLocation(LocationType::Start, "Start", 11, 0, 0);
    // Location 2: Intermediate 1 (Regular)
    auto& loc2 = mgr.addLocation(LocationType::Regular, "Intermediate1", 1, 2, 0);
    // Location 3: Intermediate 2 (Regular)
    auto& loc3 = mgr.addLocation(LocationType::Regular, "Intermediate2", 2, 3, 0);
    // Location 4: Finish
    auto& loc4 = mgr.addLocation(LocationType::Finish, "Finish", 12, 4, 0);
    model.observedVirtualPoint = QPointF(loc1.coordX, loc1.coordY);
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
