#include "gui_model.h"

using namespace tge::domain;
using tge::editor::runtime::Manager;

UiModel UiModel::makeTestGraph() {
    UiModel model;
    Manager& mgr = model.manager;
    // Location 1: Start
    auto& loc1 = mgr.addLocation(LocationType::Start, "Start", 11, 0, 0);
    // Location 2: Intermediate (Regular)
    auto& loc2 = mgr.addLocation(LocationType::Regular, "Intermediate", 1, 2, 0);
    // Location 3: Finish
    auto& loc3 = mgr.addLocation(LocationType::Finish, "Finish", 12, 4, 0);
    model.observedVirtualPoint = QPointF(loc1.coordX, loc1.coordY);
    // Add 7 edges from start to intermediate
    for (int i = 0; i < 7; ++i) {
        mgr.addEdge(loc1.id, loc2.id, "", "");
    }
    // Add 7 edges from intermediate to start
    for (int i = 0; i < 7; ++i) {
        mgr.addEdge(loc2.id, loc1.id, "", "");
    }
    // 1 from intermediate to finish
    mgr.addEdge(loc2.id, loc3.id, "", "");
    return model;
}
