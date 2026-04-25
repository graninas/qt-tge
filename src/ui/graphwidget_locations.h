#ifndef GRAPHWIDGET_LOCATIONS_H
#define GRAPHWIDGET_LOCATIONS_H

#include "graphwidget.h"

// Location-related logic extracted from GraphWidget
namespace graphwidget_locations {

void handleNewLocationMode(GraphWidget* w, QMouseEvent* event);
void handleLocationDrag(GraphWidget* w, QMouseEvent* event);
void handleLocationHover(GraphWidget* w, QMouseEvent* event);
void handleLocationEdit(GraphWidget* w, int locationId);

// Optionally, add more location-related helpers here

}

#endif // GRAPHWIDGET_LOCATIONS_H
