#ifndef GRAPHWIDGET_EDGES_H
#define GRAPHWIDGET_EDGES_H

#include "graphwidget.h"

// Edge-related logic extracted from GraphWidget
namespace graphwidget_edges {

void startEdgeCreation(GraphWidget* w);
void cancelEdgeCreation(GraphWidget* w);
void finishEdgeCreation(GraphWidget* w, int destinationLocationId);

// Optionally, add more edge-related helpers here

}

#endif // GRAPHWIDGET_EDGES_H
