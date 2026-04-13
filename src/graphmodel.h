#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "tge/domain.h"
#include <QVector>
#include <QColor>
#include <QString>
#include <QPointF>


// Appearance settings for drawing the graph (e.g. offsets for id and label)
// Layer: UI
struct AppearanceSettings
{
  int idOffsetY;
  int labelOffsetY;
  AppearanceSettings(int idOffsetY_ = -46, int labelOffsetY_ = 32)
      : idOffsetY(idOffsetY_), labelOffsetY(labelOffsetY_) {}
};

// Simple data model for the graph, containing locations and edges.
// Layer: Domain (TODO: move to tge/domain.h)
class GraphModel
{
public:
    QVector<tge::domain::LocationDef> locations;
    QVector<tge::domain::EdgeDef> edges;
    QPointF observedVirtualPoint = QPointF(0, 0); // Virtual point in graph coordinates
    AppearanceSettings appearance;

    // Helper to create a static test graph (3 nodes, 2 edges)
    static GraphModel makeTestGraph();
};

#endif // GRAPHMODEL_H
