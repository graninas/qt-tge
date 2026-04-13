#include "graphmodel.h"

using namespace tge::domain;

GraphModel GraphModel::makeTestGraph() {
    GraphModel model;
    // Location 1: Start
    LocationDef loc1;
    loc1.id = 1;
    loc1.type = LocationType::Start;
    loc1.coordX = 0; loc1.coordY = 0;
    loc1.label = "Start";
    loc1.color = 11; // Green
    model.locations.append(loc1);
    // Location 2: Intermediate (Regular)
    LocationDef loc2;
    loc2.id = 2;
    loc2.type = LocationType::Regular;
    loc2.coordX = 2; loc2.coordY = 0;
    loc2.label = "Intermediate";
    loc2.color = 1; // Blue
    model.locations.append(loc2);
    // Location 3: Finish
    LocationDef loc3;
    loc3.id = 3;
    loc3.type = LocationType::Finish;
    loc3.coordX = 4; loc3.coordY = 0;
    loc3.label = "Finish";
    loc3.color = 12; // Deep Orange
    model.locations.append(loc3);
    // Set observedVirtualPoint to the first location's point
    if (!model.locations.isEmpty()) {
        model.observedVirtualPoint = QPointF(model.locations[0].coordX, model.locations[0].coordY);
    }
    // Add 7 edges from start to intermediate
    for (int i = 1; i <= 7; ++i) {
        EdgeDef e; e.id = i; e.fromLocation = 1; e.toLocation = 2; model.edges.append(e);
    }
    // Add 7 edges from intermediate to start
    for (int i = 8; i <= 14; ++i) {
        EdgeDef e; e.id = i; e.fromLocation = 2; e.toLocation = 1; model.edges.append(e);
    }
    // 1 from intermediate to finish
    EdgeDef e15; e15.id = 15; e15.fromLocation = 2; e15.toLocation = 3; model.edges.append(e15);
    return model;
}
