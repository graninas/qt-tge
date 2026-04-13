#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "tge/domain.h"
#include <QVector>
#include <QColor>
#include <QString>
#include <QPointF>

class GraphModel {
public:
    QVector<tge::domain::LocationDef> locations;
    QVector<tge::domain::EdgeDef> edges;
    QPointF observedVirtualPoint = QPointF(0, 0); // Virtual point in graph coordinates

    // Helper to create a static test graph (3 nodes, 2 edges)
    static GraphModel makeTestGraph();
};

#endif // GRAPHMODEL_H
