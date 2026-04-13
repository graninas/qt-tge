#include "graphmodel.h"

using namespace tge::domain;

GraphModel GraphModel::makeTestGraph() {
    GraphModel model;
    // Location 1: Start
    LocationDef loc1;
    loc1.id = 1;
    loc1.type = LocationType::Start;
    loc1.coordX = 0; loc1.coordY = 0;
    loc1.label = QString::number(loc1.id);
    loc1.color = 11; // Green (see palette)
    model.locations.append(loc1);
    // Location 2: Regular
    LocationDef loc2;
    loc2.id = 2;
    loc2.type = LocationType::Regular;
    loc2.coordX = 2; loc2.coordY = 0;
    loc2.label = QString::number(loc2.id);
    loc2.color = 1; // Blue
    model.locations.append(loc2);
    // Location 3: Finish
    LocationDef loc3;
    loc3.id = 3;
    loc3.type = LocationType::Finish;
    loc3.coordX = 4; loc3.coordY = 0;
    loc3.label = QString::number(loc3.id);
    loc3.color = 12; // Deep Orange
    model.locations.append(loc3);
    // Set observedVirtualPoint to the first location's point
    if (!model.locations.isEmpty()) {
        model.observedVirtualPoint = QPointF(model.locations[0].coordX, model.locations[0].coordY);
    }
    // Edge 1: 1 -> 2
    EdgeDef e1;
    e1.id = 1;
    e1.fromLocation = 1;
    e1.toLocation = 2;
    model.edges.append(e1);
    // Edge 2: 2 -> 3
    EdgeDef e2;
    e2.id = 2;
    e2.fromLocation = 2;
    e2.toLocation = 3;
    model.edges.append(e2);
    return model;
}
