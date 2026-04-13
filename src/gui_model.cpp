#include "gui_model.h"

using namespace tge::domain;

UiModel UiModel::makeTestGraph() {
    UiModel model;
    // Location 1: Start
    LocationDef loc1;
    loc1.id = 1;
    loc1.type = LocationType::Start;
    loc1.coordX = 0; loc1.coordY = 0;
    loc1.label = "Start";
    loc1.color = 11; // Green
    model.gameDef.locations[loc1.id] = loc1;
    // Location 2: Intermediate (Regular)
    LocationDef loc2;
    loc2.id = 2;
    loc2.type = LocationType::Regular;
    loc2.coordX = 2; loc2.coordY = 0;
    loc2.label = "Intermediate";
    loc2.color = 1; // Blue
    model.gameDef.locations[loc2.id] = loc2;
    // Location 3: Finish
    LocationDef loc3;
    loc3.id = 3;
    loc3.type = LocationType::Finish;
    loc3.coordX = 4; loc3.coordY = 0;
    loc3.label = "Finish";
    loc3.color = 12; // Deep Orange
    model.gameDef.locations[loc3.id] = loc3;
    // Set observedVirtualPoint to the first location's point
    model.observedVirtualPoint = QPointF(loc1.coordX, loc1.coordY);
    // Add 7 edges from start to intermediate
    for (int i = 1; i <= 7; ++i) {
        EdgeDef e; e.id = i; e.fromLocation = 1; e.toLocation = 2; model.gameDef.edges[e.id] = e;
    }
    // Add 7 edges from intermediate to start
    for (int i = 8; i <= 14; ++i) {
        EdgeDef e; e.id = i; e.fromLocation = 2; e.toLocation = 1; model.gameDef.edges[e.id] = e;
    }
    // 1 from intermediate to finish
    EdgeDef e15; e15.id = 15; e15.fromLocation = 2; e15.toLocation = 3; model.gameDef.edges[e15.id] = e15;
    return model;
}
